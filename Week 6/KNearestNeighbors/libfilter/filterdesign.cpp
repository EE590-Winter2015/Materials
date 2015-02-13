// libfilter.cpp
#define _USE_MATH_DEFINES
#include <math.h>
#include "filterdesign.h"

using namespace libfilter;
using namespace Platform;

// the only reason this is a function is because I need the if statement when x == 0.0f
float sinc( float x ) {
	if( x == 0.0f )
		return 1.0f;
	return sinf((float)M_PI*x)/((float)M_PI*x);
}

FilterDesign::FilterDesign()
{
}

Array<float>^ FilterDesign::FIRDesignWindowed( float centerFreq, float bandwidth, WindowType wtype ) {
	// This is just a convenience method to allow passing in the bandwidth instead of the length of the filter
	return FIRDesignWindowed( centerFreq, bandwidth, getWindowLength( bandwidth, wtype ), wtype );
}

Array<float>^ FilterDesign::FIRDesignWindowed( float centerFreq, float bandwidth, unsigned int N, WindowType wtype ) {
	// Create an array to store the filter in
	Array<float>^ filter = ref new Array<float>(N);

	// Pass that array off to FIRDesignWindowed(), pretending it's a native array
	FIRDesignWindowed( filter->Data, centerFreq, bandwidth, N, wtype );

	// return it to managed code
	return filter;
}

Array<float>^ FilterDesign::createWindow( unsigned int N, WindowType wtype ) {
	// Create an array to store the filter in
	Array<float>^ window = ref new Array<float>(N);

	// Pass that array off to createWindow(), pretending it's a native array
	createWindow( window->Data, N, wtype );

	// Return it to managed code
	return window;
}

void FilterDesign::FIRDesignWindowed( float * filter, float centerFreq, float bandwidth, unsigned int N, WindowType wtype ) {
	// First, we fill filter with our requested filter:
	createWindow( filter, N, wtype );

	// Next, we multiply in a sinc wave of the requested length, centered in the middle of our buffer
	for( unsigned int i=0; i<N; ++i )
		filter[i] *= sinc(bandwidth*(i - N/2.0f));

	// Next, we modulate it up to the desired center frequency:
	if( centerFreq != 0.0f ) {
		for( unsigned int i=0; i<N; ++i )
			filter[i] *= cosf((float)M_PI*centerFreq*i);
	}

	// Finally, normalize to unit energy:
	float energy = 0.0f;
	for( unsigned int i=0; i<N; ++i )
		energy += filter[i]*filter[i];

	for( unsigned int i=0; i<N; ++i )
		filter[i] /= energy;
}

unsigned int FilterDesign::getWindowLength( float bandwidth, WindowType wtype ) {
	switch( wtype ) {
		case WindowType::RECT:
			return (unsigned int)ceil(4*M_PI/bandwidth);
		case WindowType::HANN:
		case WindowType::HAMMING:
			return (unsigned int)ceil(8*M_PI/bandwidth);
		case WindowType::BLACKMAN:
			return (unsigned int)ceil(12*M_PI/bandwidth);
	}
	// This should never happen
	return 0;
}

float FilterDesign::getWindowBandwidth( unsigned int N, WindowType wtype ) {
	switch( wtype ) {
		case WindowType::RECT:
			return 4*(float)M_PI/N;
		case WindowType::HANN:
		case WindowType::HAMMING:
			return 8*(float)M_PI/N;
		case WindowType::BLACKMAN:
			return 12*(float)M_PI/N;
	}
	// This should never happen
	return 0;
}

void FilterDesign::createWindow( float * window, unsigned int N, WindowType wtype ) {
	switch( wtype ) {
		case WindowType::RECT:
			// Create a rectangular window (all ones)
			for( unsigned int i=0; i<N; ++i )
				window[i] = 1.0f;
			break;
		case WindowType::HANN:
			// Create a Von Hann window
			for( unsigned int i=0; i<N; ++i )
				window[i] = 0.5f*(1 - cosf(2*(float)M_PI*i/(N-1.0f)));
			break;
		case WindowType::HAMMING:
			// Create a Hamming window
			for( unsigned int i=0; i<N; ++i )
				window[i] = 0.54f - 0.46f*cosf(2*(float)M_PI*i/(N-1.0f));
			break;
		case WindowType::BLACKMAN:
			// Create a Blackman window (This is the "exact" Blackman window, with alpha = 0.16)
			// If you want a different Blackman window, please extend this code!
			const float a0 = 7938/18608.0f;
			const float a1 = 9240/18608.0f;
			const float a2 = 1430/18608.0f;
			float arg;
			for( unsigned int i=0; i<N; ++i ) {
				arg = 2*(float)M_PI*i/(N-1.0f);
				window[i] = a0 - a1*cosf(arg) + a2*cosf(2*arg);
			}
			break;
	}
}