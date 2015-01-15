using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Devices.Sensors;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace BasicAccelerometer
{
    public sealed partial class MainPage : Page
    {
        Accelerometer acc;
        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            acc = Accelerometer.GetDefault();
            acc.ReportInterval = acc.MinimumReportInterval;
            acc.ReadingChanged += acc_ReadingChanged;
        }

        void process_accelerometer(Accelerometer sender, AccelerometerReadingChangedEventArgs args)
        {
            Debug.WriteLine("Accelerometer reading: <" + args.Reading.AccelerationX + ", " + args.Reading.AccelerationY + ", " + args.Reading.AccelerationZ + ">");
        }
    }
}
