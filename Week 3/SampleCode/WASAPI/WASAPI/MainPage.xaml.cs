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
using libsound;

namespace WASAPI
{
    public sealed partial class MainPage : Page
    {
        // Our Sound Input/Output object
        SoundIO sio;

        // Our "audiotool" object, which does things like convert from one audio format to another
        AudioTool at;

        public MainPage()
        {
            this.InitializeComponent();

            this.NavigationCacheMode = NavigationCacheMode.Required;
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Initialize sio
            sio = new SoundIO();

            // Initialize AudioTool with sio's output parameters:
            at = new AudioTool(sio.getOutputNumChannels(), sio.getOutputSampleRate());

            // Setup event logic with a lambda to run every time we get an input buffer of audio
            sio.audioInEvent += (float[] data) =>
            {
                // When we get a buffer of data, make Audio Tool convert it from mono to stereo:
                float[] output = at.convertChannels(data, 1);

                // Output that data to the speakers
                sio.writeAudio(output);
            };
        }

        private void startStopButton_Click(object sender, RoutedEventArgs e)
        {
            if (startStopButton.Content.Equals("Start"))
            {
                // Start the audio source running
                sio.start();

                // Output debugging info
                textOutput.Text  =  "Sound Parameters:\n";
                textOutput.Text +=  "Input: " + sio.getInputNumChannels() +
                                    " channels at " + sio.getInputSampleRate()/1000.0 +
                                    " KHz, " + sio.getInputBitdepth() + " bits per sample\n";
                textOutput.Text +=  "Output: " + sio.getOutputNumChannels() +
                                    " channels at " + sio.getOutputSampleRate()/1000.0 +
                                    " KHz, " + sio.getOutputBitdepth() + " bits per sample\n";

                startStopButton.Content = "Stop";
            }
            else
            {
                sio.stop();
                textOutput.Text = "Sound Stopped";
                startStopButton.Content = "Start";
            }
        }
    }
}
