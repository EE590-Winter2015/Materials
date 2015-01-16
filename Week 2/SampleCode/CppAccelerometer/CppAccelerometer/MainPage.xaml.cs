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
using CppSensors;
using Windows.UI.Core;

namespace CppAccelerometer
{
    public sealed partial class MainPage : Page
    {
        // Create our C++ accelerometer object
        private CppAcc myAcc;

        public MainPage()
        {
            this.InitializeComponent();
            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Initialize it
            myAcc = new CppAcc();

            // Whenever we get a new reading, call this lambda
            myAcc.onReadingChanged += (double x, double y, double z) =>
            {
                // Inside of the lambda, tell the dispatcher to call yet ANOTHER lambda, which will execute on the actual GUI thread
                Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    // Now that we're on the GUI thread, update XAML elements with the new info from the C++ object
                    this.AccXOut.Text = "X: " + x;
                    this.AccYOut.Text = "Y: " + y;
                    this.AccZOut.Text = "Z: " + z;
                });
            };
        }

    }
}
