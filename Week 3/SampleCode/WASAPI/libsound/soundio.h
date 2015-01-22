#pragma once

#include <audioclient.h>
#include <phoneaudioclient.h>

/*
    This class presents a more complete (and useful) API over the sound hardware
    in WP8.  The biggest difference between this class and the API developed in
    Homework 2 is that now we understand how to use delegates, events, etc... we
    are going to be using those to expose events to C# and C++/CX code.
*/

using namespace Windows::Foundation;

namespace libsound
{
	// Callback definitions
	public delegate void AudioInCallback(const Platform::Array<float>^ data);
	public delegate Platform::Array<float>^ AudioOutCallback( unsigned int numSamples );


    public ref class SoundIO sealed
    {
    public:
		// Opens up the audio devices and starts threads for input/output
		SoundIO();
		virtual ~SoundIO();

		// Starts callback threads, etc...
		void start();

		// Stops the callback threads, etc...
		void stop();

		// Methods used to query the audio output format properties
		unsigned int getOutputBufferSize();
		unsigned int getOutputSampleRate();
		unsigned int getOutputNumChannels();
		unsigned int getOutputBitdepth();

		// Methods used to query the audio input format properties
		unsigned int getInputBufferSize();
		unsigned int getInputSampleRate();
		unsigned int getInputNumChannels();
		unsigned int getInputBitdepth();

		// Feed a chunk of audio out to the device.  Returns number of samples written
		unsigned int writeAudio( const Platform::Array<float>^ data );

		// Read in a chunk of audio from the microphone, returning it as a C# array
		Platform::Array<float>^ readAudio( void );

		// Event interface! Yeah!
		event AudioInCallback^ audioInEvent;
		event AudioOutCallback^ audioOutEvent;
	private:
		// Audio devices and clients
		IAudioClient2 *outputDevice, *inputDevice;
		IAudioRenderClient *outputClient;
		IAudioCaptureClient *inputClient;

		// Formats of audio streams for capture and playback
		WAVEFORMATEX *outputFormat, *inputFormat;

		// Buffer lengths for input and output in samples
		unsigned int outputBufferLen, inputBufferLen;

		// Threads that get audio input and call their callback, or call their callbacks and output to the sound card
		void audioInThread( IAsyncAction^ operation );
		void audioOutThread( IAsyncAction^ operation );

		// Handles to the threads so we can close them down when we die
		IAsyncAction ^audioInThreadHandle, ^audioOutThreadHandle;

		// Handles that are used to tell if an audio buffer is ready to be read or outputted to
		HANDLE audioInReady, audioOutReady;
    };
}
