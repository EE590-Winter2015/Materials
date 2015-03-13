using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using LongListSelectorTest_Rebind.Resources;
using System.Collections.ObjectModel;
using System.ComponentModel;

namespace LongListSelectorTest_Rebind
{
    public partial class MainPage : PhoneApplicationPage
    {
        public class TestBinding : INotifyPropertyChanged
        {
            public event PropertyChangedEventHandler PropertyChanged;

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

        // Note that we can't use a typical list, we need to use an Observable container, so we use ObservableCollection!
        private ObservableCollection<TestBinding> tbs = new ObservableCollection<TestBinding>();

        // Constructor
        public MainPage()
        {
            InitializeComponent();

            var tb = new TestBinding();
            tb.Name = "Elliot Saba";
            tb.Address = "Campus box #352350";
            tbs.Add(tb);

            tb = new TestBinding();
            tb.Name = "Les Atlas";
            tb.Address = "Campus box #314159";
            tbs.Add(tb);

            tb = new TestBinding();
            tb.Name = "Shwetak Patel";
            tb.Address = "Campus box #271828";
            tbs.Add(tb);
            list.ItemsSource = tbs;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var tb = new TestBinding();
            tb.Name = "Student #" + (tbs.Count - 3);
            tb.Address = "Student Address #" + (tbs.Count - 3);
            tbs.Add(tb);
        }
    }
}