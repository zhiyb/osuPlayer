//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "osu.h"

namespace osuPlayer
{
	using namespace Platform;
	using namespace Windows::UI::Xaml;
	using namespace Windows::UI::Xaml::Data;

	[BindableAttribute]
	public ref class Loading sealed : public INotifyPropertyChanged
	{
	public:
		Loading(bool e) : loading(e) {}

		event virtual PropertyChangedEventHandler^ PropertyChanged;

		property bool isLoading
		{
			bool get() { return loading; }
			void set(bool e) { loading = e; propertyChanged(nullptr); }
		}

		property bool nLoading {bool get() { return !loading; }}
		property Visibility isVisible {Visibility get() { return loading ? Visibility::Visible : Visibility::Collapsed; }}
		property Visibility nVisible {Visibility get() { return !loading ? Visibility::Visible : Visibility::Collapsed; }}

	private:
		void propertyChanged(Platform::String^ name) { PropertyChanged(this, ref new PropertyChangedEventArgs(name)); }

		bool loading;
	};

	[BindableAttribute]
	public ref class ViewModel sealed : public INotifyPropertyChanged
	{
	public:
		ViewModel() : loading(ref new Loading(true)) {}

		event virtual PropertyChangedEventHandler^ PropertyChanged;

		property Loading^ isLoading{Loading^ get() { return loading; }}

	private:
		void propertyChanged(Platform::String^ name) { PropertyChanged(this, ref new PropertyChangedEventArgs(name)); }

		osuPlayer::Loading^ loading;
	};

	public ref class MainPage sealed
	{
	public:
		MainPage();

		property osuPlayer::ViewModel^ ViewModel
		{
			osuPlayer::ViewModel^ get() { return viewModel; }
		}

	private:
		void test(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void changeDir(Windows::Storage::StorageFolder^ dir);

		osuPlayer::ViewModel^ viewModel;
		Windows::Storage::StorageFolder^ osuDir;
		Windows::Storage::StorageFile^ osuDB;
		osu::osu^ osu;
		void debugClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
