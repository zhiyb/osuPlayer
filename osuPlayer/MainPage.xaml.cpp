//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "DebugPage.xaml.h"

#define ENABLE_DEBUG

using namespace osuPlayer;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace osu;
using namespace concurrency;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Popups;
using namespace Windows::UI::ViewManagement;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

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
	auto debugView = CoreApplication::CreateNewView();
	int debugViewID = 0;
	debugView->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([&] {
		Controls::Frame^ frame = ref new Controls::Frame;
		frame->Navigate(TypeName(Debug::DebugPage::typeid), debugParams);
		// Active the window to show it later
		Window::Current->Content = frame;
		Window::Current->Activate();
		debugViewID = ApplicationView::GetForCurrentView()->Id;

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
				MessageDialog^ msg = ref new MessageDialog("This application works by...");
				msg->Commands->Append(ref new UICommand("Ok"));
				msg->ShowAsync();
			}
		}).then([&](task<void> t) {
			try {
				t.get();
			} catch (...) {
				MessageDialog^ msg = ref new MessageDialog("This application works by...");
				msg->Commands->Append(ref new UICommand("Ok"));
				msg->ShowAsync();
			}
			viewModel->IsLoading->IsLoading = false;
		});
	}
}

void osuPlayer::MainPage::test(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	auto picker = ref new FolderPicker();
	picker->FileTypeFilter->Append(".exe");
	picker->FileTypeFilter->Append(".db");
	create_task(picker->PickSingleFolderAsync()).then([this](StorageFolder^ dir) {
		if (dir) {
			viewModel->IsLoading->IsLoading = true;
			debug->Items->Append("Picked: " + dir->Path);
			::osu::osu *o = new ::osu::osu;
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
	debug->Items->Append("Returned from test()");
}

void osuPlayer::MainPage::osuLoaded()
{
	auto o = osuData;
#ifdef ENABLE_DEBUG
#if 0
	if (debugParams->Debug)
		debugParams->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] {
			debugParams->Debug->Items->Append("osu! Version: " + o->db.version + "\n"
				"Folders: " + o->db.folderCount + "\n"
				"Unlocked: " + o->db.unlocked + "\n"
				"Player name: " + o->db.playerName + "\n"
				"Beatmaps: " + o->db.bmapCount + "\n");
		}));
#else
#if 1
	debugParams->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] {
		debugParams->ViewModel->List->Append("osu! Version: " + o->db.version + "\n"
			"Folders: " + o->db.folderCount + "\n"
			"Unlocked: " + o->db.unlocked + "\n"
			"Player name: " + o->db.playerName + "\n"
			"Beatmaps: " + o->db.bmapCount + "\n");
	}));
#else
	if (debugParams->ViewModel)
		debugParams->ViewModel->List->Append("osu! Version: " + o->db.version + "\n"
			"Folders: " + o->db.folderCount + "\n"
			"Unlocked: " + o->db.unlocked + "\n"
			"Player name: " + o->db.playerName + "\n"
			"Beatmaps: " + o->db.bmapCount + "\n");
#endif
#endif
#endif

	int i = 0;
	int count = o->db.bmapCount;
	auto bmaps = o->db.bmaps;
	for (auto bmap : bmaps) {
#ifdef ENABLE_DEBUG
#if 0
		debug->Items->Append("No. " + ++i + ":\t" + bmap->getTitle() + "\n"
			"artist: " + bmap->getArtist() + ", creator: " + bmap->creator + " [" + bmap->difficulty + "]\n"
			"audio: Songs/" + bmap->folder + "/" + bmap->audioFile + "\n"
			".osu: " + bmap->osuFile + "\n"
			"source: " + bmap->source + "\n");
#endif
#if 0
		debugParams->ViewModel->List->Append("No. " + ++i + ":\t" + bmap->getTitle() + "\n"
			"artist: " + bmap->getArtist() + ", creator: " + bmap->creator + " [" + bmap->difficulty + "]\n"
			"audio: Songs/" + bmap->folder + "/" + bmap->audioFile + "\n"
			".osu: " + bmap->osuFile + "\n"
			"source: " + bmap->source + "\n");
#endif
#endif
		ViewModel->Musics->Append(loadMusicFromBeatmap(ViewModel->_useOriginal(), bmap));
	}
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
	;
}
