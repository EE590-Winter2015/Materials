using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Threading.Tasks;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Networking.Proximity;
using Windows.Networking.Sockets;
using Windows.Storage.Streams;
using Windows.UI.Popups;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace Bluetooth
{
    public sealed partial class MainPage : Page
    {
        // The socket we'll communicate with the FoneAstra with
        private StreamSocket s;

        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Disabled;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            ReadData(SetupBluetoothLink());
        }

        private async Task<bool> SetupBluetoothLink()
        {
            // Tell PeerFinder that we're a pair to anyone that has been paried with us over BT
            PeerFinder.AlternateIdentities["Bluetooth:PAIRED"] = "";

            // Find all peers
            var devices = await PeerFinder.FindAllPeersAsync();

            // If there are no peers, then complain
            if (devices.Count == 0)
            {
                await new MessageDialog("No bluetooth devices are paired, please pair your FoneAstra").ShowAsync();

                // Neat little line to open the bluetooth settings
                await Windows.System.Launcher.LaunchUriAsync(new Uri("ms-settings-bluetooth:"));
                return false;
            }

            // Convert peers to array from strange datatype return from PeerFinder.FindAllPeersAsync()
            PeerInformation[] peers = devices.ToArray();

            // Find paired peer that is the FoneAstra
            PeerInformation peerInfo = devices.FirstOrDefault(c => c.DisplayName.Contains("FoneAstra"));

            // If that doesn't exist, complain!
            if (peerInfo == null)
            {
                await new MessageDialog("No bluetooth devices are paired, please pair your FoneAstra").ShowAsync();
                await Windows.System.Launcher.LaunchUriAsync(new Uri("ms-settings-bluetooth:"));
                return false;
            }

            // Otherwise, create our StreamSocket and connect it!
            s = new StreamSocket();
            await s.ConnectAsync(peerInfo.HostName, "1");
            return true;
        }

        // Read in bytes one at a time until we get a newline, then return the whole line
        private async Task<string> readLine(DataReader input)
        {
            string line = "";
            char a = ' ';
            // Keep looping as long as we haven't hit a newline OR line is empty
            while ((a != '\n' && a != '\r') || line.Length == 0)
            {
                // Wait until we have 1 byte available to read
                await input.LoadAsync(1);
                // Read that one byte, typecasting it as a char
                a = (char)input.ReadByte();

                // If the char is a newline or a carriage return, then don't add it on to the line and quit out
                if (a != '\n' && a != '\r')
                    line += a;
            }

            // Return the string we've built
            return line;
        }

        private async void ReadData(Task<bool> setupOK)
        {
            // Wait for the setup function to finish, when it does, it returns a boolean
            // If the boolean is false, then something failed and we shouldn't attempt to read data
            if (!await setupOK)
                return;

            // Construct a dataReader so we can read junk in
            DataReader input = new DataReader(s.InputStream);

            // Loop forever
            while (true)
            {
                // Read a line from the input, once again using await to translate a "Task<xyz>" to an "xyz"
                string line = (await readLine(input));

                // Append that line to our TextOutput
                this.textOutput.Text += line + "\n";
            }
        }
    }
}
