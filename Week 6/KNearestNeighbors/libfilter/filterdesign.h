#pragma once

using namespace Platform;
using namespace Windows::Foundation::Metadata;

namespace libfilter
{
	// Defines different window types: Rectangular, Hann, Hamming and Blackman
	public enum class WindowType {
		RECT,
		HANN,
		HAMMING,
		BLACKMAN,
	};

    public ref class FilterDesign sealed
    {
    public:
        FilterDesign();

		// This will design an FIR bandpass filter at the desired frequency, with the desired bandwidth
		// Both parameters should be within [0....1], where 1 represents the nyquist frequency. A
		// bandwidth parameber of 0.5 means the filter will drop to -3dB at centerFreq +- 0.5f. This
		// function returns the filter coefficients in a Platform::Array, if you want a raw native
		// array of them, check out the overloaded FIRDesignWindowed in the internal section.
		// [DefaultOverload] is a C++/CX thing; it's just used to squash a warning, omitting it is fine.
		// If you want to know more go here: http://msdn.microsoft.com/en-us/library/windows/apps/hh441569.aspx 
		[DefaultOverload] Array<float>^ FIRDesignWindowed( float centerFreq, float bandwidth, WindowType wtype );

		// Alternatively, you can specify the length as well as the bandwidth, if you want to try for
		// a certain bandwidth given a hard limit on the length
		Array<float>^ FIRDesignWindowed( float centerFreq, float bandwidth, unsigned int length, WindowType wtype );

		// Returns the length of a filter with the given bandwidth for the given window type
		unsigned int getWindowLength( float bandwidth, WindowType wtype );

		// Returns the bandwidth of a filter with the given length for the given window type
		float getWindowBandwidth( unsigned int N, WindowType wtype );

		// Creates a window of the given type (Check out the WindowType defined above) and given length
		Array<float>^ createWindow( unsigned int N, WindowType wtype );

	// This means that these methods are not accessible from C#, only from C++, which is good as we're asking
	// for things like float * arrays and stuff which C# doesn't like to deal with
	internal:
		// Creates a window, note that we pass in the window array this time.  This is so that all
		// memory management happens on the calling-code side of things, not inside FilterDesign
		void createWindow( float * window, unsigned int N, WindowType wtype );

		// This is what actually does the work inside of FIRDesignWindowed, but it stores everything
		// inside a float * and is internal instead of public.  This is because it uses native C++ types
		// instead of the interop datatypes like Platform::Array, and as such it cannot be public.
		void FIRDesignWindowed( float * filter, float centerFreq, float bandwidth, unsigned int N, WindowType wtype );
    };
}