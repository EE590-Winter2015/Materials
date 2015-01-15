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

namespace DispatcherTimerDemo
{
    public sealed partial class MainPage : Page
    {
        // The object that will increment a counter once every 100ms
        DispatcherTimer timer;
        int counter;

        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Initialize our counter to zero
            counter = 0;

            // Create our timer, set it to run ten times a second
            timer = new DispatcherTimer();
            timer.Interval = TimeSpan.FromMilliseconds(100);

            // Create a lambda, subscribe it to the timer tick event.
            timer.Tick += (object sender, object eb) =>
            {
                // Increment counter, and display it!
                counter += 1;

                // This is the line that will crash if we don't use Dispatcher!!!
                textOutput.Text = "" + counter;
            };

        }

        
        // Start/stop the timer and flip/flop the text on the button when the button is pressed
        private void StartStopButton_Click(object sender, RoutedEventArgs e)
        {
            if (StartStopButton.Content.Equals("Start"))
            {
                StartStopButton.Content = "Stop";
                timer.Start();
            }
            else
            {
                StartStopButton.Content = "Start";
                timer.Stop();
            }
        }
    }
}
