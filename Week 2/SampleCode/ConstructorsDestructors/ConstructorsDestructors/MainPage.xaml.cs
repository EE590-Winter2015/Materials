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
using CppComponent;
using CSharpComponent;
using System.Diagnostics;

namespace ConstructorsDestructors
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        private void doTestOne()
        {
            // Basic test; create some objects, modify them, then exit
            CppObject cpp = new CppObject();
            CSharpObject cs = new CSharpObject();

            cpp.increment(4);
            cs.increment(5);
        }

        private void doTestTwo()
        {
            // Do the same as above, but show off alternative constructors
            CppObject cpp = new CppObject(10);
            CSharpObject cs = new CSharpObject(10000);

            cpp.increment(-5);
            cs.increment(-5000);

            // Also show off explicitly running destructor
            cpp.Dispose();
        }

        private void doTestThree()
        {
            // What do you think will be output here?
            CppObject cpp = new CppObject();
            cpp = new CppObject(5);
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            doTestOne();
            Debug.WriteLine("Done with block 1");

            doTestTwo();
            Debug.WriteLine("Done with block 2");

            doTestThree();
            Debug.WriteLine("Done with block 3");

            // Finally, FORCE garbage collection
            GC.Collect();
        }
    }
}
