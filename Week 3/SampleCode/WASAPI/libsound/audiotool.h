#pragma once

/*
    AudioTool is a utility class designed to make dealing with audio data easier
    It provides simple buffer manipulations to array objects for things such as
    converting from mono -> stero, generation of sounds, and simple calculations

    This class can serve as a good learning tool for how to pass around and deal
    with the Platform::Array<float> construct.  Feel free to add new functions
    to serve your purposes in homeworks and projects.
*/

namespace libsound
{
    public ref class AudioTool sealed
    {
    public:
		// Initialize AudioTool with the parameters of the output audio format
        AudioTool( unsigned int numChannels, unsigned int samplerate );
		virtual ~AudioTool();


		/////////////// SIMPLE BUFFER MANIPULATIONS ///////////////
		// This converts a source buffer into something the output can handle (e.g. performs upmixing or downmixing if srcNumChannels != this->numChannels)
		Platform::Array<float>^ convertChannels( const Platform::Array<float>^ data, unsigned int srcNumChannels );

		// Finds the energy in a Platform::Array<float>^
        // Note: performs the analysis along each channel independently, returning an array of lenth numChannels
		Platform::Array<float>^ calcStandardDeviation( const Platform::Array<float>^ data );



		/////////////// SOUND GENERATION ROUTINES ///////////////
		// Generate random noise using standard library
		Platform::Array<float>^ randn( unsigned int numSamples );

		// Generate a sin wave that lasts [numSamples] and oscillates [numOscilllations] times
		Platform::Array<float>^ sin( unsigned int numSamples, float freq );
		Platform::Array<float>^ sin( unsigned int numSamples, float freq, float startingPhase );

		// Returns [0,0,0,0,.....,0]
		Platform::Array<float>^ silence( unsigned int numSamples );
	private:
		// these are the output audio parameters, which allow us to generate the proper types of audio
		unsigned int numChannels, samplerate;
    };
}
