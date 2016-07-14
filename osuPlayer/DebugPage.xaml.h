//
// DebugPage.xaml.h
// Declaration of the DebugPage class
//

#pragma once

#include "DebugPage.g.h"

namespace Debug
{
	using namespace Platform;
	using namespace Platform::Collections;
	using namespace Windows::Foundation::Collections;
	using namespace Windows::UI::Xaml;
	using namespace Windows::UI::Xaml::Data;
	using Windows::UI::Xaml::Navigation::NavigationEventArgs;
	using Windows::UI::Xaml::Controls::ListBox;
	using Windows::UI::Core::CoreDispatcher;

	[BindableAttribute]
	public ref class ViewModel sealed : public INotifyPropertyChanged
	{
	public:
		ViewModel() : list(ref new Vector<String^>) {}

		event virtual PropertyChangedEventHandler^ PropertyChanged;

		property IVector<String^>^ List {IVector<String^>^ get() { return list; }}

	private:
		void propertyChanged(Platform::String^ name) { PropertyChanged(this, ref new PropertyChangedEventArgs(name)); }

		Vector<String^>^ list;
	};

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DebugPage sealed
	{
	public:
		DebugPage();

		property Debug::ViewModel^ ViewModel {Debug::ViewModel^ get() { return viewModel; }}

	protected:
		void OnNavigatedTo(NavigationEventArgs^ e) override;

	private:
		Debug::ViewModel^ viewModel;
	};

	public ref class DebugParams sealed
	{
	public:
		DebugParams() : debugList(nullptr) {}

		property ListBox^ Debug
		{
			ListBox^ get() { return debugList; }
			void set(ListBox^ list) { debugList = list; }
		}

		property CoreDispatcher^ Dispatcher
		{
			CoreDispatcher^ get() { return dispatcher; }
			void set(CoreDispatcher^ dp) { dispatcher = dp; }
		}
		
		property ::Debug::ViewModel^ ViewModel
		{
			::Debug::ViewModel^ get() { return viewModel; }
			void set(::Debug::ViewModel^ vm) { viewModel = vm; }
		}

	private:
		ListBox^ debugList;
		CoreDispatcher^ dispatcher;
		::Debug::ViewModel^ viewModel;
	};
}
