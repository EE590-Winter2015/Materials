using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using NiceHTTPClient.Resources;
using System.Net.Http;

namespace NiceHTTPClient
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Constructor
        public MainPage()
        {
            InitializeComponent();

            doHttpStuff();
        }

        async void doHttpStuff()
        {
            HttpClient c = new HttpClient();
            var response = await c.GetAsync("http://google.com");

            textOutput.Text = await response.Content.ReadAsStringAsync();
        }
    }
}