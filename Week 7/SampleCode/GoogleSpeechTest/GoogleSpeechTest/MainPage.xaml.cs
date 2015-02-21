using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using GoogleSpeechTest.Resources;
using System.IO;
using System.Threading.Tasks;
using System.Windows.Resources;
using System.Diagnostics;
using Newtonsoft.Json;
using Windows.Storage.Streams;
using libsound;
using libflac_wrapper;
using System.IO.IsolatedStorage;
using System.Threading;
using System.Windows.Documents;
using System.Windows.Media;
using System.Net.Http;


/*****
 [1,2,3,4,5]

 {"result": 1}

 public class myResult {
   public int result;
 }

 myResult r = new myResult();
 r.result = 1;
*/

namespace GoogleSpeechTest
{
    // These classes made from http://json2csharp.com/
    public class Alternative
    {
        public string transcript { get; set; }
        public double confidence { get; set; }
    }

    public class Result
    {
        public List<Alternative> alternative { get; set; }
        public bool final { get; set; }
    }

    public class RecognitionResult
    {
        public List<Result> result { get; set; }
        public int result_index { get; set; }
    }

    public partial class MainPage : PhoneApplicationPage
    {
        // This is the object we'll record data into
        private libFLAC lf = new libFLAC();
        private SoundIO sio = new SoundIO();

        // This is our list of float[] chunks that we're keeping track of
        private List<float[]> recordedAudio = new List<float[]>();

        // This is our flag as to whether or not we're currently recording
        private bool recording = false;

        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            // Setup SoundIO right away
            sio.audioInEvent += sio_audioInEvent;
            sio.start();
        }

        void sio_audioInEvent(float[] data)
        {
            // Only do something if we're recording right now
            if (this.recording)
            {
                // If we are recording, throw our data into our recordedAudio list
                recordedAudio.Add(data);

                // Update progress bar
                Dispatcher.BeginInvoke(() =>
                {
                    progress.Value = recordedAudio.Count / 10.0;
                });

                // If we're reached our maximum recording limit....
                if (recordedAudio.Count == 1000)
                {
                    // We stop ourselves! :P
                    stopRecording();
                }
            }
        }

        // This gets called when the button gets pressed while it says "Go"
        private void startRecording()
        {
            this.recording = true;
            this.goButton.Content = "Stop";
            this.textOutput.Text = "Recording...";
        }

        // This gets called when the button gets pressed while it says "Stop" or when we reach
        // our maximum buffer amount (set to 10 seconds right now)
        private void stopRecording()
        {
            this.recording = false;

            // Do this in a Dispatcher.BeginInvoke since we're not certain which thread is calling this function!
            Dispatcher.BeginInvoke(() =>
            {
                this.textOutput.Text = "Processing...";
                this.progress.Value = 0;
                this.goButton.Content = "Go";
                processData();
            });
        }

        // This is a utility to take a list of arrays and mash them all together into one large array
        private T[] flattenList<T>(List<T[]> list)
        {
            // Calculate total size
            int size = 0;
            foreach (var el in list)
            {
                size += el.Length;
            }

            // Allocate the returning array
            T[] ret = new T[size];

            // Copy each chunk into this new array
            int idx = 0;
            foreach (var el in list)
            {
                el.CopyTo(ret, idx);
                idx += el.Length;
            }

            // Return the "flattened" array
            return ret;
        }

        private async void processData()
        {
            // First, convert our list of audio chunks into a flattened single array
            float[] rawData = flattenList(recordedAudio);

            // Once we've done that, we can clear this out no problem
            recordedAudio.Clear();

            // Next, convert the data into FLAC:
            byte[] flacData = null;
            flacData = lf.compressAudio(rawData, sio.getInputSampleRate(), sio.getInputNumChannels());
            
            // Upload it to the server and get a response!
            RecognitionResult result = await recognizeSpeech(flacData, sio.getInputSampleRate());

            // Check to make sure everything went okay, if it didn't, check the debug log!
            if (result.result.Count != 0)
            {
                // This is just some fancy code to display each hypothesis as sone text that gets redder
                // as our confidence goes down; note that I've never managed to get multiple hypotheses
                this.textOutput.Inlines.Clear();
                foreach (var alternative in result.result[0].alternative)
                {
                    Run run = new Run();
                    run.Text = alternative.transcript + "\n\n";
                    byte bg = (byte)(alternative.confidence * 255);
                    run.Foreground = new SolidColorBrush(Color.FromArgb(255, 255, bg, bg));
                    textOutput.Inlines.Add(run);
                }
            }
            else
            {
                textOutput.Text = "Errored out!";
            }
        }

        private async Task<RecognitionResult> recognizeSpeech(byte[] flacData, uint sampleRate)
        {
            try
            {
                // Construct our HTTP request to the server
                string url = "https://www.google.com/speech-api/v2/recognize?output=json&lang=en-us&key=AIzaSyC-YKuxG4Pe5Xg1veSXtPPt3S3aKfzXDTM";
                HttpWebRequest request = WebRequest.CreateHttp(url);

                // Make sure we tell it what kind of data we're sending
                request.ContentType = "audio/x-flac; rate=" + sampleRate;
                request.Method = "POST";

                // Actually write the data out to the stream!
                using (var stream = await Task.Factory.FromAsync<Stream>(request.BeginGetRequestStream, request.EndGetRequestStream, null))
                {
                    await stream.WriteAsync(flacData, 0, flacData.Length);
                }

                // We are going to store our json response into this RecognitionResult:
                RecognitionResult root = null;

                // Now, we wait for a response and read it in:
                using (var response = await Task.Factory.FromAsync<WebResponse>(request.BeginGetResponse, request.EndGetResponse, null))
                {
                    // Construct a datareader so we can read everything in as a string
                    DataReader dr = new DataReader(response.GetResponseStream().AsInputStream());

                    dr.InputStreamOptions = InputStreamOptions.Partial;

                    uint datalen = await dr.LoadAsync(1024 * 1024);
                    string responseStringsCombined = dr.ReadString(datalen);

                    // Split this response string by its newlines
                    var responseStrings = responseStringsCombined.Split(new string[] { "\r\n", "\n" }, StringSplitOptions.None);

                    // Now, inspect the JSON of each string
                    foreach (var responseString in responseStrings)
                    {
                        root = JsonConvert.DeserializeObject<RecognitionResult>(responseString);

                        // If this is a good result
                        if (root.result.Count != 0)
                        {
                            //return it!
                            return root;
                        }
                    }
                }

                // Aaaaand, return the root object!
                return root;
            }
            catch(Exception e)
            {
                Debug.WriteLine("Error detected!  Exception thrown: " + e.Message);
            }

            // Otherwise, something failed, and we don't know what!
            return new RecognitionResult();
        }

        private void goButton_Click(object sender, RoutedEventArgs e)
        {
            if( this.recording ) {
                stopRecording();
            } else {
                startRecording();
            }
        }
    }
}