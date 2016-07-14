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
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

DebugPage::DebugPage()
{
	viewModel = ref new Debug::ViewModel;
	InitializeComponent();
}

void Debug::DebugPage::OnNavigatedTo(NavigationEventArgs ^ e)
{
	auto params = (DebugParams^)e->Parameter;
	params->Debug = debug;
	params->Dispatcher = Window::Current->Dispatcher;
	params->ViewModel = viewModel;
}
