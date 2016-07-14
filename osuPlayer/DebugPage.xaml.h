//
// DebugPage.xaml.h
// Declaration of the DebugPage class
//

#pragma once

#include "DebugPage.g.h"

namespace Debug
{
	using Platform::String;
	using Windows::UI::Core::CoreDispatcher;
	using Windows::UI::Xaml::Navigation::NavigationEventArgs;
	using Windows::UI::Xaml::Controls::ListBox;

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class DebugPage sealed
	{
	public:
		DebugPage();

	protected:
		void OnNavigatedTo(NavigationEventArgs^ e) override;
	};

	public ref class DebugParams sealed
	{
	public:
		DebugParams() : debugList(nullptr) {}
		void Append(String^ str);

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
		
	private:
		ListBox^ debugList;
		CoreDispatcher^ dispatcher;
	};
}
