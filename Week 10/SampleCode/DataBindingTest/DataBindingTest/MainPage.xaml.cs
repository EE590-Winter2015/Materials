using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using DataBindingTest.Resources;

namespace DataBindingTest
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Extremely simple class that we will bind to our ContentPanel
        public class TestBinding
        {
            public string Name { get; set; }
            public string Address { get; set; }
        }

        public MainPage()
        {
            InitializeComponent();

            // Create a new TestBinding object, fill it out, and bind it to the ContentPanel
            TestBinding tb = new TestBinding();
            tb.Name = "Elliot Saba";
            tb.Address = "asdf";
            ContentPanel.DataContext = tb;
        }
    }
}