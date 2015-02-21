using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using RawClient.Resources;
using Windows.Networking.Sockets;
using Windows.Networking;
using System.IO;
using Windows.Storage.Streams;
using System.Threading.Tasks;
using Windows.Networking.Proximity;

namespace RawClient
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }

        private async Task<StreamSocket> connectSocket(string hostnameString, string portString)
        {
            // Create the socket we're gonna connect with
            StreamSocket s = new StreamSocket();

            // Convert our gross string to a hostname
            HostName hostname = new HostName(hostnameString);

            // Connect.  Note the await!
            await s.ConnectAsync(hostname, portString);

            // Pass s back!
            return s;
        }

        private async void sendData(StreamSocket s)
        {
            // Create a DataWriter so we can write out our data!
            DataWriter dw = new DataWriter(s.OutputStream);

            // Make sure we have "\r\n\r\n" at the end!
            string payload = this.inputText.Text;
            if (payload.Substring(payload.Length - 4, 4) != "\r\n\r\n")
                payload += "\r\n\r\n";

            dw.WriteString(payload);
            // Make sure it's sent!
            await dw.StoreAsync();
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
            while( true )
            {
                // Wait until we've got a megabyte, or the stream closed
                readyLen = await dr.LoadAsync(1024*1024);

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

        private async void sendButton_Click(object sender, RoutedEventArgs e)
        {
            // First, connect to the server
            StreamSocket s;
            try
            {
                this.outputText.Text = "Connecting...";
                s = await connectSocket(this.hostnameTextbox.Text, this.portTextbox.Text);
            }
            catch (Exception ex)
            {
                this.outputText.Text = "Error Connecting: " + ex.Message;
                return;
            }

            // Send our payload
            try
            {
                this.outputText.Text = "Transmitting...";
                sendData(s);
            }
            catch (Exception ex)
            {
                this.outputText.Text = "Error Sending: " + ex.Message;
                return;
            }


            // Get the response
            string response;
            try
            {
                this.outputText.Text = "Reading...";
                response = await readHTTPMessage(s);
            }
            catch (Exception ex)
            {
                this.outputText.Text = "Error Sending: " + ex.Message;
                return;
            }

            // Display the response to the user
            this.outputText.Text = response;
        }

    }
}