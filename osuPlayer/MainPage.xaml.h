//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "DebugPage.xaml.h"
#include "osu.h"

namespace osuPlayer
{
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation::Collections;
	using namespace Windows::UI::Xaml;
	using namespace Windows::UI::Xaml::Data;

	public ref class Music sealed
	{
	public:
		Music(IBox<bool>^ useOriginal,
			String^ artist, String^ artistOriginal,
			String^ title, String^ titleOriginal,
			String^ folder, String^ audioFile) :
			useOriginal(useOriginal),
			artist(artist), artistOriginal(artistOriginal),
			title(title), titleOriginal(titleOriginal),
			folder(folder), audioFile(audioFile) {}

		property String^ Artist {String^ get() { return (useOriginal && artistOriginal) ? artistOriginal : artist; }}
		property String^ Title {String^ get() { return (useOriginal && titleOriginal) ? titleOriginal : title; }}
		property String^ Folder {String^ get() { return folder; }}
		property String^ AudioFile {String^ get() { return audioFile; }}

	private:
		String ^artist, ^artistOriginal;
		String ^title, ^titleOriginal;
		String ^folder, ^audioFile;
		IBox<bool>^ useOriginal;
	};

	Music^ loadMusicFromBeatmap(IBox<bool>^ useOriginal, ::osu::osuBeatmap *bmap);

	[BindableAttribute]
	public ref class Loading sealed : public INotifyPropertyChanged
	{
	public:
		Loading(bool e) : isLoading(e) {}

		event virtual PropertyChangedEventHandler^ PropertyChanged;

		property bool IsLoading
		{
			bool get() { return isLoading; }
			void set(bool e) { isLoading = e; propertyChanged(nullptr); }
		}

		property bool nIsLoading {bool get() { return !isLoading; }}
		property Visibility IsVisible {Visibility get() { return isLoading ? Visibility::Visible : Visibility::Collapsed; }}
		property Visibility nVisible {Visibility get() { return !isLoading ? Visibility::Visible : Visibility::Collapsed; }}

	private:
		void propertyChanged(Platform::String^ name) { PropertyChanged(this, ref new PropertyChangedEventArgs(name)); }

		bool isLoading;
	};

	[BindableAttribute]
	public ref class ViewModel sealed : public INotifyPropertyChanged
	{
	public:
		ViewModel() : musics(ref new Vector<Music^>), isLoading(ref new Loading(true)), useOriginal(ref new Box<bool>(true)) {}
		IBox<bool>^ _useOriginal() { return useOriginal; }

		event virtual PropertyChangedEventHandler^ PropertyChanged;

		property IVector<Music^>^ Musics {IVector<Music^>^ get() { return musics; }}
		property Loading^ IsLoading {Loading^ get() { return isLoading; }}
		property bool UseOriginal {bool get() { return useOriginal->Value; }}

	private:
		void propertyChanged(Platform::String^ name) { PropertyChanged(this, ref new PropertyChangedEventArgs(name)); }

		Vector<Music^>^ musics;
		osuPlayer::Loading^ isLoading;
		Box<bool>^ useOriginal;
	};

	public ref class MainPage sealed
	{
	public:
		MainPage();

		property osuPlayer::ViewModel^ ViewModel {osuPlayer::ViewModel^ get() { return viewModel; }}

	private:
		void test(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void changeDir(Windows::Storage::StorageFolder^ dir);
		void osuLoaded();
		void debugClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void musicSelected(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);

		::Debug::DebugParams^ debugParams;

		osuPlayer::ViewModel^ viewModel;
		::osu::osu *osuData;
	};
}
