//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

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
using namespace Windows::UI::Popups;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

MainPage::MainPage() :
	osuDir(nullptr), osuDB(nullptr)
{
	osuData = new ::osu::osu;
	viewModel = ref new osuPlayer::ViewModel;
	viewModel->isLoading->isLoading = true;
	InitializeComponent();

	auto localSettings = ApplicationData::Current->LocalSettings;
	auto path = safe_cast<String^>(localSettings->Values->Lookup("osu!Path"));

	viewModel->isLoading->isLoading = !!path;
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
			viewModel->isLoading->isLoading = false;
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
			viewModel->isLoading->isLoading = true;
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
				viewModel->isLoading->isLoading = false;
			});
		}
	});
	debug->Items->Append("Returned from test()");
}

void osuPlayer::MainPage::osuLoaded()
{
	auto o = osuData;
	debug->Items->Append("osu! Version: " + o->db.version + "\n"
		"Folders: " + o->db.folderCount + "\n"
		"Unlocked: " + o->db.unlocked + "\n"
		"Player name: " + o->db.playerName + "\n"
		"Beatmaps: " + o->db.bmapCount + "\n");

	int i = 0;
	int count = o->db.bmapCount;
	auto bmaps = o->db.bmaps;
	for (auto bmap : bmaps)
		debug->Items->Append("No. " + ++i + ":\n"
			"artist: " + bmap->artist + " / " + bmap->artistUnicode + ", title: " + bmap->title + " / " + bmap->titleUnicode + "\n"
			"creator: " + bmap->creator + ", difficulty: " + bmap->difficulty + "\n"
			"audio: " + bmap->audioFile + ", hash: " + bmap->hash + ", .osu: " + bmap->osuFile + "\n"
			"folder: " + bmap->folder + "\n");
}

void osuPlayer::MainPage::debugClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	viewModel->isLoading->isLoading = !viewModel->isLoading->isLoading;
}
