// ImageProcessingPart2.cpp
#include "ImageProcessing.h"

using namespace FaceDetection;
using namespace Platform;
using namespace Windows::Foundation;


ImageProcessing::ImageProcessing( Detector^ d )
{
	this->d = d;
}

void ImageProcessing::processFrame( unsigned int width, unsigned int height, uintptr_t dataPtr )
{
	Frame f(width, height, dataPtr, 1);

	Array<Rect>^ faces = d->findFaces( width, height, dataPtr, 5, .25f, .2f );
	for( Rect r : faces ) {
		for( int y = r.Top; y< r.Bottom; ++y ) {
			for( int x = r.Left; x<r.Right; ++x ) {
				// Shade it yellow
				f(x,y).blendIn(0x99ffff00);
			}
		}
	}
	frameProcessed( width, height, dataPtr );
}