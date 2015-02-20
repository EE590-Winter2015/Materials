#pragma once
#include <vector>
#include "Frame.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;


/*
This class provides a C++ implementation of Viola-Jones face detection using a
cascade of Haar-like features.  This class requires an XML file as input,
containing the learned coefficients for the Haar-like features.

To use this class, instantiate it from C#, and pass in the given .xml file:

    StreamResourceInfo sri = Application.GetResourceStream(new Uri("Models\\face.xml", UriKind.Relative));
    Detector detector = new Detector(sri.Stream.AsInputStream(), (uint)sri.Stream.Length);

Next, call detector.findFaces() from either C# or C++, whichever you prefer.
Note that the last three parameters can have a very large impact on performace. Try
starting out with the parameter values:

    d->findFaces( width, height, dataPtr, 7, .25, .2 );

This should give you a smooth analysis experience, while still attaining decent
detection accuracy.  Play around with the  values and see what the best values you can find are.

*/


namespace FaceDetection
{
	// Simple wrapper to store a weight and a Rect together
	public ref class RectWeight sealed
	{
	public:
		RectWeight( Rect rectangle, float weight ) {
			this->rect = rectangle;
			this->weight = weight;
		}

		float getWeight( void ) {
			return weight;
		}
	internal:
		Rect& getRect( void ) {
			return rect;
		}

	private:
		Windows::Foundation::Rect rect;
		float weight;
	};

	// A node in our classifier tree, note that we do not support trees of depth > 1, therefore we
	// never have to worry about children, we're always the root note and we're alwas a leaf node.
    public ref class Feature sealed
    {
    public:
		Feature( float threshold, float leftValue, float rightValue ) {
			this->threshold = threshold;
			this->leftValue = leftValue;
			this->rightValue = rightValue;
		}

		float getLeftValue() {
			return leftValue;
		}

		float getRightValue() {
			return rightValue;
		}

		float getThreshold() {
			return threshold;
		}

		void addRectWeight( RectWeight^ newRect ) {
			rects.push_back( newRect );
		}

	internal:
		const std::vector<RectWeight^>* getRectWeights() {
			return &rects;
		}

	private:
		float threshold;
		std::vector<RectWeight^> rects;
		float leftValue, rightValue;
    };


	// A stage is a collection of features
	public ref class Stage sealed {
	public:
		Stage( float threshold ) {
			this->threshold = threshold;
		}

		void addFeature( Feature^ newFeature ) {
			this->features.push_back( newFeature );
		}

		float getThreshold() {
			return threshold;
		}

	internal:
		const std::vector<Feature^>* getFeatures() {
			return &features;
		}

	private:
		std::vector<Feature^ > features;
		float threshold;
	};


    public ref class Detector sealed
    {
    public:
		Detector( IInputStream^ input, unsigned int length );
		virtual ~Detector();

		void addStage( Stage^ newStage ) {
			this->stages.push_back( newStage );
		}

		// This is the big kahuna, the one that does all the work. Call this guy with the customary first three arguments
		// The latter three arguments are parameters that make a tradeoff between face detection accuracy and speed
		//
		// downsampleFactor controls how much the image is downsampled: I recommend a value between 5-8
		// scaleStep controls (in percentage, so a value of 0.1 == 10%) how quickly the analysis window grows.
		//     I recommend a value between .1-.4
		// shiftStep controls (in percentage, so a value of 0.1 == 10%) how much the analysis window moves when sliding across the image
		//     I recommend a value between .1-.4
		Array<Rect>^ findFaces(unsigned int width, unsigned int height, uintptr_t dataPtr, unsigned int downsampleFactor, float scaleStep, float shiftStep );

	private:
		Frame& checkArraySize( Frame& f, Frame& check);

		// Internal size of cascade; this is the native size of the feature window
		Size size;

		// stages we've loaded in
		std::vector<Stage^> stages;
		Frame squared, valscale, integral;
    };
}