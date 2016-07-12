//
// App.xaml.h
// Declaration of the App class.
//

#pragma once

#include "App.g.h"

namespace osuPlayer
{
	using namespace Platform;
	using namespace Windows::UI::Core;
	using namespace Windows::UI::Xaml::Navigation;

	/// <summary>
	/// Provides application-specific behavior to supplement the default Application class.
	/// </summary>
	ref class App sealed
	{
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		App();

	private:
		void OnSuspending(Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Object ^sender, NavigationFailedEventArgs ^e);
		void OnNavigated(Object^ sender, NavigationEventArgs^ e);
		void OnBackRequested(Object ^sender, BackRequestedEventArgs ^args);
	};
}
