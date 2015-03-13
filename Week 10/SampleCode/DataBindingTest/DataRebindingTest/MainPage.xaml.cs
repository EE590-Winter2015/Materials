using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using DataRebindingTest.Resources;
using System.ComponentModel;

namespace DataRebindingTest
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Our class TestBinding now needs to inherity from INotifyPropertyChanged
        public class TestBinding : INotifyPropertyChanged
        {
            // This is an event that tells whatever is databound to us that the internal data has changed,
            // and the GUI needs to update accordingly.  Used by the NotifyChanged() helper function
            public event PropertyChangedEventHandler PropertyChanged;

            // Notify whatever is databound to us that propertyName has changed
            private void NotifyChanged(string propertyName)
            {
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
                }
            }

            // Define getter/setter for Name, but call NotifyChanged() when setting a new value
            private string _name;
            public string Name
            {
                get { return _name; }
                set
                {
                    _name =  value.ToUpper();
                    NotifyChanged("Name");
                }
            }

            // Define getter/setter for Address, but call NotifyChanged() when setting a new value
            private string _address;
            public string Address
            {
                set
                {
                    _address = value;
                    NotifyChanged("Address");
                }
            }
        }

        private TestBinding tb = new TestBinding();

        // Constructor
        public MainPage()
        {
            InitializeComponent();

            // Initial binding
            tb.Name = "Elliot Saba";
            tb.Address = "Campus box #352350";
            ContentPanel.DataContext = tb;
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            // Now we rebind
            tb.Name = "blah2!";
        }
    }
}