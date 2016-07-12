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

using namespace concurrency;
using namespace Windows::UI::Popups;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

MainPage::MainPage() :
	osuDir(nullptr), osuDB(nullptr), osu(nullptr)
{
	viewModel = ref new osuPlayer::ViewModel;
	viewModel->isLoading->isLoading = true;
	InitializeComponent();

	auto localSettings = ApplicationData::Current->LocalSettings;
	auto path = safe_cast<String^>(localSettings->Values->Lookup("osuPath"));

	if (!path) {
		MessageDialog^ msg = ref new MessageDialog("This application works by...");
		msg->Commands->Append(ref new UICommand("Ok"));
		msg->ShowAsync();
		viewModel->isLoading->isLoading = false;
	} else {
		create_task(StorageFolder::GetFolderFromPathAsync(path)).then([=](StorageFolder^ folder) {
			return osu::osu::loadFromFolderAsync(folder);
		}).then([=](osu::osu^ o) {
			osu = o;
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
			create_task(osu::osu::loadFromFolderAsync(dir)).then([this](osu::osu^ o) {
				if (o) {
					osu = o;
					debug->Items->Append("osu! Version: " + o->db->version + "\n"
						"Folders: " + o->db->folderCount + "\n"
						"Unlocked: " + o->db->unlocked + "\n"
						"Player name: " + o->db->playerName + "\n"
						"Beatmaps: " + o->db->bmapCount + "\n");
				}
				viewModel->isLoading->isLoading = false;
			});
		}
	});
	debug->Items->Append("Returned from test()");
}

void osuPlayer::MainPage::debugClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	viewModel->isLoading->isLoading = !viewModel->isLoading->isLoading;
}
