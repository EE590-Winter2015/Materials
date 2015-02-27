using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using LPC.Resources;
using libsound;
using LinearPredictiveCoding;
using LineGraph;
using CSharpFFTWrapper;

namespace LPC
{
    public partial class MainPage : PhoneApplicationPage
    {
        SoundIO sio;
        LinearPredictiveCoder lpc;
        LineGraphInterop lpcGraph, freqGraph;
        FFTW fft;
        uint numCoeffs = 10;

        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);
            lpc = new LinearPredictiveCoder();
            sio = new SoundIO();
            sio.audioInEvent += sio_audioInEvent;
            sio.start();
            fft = new FFTW(sio.getInputBufferSize());
        }

        void sio_audioInEvent(float[] data)
        {
            float gain;
            float[] coeffs = lpc.getLPCCoeffs(data, out gain, numCoeffs);

            // Calculate sampling over frequency
            ComplexNumber[] psd = new ComplexNumber[128];
            for (int wIdx = 0; wIdx < psd.Length; ++wIdx)
            {
                float w = (float)Math.PI * wIdx / psd.Length;

                psd[wIdx] = new ComplexNumber(1.0f, 0.0f);
                for (int k = 1; k <= coeffs.Length; ++k)
                {
                    psd[wIdx] += coeffs[k-1] * new ComplexNumber((float)Math.Cos(k*w), (float)-Math.Sin(k*w));
                }
            }

            if (lpcGraph != null)
            {
                float[] psdf = psd.Select(x => (float)Math.Log(Math.Sqrt(data.Length)*gain / x.mag())).ToArray();
                lpcGraph.setArray(psdf);
            }
            if (freqGraph != null)
            {
                float[] fftMag = fft.fftLogMag(data);
                freqGraph.setArray(fftMag);
            }
        }

        private void lpcCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            this.lpcGraph = setupLineGraph(sender);
            lpcGraph.setYLimits(-5, 15);
            lpcGraph.setColor(1.0f, 0.0f, 0.0f);
        }

        private void freqCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            this.freqGraph = setupLineGraph(sender);
            freqGraph.setYLimits(-5, 15);
        }

        private LineGraphInterop setupLineGraph( object sender )
        {
            LineGraphInterop lineGraph = new LineGraphInterop();
            DrawingSurface lineCanvas = sender as DrawingSurface;

            // Set window bounds in dips
            lineGraph.WindowBounds = new Windows.Foundation.Size(
                (float)lineCanvas.ActualWidth,
                (float)lineCanvas.ActualHeight
                );

            // Set native resolution in pixels
            lineGraph.NativeResolution = new Windows.Foundation.Size(
                (float)Math.Floor(lineCanvas.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
                (float)Math.Floor(lineCanvas.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
                );

            // Set render resolution to the full native resolution
            lineGraph.RenderResolution = lineGraph.NativeResolution;

            // Hook-up native component to DrawingSurface
            lineCanvas.SetContentProvider(lineGraph.CreateContentProvider());
            lineCanvas.SetManipulationHandler(lineGraph);

            return lineGraph;
        }

        private void coeffsSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            this.numCoeffs = (uint)e.NewValue;
        }
    }
}