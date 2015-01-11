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
using CppCounter;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=391641

namespace CppTest
{
    public sealed partial class MainPage : Page
    {
        private FirstCppCounter myCounter;

        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Do initialization inside of OnNavigatedTo()
            myCounter = new FirstCppCounter();
        }

        private void incrementButton_Click(object sender, RoutedEventArgs e)
        {
            int value = myCounter.incrementBy(1);
            textOutput.Text = "Value: " + value;
        }
    }
}
