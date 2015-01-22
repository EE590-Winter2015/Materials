using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;
using CppThreadingComponent;
using Windows.UI.Core;

namespace Threading
{
    public sealed partial class MainPage : Page
    {
        CppCounter counter;
        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Initialize our counter
            counter = new CppCounter();

            // Subscribe to its events
            counter.onTimerTick += counter_onTimerTick;
        }

        void counter_onTimerTick(int count)
        {
            // Let's update a XAML element.  Remember, we HAVE to use the dispatcher here!
            Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                this.textOutput.Text = "Count: " + count;
            });
        }
    }
}
