/*
These classes are meant to aid in image processing.  They provide a simple,
efficient interface for accessing frames of pixel data.  They assume an input
format of DXGI_FORMAT_B8G8R8A8_UNORM, and should provide a transparent means
of accessing pixel data.

Frame is used to abstract an array of pixel data.  To construct it, use:

	Frame f( width, height, dataPtr );

This creates a frame object pointing to the data pointed to by dataPtr. To
access pixel data, use the () operator, and the .width() and .height() members

	for( unsigned int y=0; y<f.height(); ++y ) {
		for( unsigned int x=0; x<f.width(); ++x ) {
			f(x,y) = 0;
		}
	}

The () operator returns a Pixel object.  Pixel is a convenience type that wraps
around an unsigned int, and provides .red, .green, .blue and .alpha members.
Use those members to access the 8-bit values, and use .val to access the 32-bit
integer internally. This pixel object means you can write code like so:

	f(x,y).red = (f(x,y).green + f(x,y).blue)/2

Please note that if you write code that assigns a number directly to a Pixel
object, it will interpret that number as a 32-bit value, as follows:

	f(x,y) = 0xffffffff;  // Will assign this pixel to be white
	f(x,y) = 255;         // Is the same as assigning 0x000000ff



Finally, this class has a simple method for downsampling your data when your
algorithms are not fast enough to process every pixel individually. To do so,
when creating a Frame object, pass in the optional parameter "downsampleFactor"
and the Frame will "skip over" some pixels when indexing.  As an example:

	Frame f(width, height, dataPtr, 2);
	for( unsigned int y=0; y<f.height(); ++y ) {
		for( unsigned int x=0; x<f.width(); ++x ) {
			f(x,y) = 0;
		}
	}

Note that this example is identical to above, except for the "2" passed in to
the Frame constructor.  This will cause Frame to skip over every other pixel,
additionally .height() and .width() will return sizes half as large, and so all
loops over the entire image will "just work", setting every other pixel to 0.
This obviously is probably not what you intend, but at least it is performing
an action you don't intend 4 times faster than it would otherwise have. This
downsampling is most useful during analysis stages, when actually modifying
data, you are best off using a nondownsampled image.

Finally, to output a Frame to a TextureGraphInterop object, you may use the
.data() member function to get the address of the internal data array

	texInterop->setTexturePtr( f.width(), f.height(), f.data() );
*/
#pragma once

#include <string.h>



// Useful #define's:
#ifndef min
#define min(a,b) ((a)>(b)?(b):(a))
#endif

#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

class Pixel {
public:
  union {
		struct {
			unsigned char blue, green, red, alpha;
		};
		unsigned int val;
	};

	Pixel( const unsigned int val ) {
		this->val = val;
	}

	Pixel( const unsigned char alpha, const unsigned char red, const unsigned char green, const unsigned char blue ) {
		this->alpha = alpha;
		this->red = red;
		this->green = green;
		this->blue = blue;
	}

	operator unsigned int() {
		return this->val;
	}

	void blendIn( const Pixel& other ) {
		//float alphaMix = other.alpha*(255 - this->alpha)/255.0f;
		this->blue = (unsigned char)(this->blue * (255 - other.alpha)/255.0f + other.blue*other.alpha/255.0f);
		this->green = (unsigned char)(this->green * (255 - other.alpha)/255.0f + other.green*other.alpha/255.0f);
		this->red = (unsigned char)(this->red * (255 - other.alpha)/255.0f + other.red*other.alpha/255.0f);
	}

};

class Frame {
public:
	// Creates an "empty" frame, not typically useful
	Frame() {
		this->ptr = NULL;
		this->_width = this->_height = 0;
		this->ds = 1;
	}

	// Create a frame from the given pointer, with the given height and width
	Frame(unsigned int width, unsigned int height, uintptr_t dataPtr ) {
		this->ptr = (unsigned int*)dataPtr;
		this->_width = width;
		this->_height = height;
		this->ds = 1;
	}

	// Same as above, but include a downsampling factor
	Frame(unsigned int width, unsigned int height, uintptr_t dataPtr, int downsampleFactor) {
		this->ptr = (unsigned int*)dataPtr;
		this->_width = width;
		this->_height = height;
		this->ds = downsampleFactor;
	}

	// Overload the () operator so we have a convenient frame(x,y) syntax
	__inline Pixel& operator() (const unsigned int x, const unsigned int y) {
		return (Pixel&)this->ptr[y*_width*ds + x*ds];
	}

	// Get the pointer associated with this Frame
	const uintptr_t data() {
		return (uintptr_t)ptr;
	}

	// Get the width of this frame
	const unsigned int width() {
		return _width/ds;
	}

	// Get the height of this frame
	const unsigned int height() {
		return _height/ds;
	}

	const unsigned int downsampleFactor() {
		return ds;
	}

	// Return a copy of this frame into another one, so they are independent
	Frame makeCopy( void ) {
		unsigned int * newData = new unsigned int[_width*_height];
		memcpy( newData, ptr, sizeof(unsigned int)*_width*_height );
		return Frame(_width, _height, (uintptr_t)newData, ds);
	}

	// Returns a new Frame, but doesn't copy the memory over
	Frame makeImitator( void ) {
		unsigned int * newData = new unsigned int[_width*_height];
		return Frame(_width, _height, (uintptr_t)newData, ds);
	}
private:
	unsigned int _width, _height;

	// This is the downsample factor, allows us to "skip" pixels, when we need more speed.
	unsigned int ds;
	unsigned int * ptr;
};
