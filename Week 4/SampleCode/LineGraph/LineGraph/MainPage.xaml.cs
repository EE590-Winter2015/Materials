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
using Windows.UI;
using libsound;

namespace LineGraph
{
    public sealed partial class MainPage : Page
    {
        private VisualizationTools.LineGraph graph;
        private SoundIO sio;
        private bool shouldPlot = false;

        public MainPage()
        {
            this.InitializeComponent();

            // Create our LineGraph and hook him up to LineGraph_XAML
            graph = new VisualizationTools.LineGraph((int)LineGraph_XAML.Width, (int)LineGraph_XAML.Height);
            LineGraph_XAML.Source = graph;

            sio = new SoundIO();
            sio.audioInEvent += sio_audioInEvent;
            sio.start();
        }

        void sio_audioInEvent(float[] data)
        {
            // Plot the incoming data every time we get a new buffer
            if (shouldPlot)
                graph.setArray(data);
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Also hook the Rendering cycle up to the CompositionTarget Rendering event so we draw frames when we're supposed to
            CompositionTarget.Rendering += graph.Render;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            shouldPlot = !shouldPlot;
        }
    }
}
