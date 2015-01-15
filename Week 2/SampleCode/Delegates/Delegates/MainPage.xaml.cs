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
using CallbackTest;

namespace Delegates
{
    public sealed partial class MainPage : Page
    {
        // Our callback object which doesn't do THAT much, but shows off callbacks nonetheless.
        private CallbackObject callbackObj;

        // Keep track of whether we're pointing at Callback0 or Callback1
        private bool flipflop;

        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }
        
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Initialize everything
            this.callbackObj = new CallbackObject(Callback0);
            flipflop = false;
        }


        // Our callback functions.  These are what get called by callbackObj.process()
        protected void Callback0(float data)
        {
            this.textOutput0.Text = "Active: " + data;
            this.textOutput1.Text = "Inactive";
        }

        protected void Callback1(float data)
        {
            this.textOutput0.Text = "Inactive";
            this.textOutput1.Text = "Active: " + data;
        }

        
        // This calls process() when we click it
        private void processButton_Click(object sender, RoutedEventArgs e)
        {
            this.callbackObj.process();
        }

        // This switches which callback we have assigned to callbackObj, to change which function gets called when we call process()
        private void switchButton_Click(object sender, RoutedEventArgs e)
        {
            if (flipflop)
            {
                this.callbackObj.setCallback(Callback0);
            }
            else
            {
                this.callbackObj.setCallback(Callback1);
            }
            flipflop = !flipflop;
        }
    }
}
