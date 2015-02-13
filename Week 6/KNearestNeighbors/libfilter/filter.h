#pragma once
//#include "complex.h"

using namespace Platform;
using namespace Windows::Foundation::Metadata;

namespace libfilter
{
    public ref class Filter sealed
    {
    public:
		// Create a filter with an initial impulseResponse of [0 0 0 0 0 0 0 0]
        Filter(const Array<float>^ impulseResponse);

		// Clean everything up, releasing all internally created buffers, etc...
		virtual ~Filter();

		// Feed one new sample into the filter, return the new sample of output
		float filter( float data );

		// Convolve every sample in the input array, returning the array of new output samples
		// Don't worry about the [DefaultOverload] thing, it's just to squash a warning
		[DefaultOverload] Array<float>^ filter( const Array<float>^ data );

		// Return our internal impulseResponse
		Array<float>^ getImpulseResponse();

		// Update our internal impulseResponse
		//void updateImpulseResponse( const Platform::Array<float>^ impulseResponse );
	private:
		float *impulseResponse, *prevData;
		unsigned int idx;
		unsigned int N;
	};
}
