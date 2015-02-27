using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using HTTPClient.Resources;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using System.IO;
using System.Windows.Media;
using Newtonsoft.Json;

namespace HTTPClient
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }


        private async Task<string> readHTTPMessage(DataReader dr)
        {
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

        private async void getButton_Click(object sender, RoutedEventArgs e)
        {
            // This constructs a string of the form "http://www.google.com:80/"
            HttpWebRequest request = WebRequest.CreateHttp( "http://" + this.hostnameText.Text + ":" + this.portText.Text + this.resourceText.Text);
            request.Method = "POST";

            float[] data = new float[8];
            for (int i = 0; i < data.Length; ++i)
            {
                data[i] = 1 + i % 2;
            }

            string jsonData = JsonConvert.SerializeObject(data);
            using (var stream = await Task.Factory.FromAsync<Stream>(request.BeginGetRequestStream, request.EndGetRequestStream, null))
            {
                byte[] bytes = System.Text.Encoding.UTF8.GetBytes( jsonData );
                await stream.WriteAsync(bytes , 0, bytes.Length);
            }

            try
            {
                using (var response = await Task.Factory.FromAsync<WebResponse>(request.BeginGetResponse, request.EndGetResponse, null))
                {
                    // Create a datareader off of our response stream
                    DataReader dr = new DataReader(response.GetResponseStream().AsInputStream());

                    string rcvd_jsonData = await readHTTPMessage(dr);
                    float[] rcvd_data = JsonConvert.DeserializeObject<float[]>(rcvd_jsonData);

                    float sum = 0;
                    for (int i = 0; i < rcvd_data.Length; ++i)
                    {
                        sum += rcvd_data[i];
                    }

                    // Throw the HTTP text into our textOutput
                    this.textOutput.Text = "Sum: " + sum;

                    // As this was a successful request, make the text green:
                    this.textOutput.Foreground = new SolidColorBrush(Color.FromArgb(255, 128, 255, 128));
                }
            }
            catch (Exception ex)
            {
                // We ran into some kind of problem; output it to the user!
                this.textOutput.Text = "Error: " + ex.Message;
                this.textOutput.Foreground = new SolidColorBrush(Color.FromArgb(255, 255, 128, 128));
            }
        }
    }
}