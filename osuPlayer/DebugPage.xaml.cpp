//
// DebugPage.xaml.cpp
// Implementation of the DebugPage class
//

#include "pch.h"
#include "DebugPage.xaml.h"

using namespace Debug;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Xaml::Automation;
using namespace Windows::UI::Xaml::Automation::Peers;
using namespace Windows::UI::Xaml::Automation::Provider;

DebugPage::DebugPage()
{
	InitializeComponent();
}

void Debug::DebugPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	auto params = (DebugParams^)e->Parameter;
	params->Debug = debug;
	params->Dispatcher = Window::Current->Dispatcher;
}

void Debug::DebugParams::Append(String ^ str)
{
	if (dispatcher)
		dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([=] {
			debugList->Items->Append(str);
			debugList->UpdateLayout();
			//debugList->ScrollIntoView(str);

			ListBoxAutomationPeer^ svAutomation = (ListBoxAutomationPeer^)ScrollViewerAutomationPeer::CreatePeerForElement(debugList);
			IScrollProvider^ scrollInterface = (IScrollProvider^)svAutomation->GetPattern(PatternInterface::Scroll);
			// If the vertical scroller is not available, the operation cannot be performed, which will raise an exception. 
			if (scrollInterface->VerticallyScrollable)
				scrollInterface->Scroll(ScrollAmount::NoAmount, ScrollAmount::LargeIncrement);
		}));
}
