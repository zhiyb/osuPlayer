//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "DebugPage.xaml.h"

#define ENABLE_DEBUG

using namespace osuPlayer;
using namespace osu;

using namespace Concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Media::Core;
using namespace Windows::Media::Playback;
using namespace Windows::Security::Cryptography;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Popups;
using namespace Windows::UI::ViewManagement;

MainPage::MainPage()
{
	osuData = new ::osu::osu;
	viewModel = ref new osuPlayer::ViewModel;
	viewModel->IsLoading->IsLoading = true;
	debugParams = ref new Debug::DebugParams;
	InitializeComponent();

#ifdef ENABLE_DEBUG
	// The debug window
	auto dispatcher = Window::Current->Dispatcher;
	CoreApplication::CreateNewView()->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([&] {
		auto frame = ref new Controls::Frame;
		frame->Navigate(TypeName(Debug::DebugPage::typeid), debugParams);
		// Active the window to show it later
		Window::Current->Content = frame;
		Window::Current->Activate();
		int debugViewID = ApplicationView::GetForCurrentView()->Id;

		dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] {
			ApplicationViewSwitcher::TryShowAsStandaloneAsync(debugViewID);
		}));
	}));
#endif

	auto localSettings = ApplicationData::Current->LocalSettings;
	auto path = safe_cast<String^>(localSettings->Values->Lookup("osu!Path"));

	viewModel->IsLoading->IsLoading = !!path;
	if (path) {
		create_task(StorageFolder::GetFolderFromPathAsync(path)).then([=](StorageFolder^ folder) {
			return ::osu::loadFromFolderAsync(folder, osuData);
		}).then([=](bool res) {
			if (res) {
				osuLoaded();
			} else {
				auto msg = ref new MessageDialog("This application works by...");
				msg->Commands->Append(ref new UICommand("Ok"));
				msg->ShowAsync();
			}
		}).then([&](task<void> t) {
			try {
				t.get();
			} catch (...) {
				auto msg = ref new MessageDialog("This application works by...");
				msg->Commands->Append(ref new UICommand("Ok"));
				msg->ShowAsync();
			}
			viewModel->IsLoading->IsLoading = false;
		});
	}
}

void osuPlayer::MainPage::open(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto picker = ref new FolderPicker();
	picker->FileTypeFilter->Append(".exe");
	picker->FileTypeFilter->Append(".db");
	create_task(picker->PickSingleFolderAsync()).then([this](StorageFolder^ dir) {
		if (dir) {
			viewModel->IsLoading->IsLoading = true;
#ifdef ENABLE_DEBUG
			debugParams->Append("Picked: " + dir->Path);
#endif
			auto o = new ::osu::osu;
			create_task(::osu::loadFromFolderAsync(dir, o)).then([=](bool res) {
				if (res) {
					auto localSettings = ApplicationData::Current->LocalSettings;
					localSettings->Values->Insert("osu!Path", dir->Path);
					AccessCache::StorageApplicationPermissions::FutureAccessList->Add(dir);

					delete osuData;
					osuData = o;
					osuLoaded();
				}
				viewModel->IsLoading->IsLoading = false;
			});
		}
	});
#ifdef ENABLE_DEBUG
	debugParams->Append("Returned from test()");
#endif
}

void osuPlayer::MainPage::osuLoaded()
{
	auto o = osuData;
#ifdef ENABLE_DEBUG
	debugParams->Append("osu! Version: " + o->db.version + "\n"
		"Folders: " + o->db.folderCount + "\n"
		"Unlocked: " + o->db.unlocked + "\n"
		"Player name: " + o->db.playerName + "\n"
		"Beatmaps: " + o->db.bmapCount + "\n");
#endif

	int i = 0;
	int count = o->db.bmapCount;
	auto bmaps = o->db.bmaps;
	for (auto bmap : bmaps) {
#ifdef ENABLE_DEBUG
#if 0
		debugParams->Append("No. " + ++i + ":\t" + bmap->getTitle() + "\n"
			"artist: " + bmap->getArtist() + ", creator: " + bmap->creator + " [" + bmap->difficulty + "]\n"
			"audio: Songs/" + bmap->folder + "/" + bmap->audioFile + "\n"
			".osu: " + bmap->osuFile + "\n"
			"source: " + bmap->source + "\n");
#endif
#endif
		if (ViewModel->Musics->Size == 0 ||
			bmap->folder != ViewModel->Musics->GetAt(ViewModel->Musics->Size - 1)->Folder ||
			bmap->audioFile != ViewModel->Musics->GetAt(ViewModel->Musics->Size - 1)->AudioFile)
			ViewModel->Musics->Append(loadMusicFromBeatmap(ViewModel->_useOriginal(), bmap));
	}
}

void osuPlayer::MainPage::playMusic(Music ^ music)
{
	if (!music)
		return;
	debugParams->Append("Selected: " + music->Title + "\n" +
		music->Folder + "/" + music->AudioFile);
	create_task(osuData->dirSongs->GetFolderAsync(music->Folder)).then([=](StorageFolder^ folder) {
		//debugParams->Append(folder ? "GetFolder: Success" : "GetFolder: Failed");
		return folder->GetFileAsync(music->AudioFile);
	}).then([&](StorageFile^ file) {
		//debugParams->Append(file ? ("GetFile: Success: " + file->Path) : "GetFile: Failed");
		//return file->OpenAsync(FileAccessMode::Read);
		mediaElement->SetPlaybackSource(MediaSource::CreateFromStorageFile(file));
#if 0
	}).then([&](IRandomAccessStream^ stream) {
		//debugParams->Append(mediaSource ? "mediaSource: Success" : "mediaSource: Failed");
		debugParams->Append(stream ? "stream: Success" : "stream: Failed");
		mediaElement->SetSource(stream, audioFile->ContentType);
#endif
		//debugParams->Append("Volume: " + mediaElement->Volume);
		//mediaElement->Play();
	});
}

void osuPlayer::MainPage::debugClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	//viewModel->IsLoading->IsLoading = !viewModel->IsLoading->IsLoading;
}

Music^ osuPlayer::loadMusicFromBeatmap(IBox<bool>^ useOriginal, ::osu::osuBeatmap *bmap)
{
	return ref new Music(useOriginal,
		bmap->artist, bmap->artistUnicode,
		bmap->title, bmap->titleUnicode,
		bmap->folder, bmap->audioFile);
}

void osuPlayer::MainPage::musicSelected(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	auto items = e->AddedItems;
	if (items->Size)
		playMusic((Music^)items->GetAt(0));
}

void osuPlayer::MainPage::mediaStateChanged(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	switch (mediaElement->CurrentState) {
	case MediaElementState::Closed:
		debugParams->Append("MediaElementState::Closed");
		break;
	case MediaElementState::Opening:
		debugParams->Append("MediaElementState::Opening");
		break;
	case MediaElementState::Buffering:
		debugParams->Append("MediaElementState::Buffering");
		break;
	case MediaElementState::Playing:
		debugParams->Append("MediaElementState::Playing");
		break;
	case MediaElementState::Paused:
		debugParams->Append("MediaElementState::Paused");
		break;
	case MediaElementState::Stopped:
		debugParams->Append("MediaElementState::Stopped");
		break;
	}
}

void osuPlayer::MainPage::mediaEnded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	debugParams->Append("mediaEnded: " + mediaElement->Position.Duration);
#if 0
	if (!mediaElement->Position.Duration)
		return;
#endif
	shuffle();
}


void osuPlayer::MainPage::shuffle(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
	auto i = CryptographicBuffer::GenerateRandomNumber() % playlist->Items->Size;
	auto item = playlist->Items->GetAt(i);
	playlist->SelectedItem = item;
	playlist->ScrollIntoView(item);
}
