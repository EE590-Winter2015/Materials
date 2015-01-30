using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace Async
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Call our first function
            FirstFunction();
        }

        private async void FirstFunction()
        {
            Debug.WriteLine("Entered FirstFunction()!");
            SecondFunction();
            Debug.WriteLine("Exiting FirstFunction()!");
        }

        private async void SecondFunction()
        {
            Debug.WriteLine("Entered SecondFunction()!");
            await ThirdFunction();
            Debug.WriteLine("Exiting SecondFunction()!");
        }

        private async Task ThirdFunction()
        {
            Debug.WriteLine("Entered ThirdFunction()!");

            // Simulate a workload that takes a while, so this function will be stopped, and others will continue
            await Task.Delay(1000);
            Debug.WriteLine("Exiting ThirdFunction()!");
        }
    }
}
