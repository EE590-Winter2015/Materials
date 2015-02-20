// FaceDetection.cpp
#include <ppltasks.h>
#include "FaceDetection.h"
#include "pugixml.hpp"
#include <windows.h>

using namespace concurrency;
using namespace FaceDetection;
using namespace pugi;

Detector::Detector( IInputStream^ input, unsigned int length )
{
	DataReader^ dr = ref new DataReader( input );
	create_task( dr->LoadAsync(length) ).then( [this,dr,length](task<unsigned int> bytesLoaded ) {
		unsigned int bl = bytesLoaded.get();
		if( bl != length ) {
			OutputDebugString( L"length != bytesLoaded!" );
		}
		Array<unsigned char>^ a = ref new Array<unsigned char>(length);
		dr->ReadBytes(a);
		xml_document doc;
		xml_parse_result result = doc.load_buffer( a->Data, a->Length );
		if( !result ) {
			OutputDebugString( L"Invalid XML!  Error message:" );
			OutputDebugStringA( result.description() );
			throw ref new InvalidArgumentException( L"Invalid XML file, failed to parse!" );
		}
		
		xml_node cascadeNode = doc.child("opencv_storage").first_child();

		// First, find the size
		xml_node sizeNode = cascadeNode.child("size");
		sscanf_s(sizeNode.first_child().value(), "%f %f", &this->size.Width, &this->size.Height );

		// Iterate over the stages:
		for( xml_node stageNode : cascadeNode.child("stages").children("_") ) {
			// Get this stage's threshold:
			float threshold;
			sscanf_s(stageNode.child("stage_threshold").first_child().value(), "%f", &threshold );
			
			// Create a stage object
			Stage^ stage = ref new Stage(threshold);

			// Create depth-first iterator to skip over all the <trees> and <_> nonsense:
			struct featureWalker : xml_tree_walker {
				virtual bool for_each( xml_node& node ) {
					if( strcmp(node.name(), "feature") == 0 ) {
						// First, grab the threshold, left_val and right_val
						float threshold, left_val, right_val;
						xml_node parent = node.parent();
						sscanf_s(parent.child("threshold").first_child().value(), "%f", &threshold );
						sscanf_s(parent.child("left_val").first_child().value(),  "%f", &left_val );
						sscanf_s(parent.child("right_val").first_child().value(), "%f", &right_val );

						// Create the Feature object
						Feature^ feature = ref new Feature(threshold, left_val, right_val );

						// Loop through RectWeights
						for( xml_node rectNode : node.child("rects").children("_") ) {
							float weight;
							float x, y, width, height;

							// Scan in the parameters for the RectWeight
							sscanf_s(rectNode.first_child().value(), "%f %f %f %f %f", &x, &y, &width, &height, &weight);

							// Add it to our feature
							feature->addRectWeight( ref new RectWeight( Rect( x, y, width, height), weight ) );
						}

						// Once we're done, add our feature to our stage:
						stage->addFeature( feature );
					}
					return true;
				}

				Stage^ stage;
			};

			featureWalker fWalker;
			fWalker.stage = stage;
			stageNode.traverse( fWalker );

			// Once we're done with this stage, add it to our vector of stages!
			this->addStage( stage );
		}
	});
}

Detector::~Detector() {
	if( this->integral.data() )
		delete (unsigned int*)this->integral.data();
	if( this->squared.data() )
		delete (unsigned int*)this->squared.data();
	if( this->valscale.data() )
		delete (unsigned int*)this->valscale.data();
}

// Calculates an integral image from a frame, in-place.  Operates on a "valscale" image.
void calcIntegral( Frame& in, Frame& out ) {
	// Initialize first row and first column
	for( unsigned int x=1; x<in.width(); ++x )
		out(x,0).val = in(x,0).val + in(x-1,0).val;
	// Boo-hoo, cache locality.... oh well, at least we re-use the previous value
	for( unsigned int y=1; y<in.height(); ++y )
		out(0,y).val = in(0,y).val + in(0,y-1).val;

	for( unsigned int y=1; y<in.height(); ++y ) {
		for( unsigned int x=1; x<in.width(); ++x ) {
			out(x,y).val = in(x,y).val + out(x,y-1).val + out(x-1,y).val - out(x-1,y-1).val;
		}
	}
}

// Calculates the square of an image.  Operates on a "valscale" image
void squareFrame( Frame& in, Frame& out ) {
	for( unsigned int y=0; y<in.height(); ++y ) {
		for( unsigned int x=0; x<in.width(); ++x ) {
			out(x,y).val = in(x,y).val*in(x,y).val;
		}
	}
}

// Converts a color image into "valscale", where each pixel is represented by its grayscale
// value in the "val" component, so we can overflow all the way up to awesomeness
void convertToValscale( Frame& in, Frame& out ) {
	for( unsigned int y=0; y<in.height(); ++y ) {
		for( unsigned int x=0; x<in.width(); ++x ) {
			out(x,y).val = (30*in(x,y).red + 59*in(x,y).green + 11*in(x,y).blue)/100;
		}
	}
}

Frame& Detector::checkArraySize( Frame& f, Frame& check ) {
	if( check.width() != f.width() || check.height() != f.height() ) {
		if( this->integral.data() )
			delete (unsigned int *) check.data();
		check = f.makeImitator();
	}
	return check;
}


Platform::Array<Rect>^ Detector::findFaces(unsigned int width, unsigned int height, uintptr_t dataPtr, unsigned int downsampleFactor, float scaleStep, float shiftStep ) {
	Frame f( width, height, dataPtr, downsampleFactor );
	// The variable we'll store our faces in
	std::vector<Rect> faces;

	// This will allocate them on the first time, and hopefully never again after that!
	integral = checkArraySize( f, integral );
	squared = checkArraySize( f, squared );
	valscale = checkArraySize( f, valscale );

	// Calculate valscaled image, and then from that the integral image, and squaredIntegral image
	convertToValscale(f, valscale);
	calcIntegral(valscale, integral);
	squareFrame( valscale, squared );
	calcIntegral( squared, squared );

	float maxScale = min(f.height()/size.Height, f.width()/size.Width);

	// Loop through scales
	for( float scale = 1.0f; scale < maxScale; scale *= (1.0 + scaleStep) ) {
		// Calculate the width and height of this cascade at this scale
		unsigned int width = (unsigned int)(size.Width * scale);
		unsigned int height = (unsigned int)(size.Height * scale);
			
		// Calculate the inverse area of our feature at the asked for scale
		float inv_area = 1.0f/((size.Width * scale) * (size.Height * scale));

		// Loop through different window starting positions
		for( unsigned int x = 0; x < f.width() - width; x += (unsigned int)(width*shiftStep) ) {
			//OutputDebugString( String::Concat(x, L"\n")->Data() );
			for( unsigned int y = 0; y < f.height() - height; y += (unsigned int)(height*shiftStep) ) {
			
				// Calculate the total integrated value covered by this position/scale
				unsigned int total = integral(x + width,y + height) - integral(x + width, y) - integral(x,y + height) + integral(x,y);
				unsigned int total_sq = squared(x + width,y + height) - squared(x + width, y) - squared(x,y + height) + squared(x,y);

				// Calculate the mean and standard deviation of this position/scale
				float mean = total * inv_area;
				float stddev = sqrtf(total_sq*inv_area - mean*mean);

				// To test a single position, we must pass all of our stages
				unsigned int stageIdx;
				for( stageIdx = 0; stageIdx < stages.size(); stageIdx++ ) {
					float stage_sum = 0.0f;

					// Within a single stage, we must loop over the features to calculate the stage_sum
					const std::vector<Feature^> * features = stages[stageIdx]->getFeatures();
					for( unsigned int featureIdx = 0; featureIdx < features->size(); ++featureIdx ) {
						Feature^ feature = (*features)[featureIdx];

						// Loop over all rectangles in this feature, sum their responses into feature_sum:
						float feature_sum = 0.0;
						const std::vector<RectWeight^> * rects = feature->getRectWeights();
						for( unsigned int i = 0; i < rects->size(); ++i ) {
							RectWeight^ rw = (*rects)[i];
							Rect r = rw->getRect();
				
							// Calculate this rectangle's value from the integral, then multiply by weight
							feature_sum +=	((float)integral((unsigned int)(x + (r.X + r.Width)*scale), (unsigned int)(y + (r.Y + r.Height)*scale)) +
											(float)integral((unsigned int)(x + r.X*scale), (unsigned int)(y + r.Y*scale)) -
											(float)integral((unsigned int)(x + (r.X + r.Width)*scale), (unsigned int)(y + r.Y*scale) ) -
											(float)integral((unsigned int)(x + r.X*scale), (unsigned int)(y + (r.Y + r.Height)*scale) ) ) *
											rw->getWeight();
						}

						// Decide if we should add on left or right from this feature_sum and our stored threshold:
						if( feature_sum*inv_area < feature->getThreshold() * stddev )
							stage_sum += feature->getLeftValue();
						else
							stage_sum += feature->getRightValue();
					}

					// If we failed this stage, exit immediately
					if( stage_sum <= stages[stageIdx]->getThreshold() )
						break;
				}

				// This means we passed!  Add this position/scale to the list of faces!
				if( stageIdx >= stages.size() )
					faces.push_back( Rect( (float)x*downsampleFactor, (float)y*downsampleFactor, (float)width*downsampleFactor, (float)height*downsampleFactor ) );
			}
		}
	}

	Array<Rect>^ ret = ref new Array<Rect>(faces.size());
	for( unsigned int i=0; i<faces.size(); ++i )
		ret[i] = faces[i];
	return ret;
}