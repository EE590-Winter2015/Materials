using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Storage;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace Storage
{
    public sealed partial class MainPage : Page
    {
        public MainPage()
        {
            this.InitializeComponent();
        }

        protected async override void OnNavigatedTo(NavigationEventArgs e)
        {
            // First, get our local folder
            StorageFolder localFolder = ApplicationData.Current.LocalFolder;

            await CreateTestFile(localFolder, "test.txt", "This is a test!");
            string contents = await ReadTestFile(localFolder, "test.txt");

            Debug.WriteLine(contents);
        }

        private async Task CreateTestFile(StorageFolder folder, string filename, string contents)
        {
            // Write out to a file
            StorageFile file = await folder.CreateFileAsync(filename);
            await FileIO.WriteTextAsync(file, contents);
        }

        private async Task<string> ReadTestFile(StorageFolder folder, string filename) {
            StorageFile file = await folder.GetFileAsync(filename);
            return await FileIO.ReadTextAsync(file);
        }
    }
}
