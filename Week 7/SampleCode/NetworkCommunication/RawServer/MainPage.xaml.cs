using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using RawServer.Resources;
using Windows.Networking.Sockets;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using Windows.Networking.Connectivity;

namespace RawServer
{
    public partial class MainPage : PhoneApplicationPage
    {
        // This is our global listener socket.  We need to keep track of it, since 
        private StreamSocketListener listener = null;

        // We grab the response text from the user and store it here just so that we don't have to
        // grab it from the UI control inside the socket communication routine, which would necessitate
        // Dispatcher.BeginInvoke() calls everywhere.
        private string responseText = "";

        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }

        private async Task<StreamSocketListener> startListener()
        {
            // Create the listening socket
            StreamSocketListener listener = new StreamSocketListener();

            // This is the event that gets triggered when somebody connects to us
            listener.ConnectionReceived += listener_ConnectionReceived;

            // We'll bind to the port given in portTextbox
            await listener.BindServiceNameAsync(this.portTextbox.Text);

            // Return our socket
            return listener;
        }

        // This gets the IP Addresses currently in use by this phone
        public static string[] FindIPAddresses()
        {
            // We're going to return a list of ip addresses
            List<string> ipAddresses = new List<string>();

            // Get all hostnames associated with this phone
            var hostnames = NetworkInformation.GetHostNames();
            foreach (var hn in hostnames)
            {
                // If this hostname has IP address information
                if (hn.IPInformation != null)
                {
                    // Store it in our List<>
                    ipAddresses.Add(hn.DisplayName);
                }
            }

            // Return an array instead of a list
            return ipAddresses.ToArray();
        }

        private async void listenButton_Click(object sender, RoutedEventArgs e)
        {
            // Are we starting or stopping?
            if (this.textInput.IsEnabled)
            {
                // Disable the UI controls so as to not spark any confusion
                this.textInput.IsEnabled = false;
                this.portTextbox.IsEnabled = false;
                this.listenButton.Content = "Stop";
                this.responseText = this.textInput.Text;

                // Start up the listening socket
                try
                {
                    this.listener = await startListener();
                }
                catch (Exception ex)
                {
                    this.textOutput.Text = "Bind error: " + ex.Message;
                    return;
                }

                // Tell the user of our smashing success!
                this.textOutput.Text += "Listening on port " + this.portTextbox.Text + ", on IP addresses:\n";
                // Initialize by printing out some debugging text
                string[] addresses = FindIPAddresses();
                foreach (var ip in addresses)
                {
                    // Output IP addresses to the screen
                    textOutput.Text += "  " + ip + "\n";
                }
                textOutput.Text += "\n";

                // The rest is left up to the ConnectionReceived function!
            }
            else
            {
                // Enable the UI controls again, 'cause we're stopping listening!
                this.textInput.IsEnabled = true;
                this.portTextbox.IsEnabled = true;
                this.listenButton.Content = "Start";

                // This manually disposes of the socket, closing it so we can rebind later
                this.listener.Dispose();
            }
        }

        private async Task<string> readHTTPMessage(StreamSocket s)
        {
            // Wait for a response
            DataReader dr = new DataReader(s.InputStream);

            // Make sure we are allowed to return early
            dr.InputStreamOptions = InputStreamOptions.Partial;

            // This is the value we will return
            string ret = "";

            // This is the amount of data ready to be read in
            uint readyLen;
            while (true)
            {
                // Wait until we've got a kilobyte, or the stream closed
                readyLen = await dr.LoadAsync(1024 * 1024);

                // Check to see if we actually have any data
                if (readyLen > 0)
                {
                    // Read in that string, append it to ret
                    ret += dr.ReadString(readyLen);

                    // Check for the "\r\n\r\n" at the end of messages
                    if (ret.Substring(ret.Length - 4, 4) == "\r\n\r\n")
                        break;
                }
                else
                {
                    // If not, the connection is closed!
                    return ret;
                }
            }

            // Finally, return that string
            return ret;
        }

        async void listener_ConnectionReceived(StreamSocketListener sender, StreamSocketListenerConnectionReceivedEventArgs args)
        {
            // Get the HTTP message from the phone
            string message = await readHTTPMessage(args.Socket);

            // Display it to the user
            Dispatcher.BeginInvoke(() =>
            {
                textOutput.Text += "Connection received from " + args.Socket.Information.RemoteHostName.DisplayName + "\n";
                textOutput.Text += message;
            });

            // Next, write out our predefined response (only once).  Append with "\r\n\r\n" so that the client knows when the message ends!
            DataWriter dw = new DataWriter(args.Socket.OutputStream);
            dw.WriteString(this.responseText + "\r\n\r\n");

            // Store that output!
            await dw.StoreAsync();
        }
    }
}