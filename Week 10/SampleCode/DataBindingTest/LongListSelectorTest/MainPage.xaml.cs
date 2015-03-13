using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using LongListSelectorTest.Resources;
using System.ComponentModel;

namespace LongListSelectorTest
{
    public partial class MainPage : PhoneApplicationPage
    {
        public class TestBinding : INotifyPropertyChanged
        {
            // This is an event that tells whatever is databound to us that the internal data has changed,
            // and the GUI needs to update accordingly.  Used by the NotifyChanged() helper function
            public event PropertyChangedEventHandler PropertyChanged;

            // Notify whatever is databound to us that [propertyName] has changed
            private void NotifyChanged(string propertyName)
            {
                if (PropertyChanged != null)
                {
                    PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
                }
            }

            private string _name;
            public string Name
            {
                get { return _name; }
                set
                {
                    _name = value.ToUpper();
                    NotifyChanged("Name");
                }
            }

            private string _address;
            public string Address
            {
                get { return _address; }
                set
                {
                    _address = value;
                    NotifyChanged("Address");
                }
            }
        }

        private TestBinding[] tbs = new TestBinding[10];

        // Constructor
        public MainPage()
        {
            InitializeComponent();

            // Initial binding
            tbs[0] = new TestBinding();
            tbs[1] = new TestBinding();
            tbs[2] = new TestBinding();

            tbs[0].Name = "Elliot Saba";
            tbs[0].Address = "Campus box #352350";

            tbs[1].Name = "Les Atlas";
            tbs[1].Address = "Campus box #314159";

            tbs[2].Name = "Shwetak Patel";
            tbs[2].Address = "Campus box #271828";

            for (int i = 3; i < 10; ++i)
            {
                tbs[i] = new TestBinding();
                tbs[i].Name = "Student #" + i;
                tbs[i].Address = "Student Address #" + i;
            }
            list.ItemsSource = tbs;
        }
    }
}