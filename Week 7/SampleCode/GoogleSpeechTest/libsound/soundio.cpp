// libsoundio.cpp
#include "soundio.h"
#include <stdio.h>
#include <math.h>

using namespace libsound;

// Need this for our Platform:: types
using namespace Platform;

// Need this for our threading types such as ThreadPool
using namespace Windows::System::Threading;



SoundIO::SoundIO()
{
	HRESULT hr = E_FAIL;

	// First, we'll open the capture device
	auto deviceId = GetDefaultAudioCaptureId( AudioDeviceRole::Default );
	auto classId = __uuidof(IAudioClient2);
	hr = ActivateAudioInterface( deviceId, classId, (void **)&inputDevice );
	if( FAILED(hr) ) {
		throw ref new FailureException("Could not open default audio capturer!");
	}

	// This gets the audio format that is used internally by the mixing engine
	// We will go ahead and just use that format
	hr = inputDevice->GetMixFormat(&inputFormat);
	if( FAILED(hr) ) {
		throw ref new FailureException( "Could not get mix format!" );
	}
	if( inputFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE ||
		((WAVEFORMATEXTENSIBLE*)inputFormat)->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) {
		throw ref new FailureException( "Audio format is not float!" );
	}

	// Always asking for event-based processing, as we're using C# events now!
	unsigned int flags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK;

	// Initialize the capture device with the format we've specified
	hr = inputDevice->Initialize( AUDCLNT_SHAREMODE_SHARED, flags, 0, 0, inputFormat, NULL );
	if( FAILED(hr) ) {
		throw ref new FailureException( "Could not initialize capture device!" );
	}

	// Get the size of the current buffer
	hr = inputDevice->GetBufferSize( &inputBufferLen );
	if( FAILED(hr) ) {
		OutputDebugString( L"Cannot get input buffer size!\n" );
	}

	// Get the capture client
	hr = inputDevice->GetService(__uuidof(IAudioCaptureClient), (void**)&inputClient);
	if( FAILED(hr) ) {
		throw ref new FailureException( "Could not get capture client!" );
	}



	// First, activate default audio render device
	hr = ActivateAudioInterface( GetDefaultAudioRenderId( AudioDeviceRole::Default ), __uuidof(IAudioClient2), (void **)&outputDevice );
	if( FAILED(hr) ) {
		throw ref new FailureException("Could not open default audio renderer!");
	}

	// This gets the audio format that is used internally by the mixing engine
	// We will go ahead and just use that format to spit out onto the audio device
	hr = outputDevice->GetMixFormat(&outputFormat);
	if( FAILED(hr) ) {
		throw ref new FailureException( "Could not get mix format!" );
	}
	if( outputFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE ||
		((WAVEFORMATEXTENSIBLE*)outputFormat)->SubFormat != KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) {
		throw ref new FailureException( "Audio format is not float!" );
	}

	// Initialize the render device with the format we've specified
	hr = outputDevice->Initialize( AUDCLNT_SHAREMODE_SHARED, flags, 0, 0, outputFormat, NULL );
	if( FAILED(hr) ) {
		throw ref new FailureException( "Could not initialize output device!" );
	}

	// Get the size of the current buffer, in frames (that is samples*channels)
	hr = outputDevice->GetBufferSize( &outputBufferLen );
	if( FAILED(hr) ) {
		OutputDebugString( L"Cannot get buffer size!\n" );
	}

	// Get the render client
	hr = outputDevice->GetService(__uuidof(IAudioRenderClient), (void**)&outputClient);
	if( FAILED(hr) ) {
		throw ref new FailureException( "Could not get render client!" );
	}

	// Start up a thread to wait for input events, if the user wants that
	// First, create event to know when we have audio to capture!
	audioInReady = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
    if (NULL == audioInReady) {
		OutputDebugString( L"Couldn't create audioInEvent\n" );
	} else {
		// register the event
		hr = inputDevice->SetEventHandle(audioInReady);
		if( FAILED(hr) ) {
			OutputDebugString( L"Couldn't register audioInEvent!\n" );
		}

		// Start the thread with a high priority, and timesliced
		audioInThreadHandle = ThreadPool::RunAsync( ref new WorkItemHandler(this, &SoundIO::audioInThread), WorkItemPriority::High, WorkItemOptions::TimeSliced );
	}

	audioOutReady = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);
    if (NULL == audioOutReady) {
		OutputDebugString( L"Couldn't create audioInEvent\n" );
	} else {
		// register the event
		hr = outputDevice->SetEventHandle(audioOutReady);
		if( FAILED(hr) ) {
			OutputDebugString( L"Couldn't register audioInEvent!\n" );
		}

		// Start the thread with a high priority, and timesliced
		audioOutThreadHandle = ThreadPool::RunAsync( ref new WorkItemHandler(this, &SoundIO::audioOutThread), WorkItemPriority::High, WorkItemOptions::TimeSliced );
	}
}

SoundIO::~SoundIO() {
	this->stop();

	// Clean up threads here
	if( this->audioInThreadHandle != nullptr )
		this->audioInThreadHandle->Cancel();

	if( this->audioOutThreadHandle != nullptr )
		this->audioOutThreadHandle->Cancel();

	while( this->audioInThreadHandle != nullptr ) {
		WaitForSingleObjectEx(GetCurrentThread(), 1, FALSE);
	}

	while( this->audioInThreadHandle != nullptr ) {
		WaitForSingleObjectEx(GetCurrentThread(), 1, FALSE);
	}
}

void SoundIO::start() {
	// Start 'em up!
	HRESULT hr = inputDevice->Start();
	if( FAILED(hr) ) {
		OutputDebugString( L"Couldn't start capture device!\n" );
	}

	// Him too, I suppose...
	hr = outputDevice->Start();
	if( FAILED(hr) ) {
		OutputDebugString( L"Couldn't start render device!\n" );
	}
}

void SoundIO::stop() {
	inputDevice->Stop();
	outputDevice->Stop();
}

unsigned int SoundIO::getOutputBufferSize() {
	return this->outputBufferLen;
}

unsigned int SoundIO::getOutputSampleRate() {
	return this->outputFormat->nSamplesPerSec;
}

unsigned int SoundIO::getOutputNumChannels() {
	return this->outputFormat->nChannels;
}

unsigned int SoundIO::getOutputBitdepth() {
	return this->outputFormat->wBitsPerSample;
}



unsigned int SoundIO::getInputBufferSize() {
	return this->inputBufferLen;
}

unsigned int SoundIO::getInputSampleRate() {
	return this->inputFormat->nSamplesPerSec;
}

unsigned int SoundIO::getInputNumChannels() {
	return this->inputFormat->nChannels;
}

unsigned int SoundIO::getInputBitdepth() {
	return this->inputFormat->wBitsPerSample;
}

unsigned int SoundIO::writeAudio( const Platform::Array<float>^ data ) {
	if( data == nullptr || data->Length == 0 )
		return 0;

	// padding is the amount into the buffer we must start writing, the total
	// amount of data we can write is bufferLen - padding
	unsigned int padding = 0;
	HRESULT hr = outputDevice->GetCurrentPadding(&padding);
	if( FAILED(hr) ) {
		throw ref new FailureException("Could not get padding for render device!");
	}

	// Calculate how many samples to write
	int numSamplesToWrite = data->Length/outputFormat->nChannels;
	int numSamplesCanWrite = min( (int)(this->outputBufferLen - padding), numSamplesToWrite );
	if( numSamplesCanWrite < numSamplesToWrite ) {
		OutputDebugString( L"Not writing buffer, as not enough space in output buffer!\n" );
	}

	// If we actually have space to write something, let's do it!
	if( numSamplesCanWrite >= numSamplesToWrite ) {
		// Get the output buffer
		void * outputBuff = NULL;
		hr = outputClient->GetBuffer( numSamplesToWrite, (unsigned char **)&outputBuff );
		if( FAILED(hr) ) {
			OutputDebugString( L"Could not GetBuffer() for render client!\n" );
			return 0;
		}

		// Write out the data!
		memcpy( outputBuff, data->begin(), numSamplesToWrite*outputFormat->nBlockAlign );

		// Release the buffer (this is important to do as quickly as possible,
		// so the OS can start using the data in the buffer!)
		outputClient->ReleaseBuffer( numSamplesToWrite, 0 );
		return numSamplesToWrite;
	}

	// Return the number of samples we wrote, or 0 if we couldn't write any
	return 0;
}


// Read audio in from the mic, return it in an array
Platform::Array<float>^ SoundIO::readAudio() {
	unsigned char * rawData;
	unsigned int numSamples;
	unsigned long flags;

	HRESULT hr = inputClient->GetBuffer( &rawData, &numSamples, &flags, NULL, NULL );
	if( FAILED(hr) ) {
		OutputDebugString( L"Could not GetBuffer() for capture client!" );
	}

	Platform::Array<float>^ data = nullptr;
	if( numSamples > 0 ) {
		data = ref new Platform::Array<float>(numSamples);
		if( flags & AUDCLNT_BUFFERFLAGS_SILENT ) {
			memset( data->begin(), 0, numSamples*inputFormat->nBlockAlign );
		} else {
			memcpy( data->begin(), rawData, numSamples*inputFormat->nBlockAlign );
		}

		if( flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY ) {
			OutputDebugString( L"Data discontinuity!\n" );
		}
		if( flags & AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR ) {
			OutputDebugString( L"Timestamp error?\n" );
		}
	}

	inputClient->ReleaseBuffer( numSamples );

	return data;
}

void SoundIO::audioInThread( Windows::Foundation::IAsyncAction^ operation ) {
	while( operation->Status != AsyncStatus::Canceled ) {
		if( WaitForSingleObjectEx( this->audioInReady, INFINITE, FALSE ) == WAIT_OBJECT_0 ) {
			if( operation->Status == AsyncStatus::Canceled )
				break;
			auto buffer = this->readAudio();

			// Sometimes the buffer can be NULL, because something failed inside of readAudio()
			// Otherwise, trigger the event!
			if( buffer != nullptr )
				audioInEvent( buffer );
		}
	}
	this->audioInThreadHandle->Close();
	this->audioInThreadHandle = nullptr;
}

void SoundIO::audioOutThread( Windows::Foundation::IAsyncAction^ operation ) {
	while( operation->Status != AsyncStatus::Canceled ) {
		if( WaitForSingleObjectEx( this->audioOutReady, INFINITE, FALSE ) == WAIT_OBJECT_0 ) {
			if( operation->Status == AsyncStatus::Canceled )
				break;
			auto buffer = audioOutEvent( outputBufferLen/2 );

			// Sometimes the buffer can be NULL, because something failed inside of audioOutEvent()
			// Otherwise, trigger the event!
			if( buffer != nullptr )
				this->writeAudio( buffer );
		}
	}
	this->audioOutThreadHandle->Close();
	this->audioOutThreadHandle = nullptr;
}
