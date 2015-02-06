using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace LocksFixed
{
    public sealed partial class MainPage : Page
    {
        // This is a Timer, not a DispatcherTimer!  There is a BIG DIFFERENCE!
        private Timer t;
        float[] data;

        // This is a random object we create to act as our lock key
        Object lockKey = new Object();

        public MainPage()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Initialize data as a relatively small array of zeros
            data = new float[1024];

            // Start a timer (that runs on a different thread!) that will run thread_tick() every 1 milliseconds
            t = new Timer(thread_tick, null, 0, 1);
        }

        public void thread_tick(Object ignored)
        {
            // Simply for demonstration's sake, let's save data.Length
            int oldLength = data.Length;

            // Calculate average of data:
            float avg = 0.0f;
            lock (lockKey)
            {
                for (int i = 0; i < data.Length; ++i)
                {
                    avg += data[i];
                }
                avg /= data.Length;
            }

            // Output length to user
            Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                avgOut.Text = "Average: " + avg.ToString("0.00");
            });
        }

        private void randomizeButton_Click(object sender, RoutedEventArgs e)
        {
            // Create a "Random" object so we can generate random variables
            Random r = new Random();

            // Calculate the new length of data
            int newLen = (int)(r.NextDouble() * 1000000);

            lock (lockKey)
            {
                // Allocate space for data
                data = new float[newLen];

                // Fill data with randomness
                for (int i = 0; i < data.Length; ++i)
                {
                    data[i] = (float)r.NextDouble();
                }
            }
        }
    }
}
