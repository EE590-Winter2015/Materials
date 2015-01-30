// libAudioTool.cpp
#include "AudioTool.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>

using namespace libsound;
using namespace Platform;

// Defined for randn()
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

AudioTool::AudioTool( unsigned int numChannels, unsigned int samplerate )
{
	this->numChannels = numChannels;
	this->samplerate = samplerate;
}

AudioTool::~AudioTool()
{
	this->numChannels = 0;
	this->samplerate = 0;
}

Platform::Array<float>^ AudioTool::convertChannels( const Platform::Array<float>^ data, unsigned int srcNumChannels ) {
	// If the channels are the same, just pass the original data back
	if( srcNumChannels == this->numChannels )
		return *((Platform::Array<float>^ *) &data);	// I feel dirty just for writing this

	// Otherwise, we need to fix up the number of channels
	auto newData = ref new Platform::Array<float>(this->numChannels*data->Length);

	// If we're going from 1 channel to more (e.g. 2), then we'll just copy it!
	if( srcNumChannels == 1 ) {
		for( unsigned int i=0; i<data->Length; ++i ) {
			for( unsigned int k=0; k<this->numChannels; ++k ) {
				newData[this->numChannels*i + k] = data[i];
			}
		}
	}

	// Otherwise, if we're going from a multi-channel mix to a single-channel, just average everything together
	if( this->numChannels == 1 ) {
		for( unsigned int i=0; i<data->Length; ++i ) {
			newData[i] = 0;
			for( unsigned int k=0; k<srcNumChannels; ++k ) {
				newData[i] += data[srcNumChannels*i + k];
			}
		}
	}
	return newData;
}

Platform::Array<float>^ AudioTool::calcStandardDeviation( const Platform::Array<float>^ data ) {
    // Get pointer to data inside of C# array class
    float * realData = data->Data;

    auto energy = ref new Platform::Array<float>(this->numChannels);
    auto mean = ref new Platform::Array<float>(this->numChannels);

    // Since we're using these in an inner loop, we want to deal with the data directly
    float * e = energy->Data;
    float * m = mean->Data;

    for( unsigned int i=0;i<data->Length/this->numChannels; ++i ) {
        for( unsigned int k=0;k<this->numChannels; ++k ) {
            m[k] += realData[this->numChannels*i];
            e[k] += realData[this->numChannels*i]*realData[this->numChannels*i];
        }
    }

    // Energy of IID process `x` is `E[x^2] - E[x]^2`, so subtract out the means now:
    for( unsigned int k=0;k<this->numChannels; ++k )
        e[k] -= m[k]*m[k];

    // Finally, return the energy
    return energy;
}



Platform::Array<float>^ AudioTool::randn( unsigned int numSamples ) {
	// Buffer to store output data
	Platform::Array<float>^ data = ref new Platform::Array<float>(numSamples);

	// Use stdlib's random number generation
	std::default_random_engine generator;

    // We ask for a normally distributed random variable with 0 mean, 1/4 stddev
	std::normal_distribution<float> randn(0.0,.25);
	for( unsigned int i=0; i<numSamples; ++i ) {
        // We clip values greater or less than 1, on the off chance those occur
		data[i] = max(min(randn(generator), 1.0f), -1.0f);
	}

	// Send the data back!
	return data;
}

Platform::Array<float>^ AudioTool::silence( unsigned int numSamples ) {
	auto data = ref new Platform::Array<float>(this->numChannels*numSamples);
	memset( data->Data, 0, sizeof(float)*data->Length );
	return data;
}

Platform::Array<float>^ AudioTool::sin( unsigned int numSamples, float freq ) {
	return AudioTool::sin( numSamples, freq, 0.0f );
}

Platform::Array<float>^ AudioTool::sin( unsigned int numSamples, float freq, float startingPhase  ) {
	auto data = ref new Platform::Array<float>(this->numChannels*numSamples);
	for( unsigned int i=0; i<numSamples; ++i ) {
        // Generate the sin wave on the zeroeth channel
		data[this->numChannels*i+0] = .5f*sinf(i*freq*2*M_PI/this->samplerate + startingPhase);

        // Copy it onto all other channels, if they exist
		for( unsigned int k=1; k<this->numChannels; ++k )
			data[this->numChannels*i+k] = data[this->numChannels*i+0];
	}
	return data;
}
