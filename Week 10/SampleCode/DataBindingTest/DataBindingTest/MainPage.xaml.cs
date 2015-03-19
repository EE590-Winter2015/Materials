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
            private string _name;
            public string Name {
                get { return _name; }
                set { _name = value.ToUpper(); }
            }
            public string Address { get; set; }
        }

        // Create a new TestBinding object, fill it out, and bind it to the ContentPanel
        TestBinding tb = new TestBinding();

        public MainPage()
        {
            InitializeComponent();

            
            tb.Name = "Elliot Saba";
            tb.Address = "asdf";
            ContentPanel.DataContext = tb;
        }

        private void change_Click(object sender, RoutedEventArgs e)
        {
            tb.Name = "Shwetak Patel";
        }
    }
}