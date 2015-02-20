using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using ViolaJones.Resources;
using TextureGraph;
using libvideo;
using FaceDetection;
using Windows.Phone.Media.Capture;
using System.Windows.Resources;
// NOTE: Need this for .AsInputStream()!
using System.IO;


namespace ViolaJones
{
    public partial class MainPage : PhoneApplicationPage
    {
        private TextureGraphInterop texInterop;
        private Camera cam;
        private Detector detector;
        private ImageProcessing im;

        // Constructor
        public MainPage()
        {
            InitializeComponent();

            // Load in our detector
            StreamResourceInfo sri = Application.GetResourceStream(new Uri("Models\\face.xml", UriKind.Relative));
            detector = new Detector(sri.Stream.AsInputStream(), (uint)sri.Stream.Length);
        }

        private void canvas_Loaded(object sender, RoutedEventArgs e)
        {
            texInterop = new TextureGraphInterop();

            // Set window bounds in dips
            texInterop.WindowBounds = new Windows.Foundation.Size(
                (float)canvas.ActualWidth,
                (float)canvas.ActualHeight
                );

            // Set native resolution in pixels
            texInterop.NativeResolution = new Windows.Foundation.Size(
                (float)Math.Floor(canvas.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
                (float)Math.Floor(canvas.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
                );

            // Set render resolution to the full native resolution
            texInterop.RenderResolution = texInterop.NativeResolution;

            // Hook-up native component to DrawingSurface
            canvas.SetContentProvider(texInterop.CreateContentProvider());
            canvas.SetManipulationHandler(texInterop);

            var previewSize = new Windows.Foundation.Size(800, 448);

            cam = new Camera(previewSize, CameraSensorLocation.Back);
            im = new ImageProcessing(detector);

            // When we have an input frame, call ImageProcessing::processFrame
            cam.OnFrameReady += im.processFrame;

            // When we have processed a frame, output it to the textureInterop
            im.frameProcessed += texInterop.setTexturePtr;
        }
    }
}