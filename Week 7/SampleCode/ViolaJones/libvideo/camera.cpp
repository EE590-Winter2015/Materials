// libvideo.cpp
#include "camera.h"

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Collections;
using namespace Windows::Phone::Media::Capture;
using namespace libvideo;
using namespace Platform;

// Need this for our threading types such as ThreadPool
using namespace Windows::System::Threading;

Camera::Camera( Size previewDimensions, CameraSensorLocation sensor )
{
	// First, check to make sure that captureDimensions is valid:
	Size captureDimensions(1280, 720);
	CheckResolutions( captureDimensions, previewDimensions, sensor );

	this->captureResolution = captureDimensions;
	this->previewResolution = previewDimensions;

	// If it is, start opening it, we'll quit out of the while loop at the end of this loop
	IAsyncOperation<AudioVideoCaptureDevice^ >^ openOp;

	// If we want audio, we've got a different path we must follow
	/*
	if( captureAudio )
		openOp = AudioVideoCaptureDevice::OpenAsync(sensor, captureDimensions );
	else
	*/
	openOp = AudioVideoCaptureDevice::OpenForVideoOnlyAsync(sensor, this->captureResolution );

	// Start up our frame-processing thread
	processingThreadHandle = ThreadPool::RunAsync( ref new WorkItemHandler(this, &Camera::processingThread) );

	// We subscribe to the "Completed" event.  Notice we're using an = sign here, not a +=.
	// This is because a lot of this API is oldschool COM, and not the newer style C++ code we're used
	// to dealing with.  Additionally, this is more "pure" C++, not C++/CX, so in some cases the event
	// keyword etc. aren't even available to be used, as the client code isn't C++/CX!
	openOp->Completed = ref new AsyncOperationCompletedHandler<AudioVideoCaptureDevice^>(
		[this,previewDimensions](IAsyncOperation<AudioVideoCaptureDevice^> ^operation, Windows::Foundation::AsyncStatus status ) {
			if( status == Windows::Foundation::AsyncStatus::Completed ) {
				// GetResults() gives us the actual opened AudioVideoCaptureDevice!
				AVDevice = operation->GetResults();

				// We use this AudioVideoCaptureDevice to get an ICameraCaptureDeviceNative, storing
				// that into captureDevice.  Note that AVDevice is the same object we can use from C#,
				// but captureDevice is something new, we can't use that from C++!
				reinterpret_cast<IUnknown *>(AVDevice)->QueryInterface(__uuidof(ICameraCaptureDeviceNative), (void **) &captureDevice);

				// We also get an IAudioVideoCaptureDeviceNative, because there are apparently a few
				// C++ only methods hidden away in this native counterpart to AudioVideoCaptureDevice
				// that we need.  So we get it in the same was as above.
				reinterpret_cast<IUnknown *>(AVDevice)->QueryInterface(__uuidof(IAudioVideoCaptureDeviceNative), (void**) &nativeAVDevice );

				// Now we start on the work, now that we've got all our interfaces sorted.  We first setup
				// the preview resolution, create our PreviewSink and connect everything up asynchronously:
				this->SetupPreview();

				// Next, we setup the capture video input:
				this->SetupVideo();

				// Setup audio input, if it's wanted
				//if( captureAudio )
				//	this->SetupAudio();

				// Send the "initialized" event
				this->Initialized();
			} else {
				throw ref new InvalidArgumentException("Could not open AudioVideoCaptureDevice with given captureDimensions!");
			}
		}
	);
}

Camera::~Camera() {
	// Only run Cleanup after properly stopping
	this->Stopped += ref new CameraStateChange(this, &Camera::Cleanup);
	this->Stop();
}

void Camera::Cleanup() {
	if( processingThreadHandle ) {
		processingThreadHandle->Cancel();
		processingThreadHandle->Close();

		// Don't forget to clean up the framePool
		for( unsigned int i=0; i<framePool->capacity(); ++i )
			delete (*framePool)[i];

		delete framePool;
	}
	previewSink->Release();
	videoSink->Release();
	nativeAVDevice->Release();
	captureDevice->Release();
}

void Camera::Start() {
	// This will actually complain (loudly) if we're already recording, so we've gotta make sure we aren't
	if( !this->recording ) {
		AVDevice->StartRecordingToSinkAsync()->Completed = ref new AsyncActionCompletedHandler(
			[this]( IAsyncAction^ operation, Windows::Foundation::AsyncStatus status ) {
				if( status == Windows::Foundation::AsyncStatus::Completed ) {
					this->recording = true;
					this->Started();
				}
			}
		);
	}
}

void Camera::Stop() {
	// This will actually complain (loudly) if we're not already recording, so we've gotta make sure we are
	if( this->recording ) {
		AVDevice->StopRecordingAsync()->Completed = ref new AsyncActionCompletedHandler(
			[this]( IAsyncAction^ operation, Windows::Foundation::AsyncStatus status ) {
				if( status == Windows::Foundation::AsyncStatus::Completed ) {
					this->recording = false;
					this->Stopped();
				}
			}
		);
	}
}


void Camera::CheckResolutions( Size capture, Size preview, CameraSensorLocation sensor ) {
	// This is a collection of possible sizes
	IVectorView<Size> ^availableSizes = AudioVideoCaptureDevice::GetAvailableCaptureResolutions(sensor);

	// An iterator to move through those sizes
    IIterator<Size> ^availableSizesIterator = availableSizes->First();

	// A flag to let us know if we've foudn an acceptable size
	bool sizeOK = false;

	// Loop through all available capture sizes until we run out or we find one that matches
	while(availableSizesIterator->HasCurrent && availableSizesIterator->Current != capture )
		availableSizesIterator->MoveNext();

	// If we didn't find one that matches, error out!
	if( !availableSizesIterator->HasCurrent ) {
		throw ref new InvalidArgumentException("captureDimensions invalid: Use AudioVideoCaptureDevice.GetAvailableCaptureResolutions() to find valid resolutions!");
	}

	// Otherwise, continue on to check the preview resolutions
	availableSizes = AudioVideoCaptureDevice::GetAvailablePreviewResolutions(sensor);
	availableSizesIterator = availableSizes->First();
	
	while(availableSizesIterator->HasCurrent && availableSizesIterator->Current != preview )
		availableSizesIterator->MoveNext();

	if( !availableSizesIterator->HasCurrent ) {
		throw ref new InvalidArgumentException("previewDimensions invalid: Use AudioVideoCaptureDevice.GetAvailablePreviewResolutions() to find valid resolutions!");
	}
}

void Camera::SetupPreview() {
	// First, we tell our captureDevice what preview dimensions we want
	auto previewResOp = AVDevice->SetPreviewResolutionAsync(previewResolution);

	// Once it's done setting the preview format, we hook it up to the PreviewSink:
	previewResOp->Completed = ref new AsyncActionCompletedHandler(
		[this](IAsyncAction^ action, Windows::Foundation::AsyncStatus status) {
			if( status == Windows::Foundation::AsyncStatus::Completed ) {
				// Create the sink object, note that this is just like a new(), only oldschool style
				MakeAndInitialize<CameraPreviewSink>(&this->previewSink);

				// Tell the previewSink where we are, so that they can call back to us
				previewSink->camera = this;

				// Hook it up to the Capture Device
				captureDevice->SetPreviewSink(this->previewSink);

				// Finally, set the preview format on this guy too; this sets pixel type, not dimensions. We
				// ask for 32-bit, ARGB format.  There are many formats available, this one works with C# best
				captureDevice->SetPreviewFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM);
			} else {
				throw ref new InvalidArgumentException("Could not set preview format!");
			}
		}
	);
}

void Camera::SetupVideo( ) {
	// We ask for ARGB, as we want to process the data.  If we were piping it across the network,
	// we might ask for H264 instead, as that is a compressed format, not uncompressed raw data
	AVDevice->VideoEncodingFormat = CameraCaptureVideoFormat::H264;

	// Create the sink object, note that this is just like a new(), only oldschool style
	MakeAndInitialize<CameraVideoSink>(&this->videoSink);

	// Tell the videoSink where we are, so they can call back to us
	videoSink->camera = this;

	// Hook it up to the capture device
	nativeAVDevice->SetVideoSampleSink(this->videoSink);

	// Note that we don't have to use a Completed event handler here or anything.  That's
	// because we're already inside of an event handler related to the sample stream: the
	// event handler in the constructor.  The preview stream is a separate stream, so it
	// needs its own "permission" to setup and its own event handler.
}

void Camera::processingThread( IAsyncAction^ operation ) {
	// Reserve memory
	framePool = new std::vector<unsigned int *>();
	framePool->reserve(7);
	
	// Any H264-encoded memory streams are going to be so very much smaller than any preview sizes, this will be fine
	int maxSize = (int)(previewResolution.Height*previewResolution.Width);
	for( unsigned int i=0; i<framePool->capacity(); ++i )
		framePool->push_back( new unsigned int[maxSize] );

	// Here, we spin in a loop for all eternity, waiting for frames from the input, and giving them to the output events
	while( true ) {
		if( !previewQueue.size() && !compressedQueue.size() ) {
			WaitForSingleObjectEx( GetCurrentThread(), 1, true );
		} else {
			if( previewQueue.size() ) {
				unsigned int * p = previewQueue.front();

				this->OnFrameReady( (unsigned int)this->previewResolution.Width, (unsigned int)this->previewResolution.Height, (uintptr_t)p );
				previewQueueMutex.lock();
				previewQueue.pop_front();
				previewQueueMutex.unlock();
			}

			if( compressedQueue.size() ) {
				compressedFrame p = compressedQueue.front();

				this->OnCompressedFrameReady( Platform::ArrayReference<unsigned char>(p.data, p.dataLen), p.frameTime, p.frameDuration );
				compressedQueueMutex.lock();
				compressedQueue.pop_front();
				compressedQueueMutex.unlock();
			}
		}	

	}
}

// Here we setup the handlers on the sinks.  They are very, very basic:
void CameraPreviewSink::OnFrameAvailable( DXGI_FORMAT format, UINT width, UINT height, BYTE* pixels ) {
	// First, check to see if the length is small.  If not, we won't add on
	if( camera->previewQueue.size() + camera->compressedQueue.size() < 2 ) {
		// Get a buffer from the pool
		unsigned int * newBuff = (*camera->framePool)[camera->poolIdx];
		camera->poolIdx = (camera->poolIdx + 1)%camera->framePool->capacity();
		memcpy(newBuff, pixels, sizeof(int)*width*height);

		// Get a lock on the mutex, and push it onto the frameQueue
		camera->previewQueueMutex.lock();
		camera->previewQueue.push_back( newBuff );
		camera->previewQueueMutex.unlock();
	}
}

double getCurrentTime() {
	uint64_t performanceCounterFreq;
	QueryPerformanceFrequency((LARGE_INTEGER *)&performanceCounterFreq);
	uint64_t time;
	QueryPerformanceCounter((LARGE_INTEGER *)&time);
	return ((double)time)/performanceCounterFreq;
}

void CameraVideoSink::OnSampleAvailable( ULONGLONG hnsPresentationTime, ULONGLONG hnsSampleDuration, DWORD cbSample, BYTE* pSample) {
	if( camera->compressedQueue.size() + camera->previewQueue.size() < 5 ) {
		// Get a buffer from the pool
		unsigned int * newBuff = (*camera->framePool)[camera->poolIdx];
		camera->poolIdx = (camera->poolIdx + 1)%camera->framePool->capacity();
		memcpy(newBuff, pSample, cbSample);
		
		// Get a lock on the mutex, and push it onto the frameQueue
		camera->compressedQueueMutex.lock();
		Camera::compressedFrame p;
		p.data = (unsigned char*)newBuff;
		p.dataLen = cbSample;
		p.frameTime = hnsPresentationTime;
		p.frameDuration = hnsSampleDuration;
		camera->compressedQueue.push_back( p );
		camera->compressedQueueMutex.unlock();
	} else
		OutputDebugStringA("Dropped a frame!");
}