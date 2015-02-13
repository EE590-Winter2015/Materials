using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using KNearestNeighbors.Resources;
using LineGraph;
using libsound;
using libfilter;
using System.Windows.Threading;
using Microsoft.Devices.Sensors;
using System.Threading.Tasks;
using System.Diagnostics;

namespace KNearestNeighbors
{
    public partial class MainPage : PhoneApplicationPage
    {
        // Our RGB objects
        LineGraphInterop rGraph = null;
        LineGraphInterop gGraph = null;
        LineGraphInterop bGraph = null;
        LineGraphInterop posGraph = null;
        LineGraphInterop negGraph = null;

        // Our sensor objects
        SoundIO sio = null;
        AudioTool at = null;
        Motion motion = null;

        // Our analysis tools
        Filter[] filters = null;


        // List of "positive" training examples
        List<float[]> posExamples = null;

        // List of "negative" training examples
        List<float[]> negExamples = null;
        
        // Random stuff
        float beepFreq = 0.0f;
        float beepPhase = 0.0f;
        int currPlotType = 0;   // The current plot type, 0 => "Acceleration", 1 => "Rotation", 2 => "Attitude"
        string[] plotTypes = { "Acceleration", "Rotation Rate", "Attitude" };

        // These are flags that determine what the sensor callback does
        bool recordingNegative = false;
        bool recordingPositive = false;
        bool classifying = false;


        // Constructor
        public MainPage()
        {
            InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            base.OnNavigatedTo(e);

            // Initialize our machine learning subsystems
            setupLearning();

            // Initialize our sensor packages
            setupSensors();
        }

        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            // Cleanup our sensor packages
            cleanupSensors();
            base.OnNavigatingFrom(e);
        }

        private void setupSensors()
        {
            // Initialize the combined orientation sensor
            try
            {
                motion = new Motion();
                motion.TimeBetweenUpdates = TimeSpan.FromMilliseconds(100);
                motion.CurrentValueChanged += motion_CurrentValueChanged;
                motion.Start();
            }
            catch
            {
                // Print out an error
                MessageBox.Show("Could not initialize Motion API.  This phone does not have the necessary sensors to run this code properly!");

                // The kill the current application
                Application.Current.Terminate();
            }

            // Setup sound output
            sio = new SoundIO();
            sio.audioOutEvent += sio_audioOutEvent;
            sio.start();
            at = new AudioTool(sio.getOutputNumChannels(), sio.getOutputSampleRate());
        }

        private void cleanupSensors()
        {
            if (motion != null)
                motion.Stop();

            sio.stop();
        }

        private void setupLearning()
        {
            posExamples = new List<float[]>();
            negExamples = new List<float[]>();

            // Initialize dummy filters for all 9 datatypes
            filters = new Filter[9];
            for (int i = 0; i < 9; ++i)
            {
                float[] h = { 1.0f, 1.0f, 1.0f };
                filters[i] = new Filter(h);
            }
        }

        private float[] calcFeatureVector(MotionReading r)
        {
            float[] fv = new float[2];

            float accVecMag = (float)(Math.Pow(r.DeviceAcceleration.X, 2) + Math.Pow(r.DeviceAcceleration.Y, 2) + Math.Pow(r.DeviceAcceleration.Z, 2));
            fv[0] = filters[0].filter(accVecMag);
            fv[1] = filters[1].filter((float)r.Attitude.Yaw);

            return fv;
        }

        private void motion_CurrentValueChanged(object sender, SensorReadingEventArgs<MotionReading> e)
        {
            float[] fv = calcFeatureVector(e.SensorReading);
            if (recordingNegative || recordingPositive)
            {
                if (recordingNegative)
                    negExamples.Add(fv);
                if (recordingPositive)
                    posExamples.Add(fv);

                recordingNegative = false;
                recordingPositive = false;
                Dispatcher.BeginInvoke(() =>
                {
                    recPosButton.IsEnabled = true;
                    recNegButton.IsEnabled = true;
                    updateClassifyButton();
                });
            }

            if (classifying)
            {
                if (classifyData(fv, 3))
                {
                    Dispatcher.BeginInvoke(() =>
                    {
                        textOut.Text = "Positive!";
                    });
                    beepFreq = 1000.0f;
                }
                else
                {
                    Dispatcher.BeginInvoke(() =>
                    {
                        textOut.Text = "Negative!";
                    });
                    beepFreq = 0.0f;
                }
            }

            plotMotion(e.SensorReading);
        }

        // Calculate the distance between the feature vectors a and b
        double calcDistance(float[] a, float[] b)
        {
            double result = 0.0;

            for (int i = 0; i < a.Length; ++i)
            {
                result += Math.Abs(a[i] - b[i]);
            }

            return result;
        }
        
        private double sortNeighbors(ref List<float[]> neighbors, float[] test_reading)
        {
            // Sort them such that nearestNeighbors[0] is the farthest one
            neighbors.Sort(delegate(float[] x, float[] y)
            {
                return -calcDistance(x, test_reading).CompareTo(calcDistance(y, test_reading));
            });

            // Return the farthest distance, (e.g. the distanceThreshold)
            return calcDistance(neighbors[0], test_reading);
        }

        // Here is where we do the actual magic.  We take in a MotionReading r, and decide
        // whether it is most similar to a "positive" example, or a "negative" example.
        private bool classifyData(float[] test_reading, int K)
        {
            // Just classify it as a negative example if we don't have enough data
            if( posExamples.Count < K || negExamples.Count < K )
                return false;

            List<float[]> nearestNeighbors = new List<float[]>();

            // Initialize nearestNeighbors with K random positive neighbors, so we start out with something
            for( int k=0; k<K; ++k )
                nearestNeighbors.Add(posExamples[k]);

            // This is the distance threshold that a new training vector needs to be smaller than to surpass any of the existing neighbors
            double distanceThreshold = sortNeighbors(ref nearestNeighbors, test_reading);

            float[] posDistances = new float[posExamples.Count];
            int idx = 0;
            foreach (var r in posExamples)
            {
                // Calculate the distance between this training vector and our reading
                double distance = calcDistance(r, test_reading);

                // If the distance is less than that of the farthest neighbor, kick that neighbor out and replace him with this one
                if (distance < distanceThreshold)
                {
                    // The neighbor that is the farthest away is the first one
                    nearestNeighbors[0] = r;
                    distanceThreshold = sortNeighbors(ref nearestNeighbors, test_reading);
                }

                // Add this into posDistances to keep track for visualization later
                posDistances[idx] = (float)distance;
                idx++;
            }

            float[] negDistances = new float[negExamples.Count];
            idx = 0;
            // Do the same for negExamples
            foreach (var q in negExamples)
            {
                // Calculate the distance between this training vector and our reading
                double distance = calcDistance(q, test_reading);

                // If the distance is less than that of the farthest neighbor, kick that neighbor out and replace him with this one
                if (distance < distanceThreshold)
                {
                    // The neighbor that is the farthest away is the first one
                    nearestNeighbors[0] = q;
                    distanceThreshold = sortNeighbors(ref nearestNeighbors, test_reading);
                }

                // Add this into posDistances to keep track for visualization later
                negDistances[idx] = (float)distance;
                idx++;
            }

            // Now, figure out if the nearest neighbors are mostly negative or positive
            int numPos = 0;
            int numNeg = 0;
            foreach (var r in nearestNeighbors)
            {
                if( posExamples.Contains(r) )
                    numPos++;
                if( negExamples.Contains(r) )
                    numNeg++;
            }

            // For visualization purposes, plot sorted versions of posDistances and negDistances' reciprocals
            Array.Sort(posDistances);
            Array.Sort(negDistances);

            for (int i = 0; i < posDistances.Length; ++i)
            {
                posDistances[i] = posDistances[0] / posDistances[i];
            }
            for (int i = 0; i < negDistances.Length; ++i)
            {
                negDistances[i] = negDistances[0] / negDistances[i];
            }

            posGraph.setArray(posDistances);
            negGraph.setArray(negDistances);

            return numPos > numNeg;
        }

        void plotMotion(MotionReading r)
        {
            // If any of the graphs are null, we don't want to do anything
            if (rGraph == null || gGraph == null || bGraph == null)
                return;

            float normalizationTerm;
            switch (currPlotType)
            {
                // 0 => Acceleration
                case 0:
                    normalizationTerm = 4.0f;
                    rGraph.appendToArray((float)r.DeviceAcceleration.X / normalizationTerm);
                    gGraph.appendToArray((float)r.DeviceAcceleration.Y / normalizationTerm);
                    bGraph.appendToArray((float)r.DeviceAcceleration.Z / normalizationTerm);
                    break;
                // 1 => Rotation Rate
                case 1:
                    normalizationTerm = 20.0f;
                    rGraph.appendToArray((float)r.DeviceRotationRate.X / normalizationTerm);
                    gGraph.appendToArray((float)r.DeviceRotationRate.Y / normalizationTerm);
                    bGraph.appendToArray((float)r.DeviceRotationRate.Z / normalizationTerm);
                    break;
                // 2 => Attitude
                case 2:
                    normalizationTerm = 2.0f*(float)Math.PI;
                    rGraph.appendToArray((float)r.Attitude.Pitch / normalizationTerm);
                    gGraph.appendToArray((float)r.Attitude.Yaw / normalizationTerm);
                    bGraph.appendToArray((float)r.Attitude.Roll / normalizationTerm);
                    break;
            }
        }


        float[] sio_audioOutEvent(uint numSamples)
        {
            // If we're supposed to beep,
            if (beepFreq != 0.0f)
            {
                // Generate the data, and track our phase so it sounds nice and smooth
                var data = at.sin(numSamples, beepFreq, beepPhase);
                beepPhase = (beepPhase + 2 * (float)Math.PI * numSamples * beepFreq / sio.getOutputSampleRate()) % (2 * (float)Math.PI);
                return data;
            }

            // Don't output anything
            return null;
        }

        #region Graph initialization

        // Generic way to hook a LineGraphInterop up with a DrawingSurface
        private LineGraphInterop initializeLineGraph(DrawingSurface canvas, LineGraph.LineGraphReady InitializedCallback)
        {
            // Create the LineGraphInterop, the C++/CX component that will draw out to the canvas
            LineGraphInterop graph = new LineGraphInterop();

            // Set window bounds
            graph.WindowBounds = new Windows.Foundation.Size(
                (float)canvas.ActualWidth,
                (float)canvas.ActualHeight
                );

            // Set native/rendering resolution!
            graph.NativeResolution = new Windows.Foundation.Size(
                (float)Math.Floor(canvas.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
                (float)Math.Floor(canvas.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
                );
            graph.RenderResolution = graph.NativeResolution;

            // Notify the canvas who we're hooking up to them
            canvas.SetContentProvider(graph.CreateContentProvider());
            canvas.SetManipulationHandler(graph);

            graph.Initialized += InitializedCallback;
            Dispatcher.BeginInvoke(() => InitializedCallback());

            // Give back the graph object!
            return graph;
        }

        private void rCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            rGraph = initializeLineGraph(rCanvas, rGraph_Initialized);
        }

        void rGraph_Initialized()
        {
            rGraph.setColor(1.0f, 0.0f, 0.0f);
            resetGraphs();
        }

        private void gCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            gGraph = initializeLineGraph(gCanvas, gGraph_Initialized);
        }

        void gGraph_Initialized()
        {
            gGraph.setColor(0.0f, 1.0f, 0.0f);
            resetGraphs();
        }

        private void bCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            bGraph = initializeLineGraph(bCanvas, bGraph_Initialized);
        }

        void bGraph_Initialized()
        {
            bGraph.setColor(0.0f, 0.0f, 1.0f);
            resetGraphs();
        }

        private void posCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            posGraph = initializeLineGraph(posCanvas, posGraph_Initialized);
        }

        void posGraph_Initialized()
        {
            posGraph.setYLimits(0, 2);
            posGraph.setColor(1.0f, 0.0f, 0.0f);
        }

        private void negCanvas_Loaded(object sender, RoutedEventArgs e)
        {
            negGraph = initializeLineGraph(negCanvas, negGraph_Initialized);
        }

        void negGraph_Initialized()
        {
            negGraph.setYLimits(0, 2);
            negGraph.setColor(0.0f, 0.0f, 1.0f);
        }
        #endregion

        private void resetGraphs()
        {
            if (rGraph != null)
                rGraph.setArray(new float[25]);
            if (gGraph != null)
                gGraph.setArray(new float[25]);
            if (bGraph != null)
                bGraph.setArray(new float[25]);
        }

        private void plotTypeButton_Click(object sender, RoutedEventArgs e)
        {
            currPlotType = (currPlotType + 1) % plotTypes.Length;
            plotTypeButton.Content = plotTypes[currPlotType];
            resetGraphs();
        }

        private async Task doBeep(float freq, int duration)
        {
            beepFreq = freq;
            await Task.Delay(duration);
            beepFreq = 0;
        }


        private async void recPosButton_Click(object sender, RoutedEventArgs e)
        {
            recNegButton.IsEnabled = false;
            recPosButton.IsEnabled = false;
            recordingPositive = true;
        }

        private async void recNegButton_Click(object sender, RoutedEventArgs e)
        {
            recNegButton.IsEnabled = false;
            recPosButton.IsEnabled = false;
            recordingNegative = true;
        }

        private void updateClassifyButton()
        {
            if (posExamples.Count > 0 && negExamples.Count > 0)
            {
                classifyButton.IsEnabled = true;
            }
            else
            {
                classifyButton.IsEnabled = false;
            }
        }

        private void classifyButton_Click(object sender, RoutedEventArgs e)
        {
            if (classifying)
            {
                classifyButton.Content = "Stop";
            }
            else
            {
                classifyButton.Content = "Classify!";
            }
            classifying = !classifying;
        }

        private void clearButton_Click(object sender, RoutedEventArgs e)
        {
            posExamples.Clear();
            negExamples.Clear();
            classifying = false;
            updateClassifyButton();
            posGraph.setArray(new float[2]);
            negGraph.setArray(new float[2]);
        }

    }
}