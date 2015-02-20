#pragma once

#include <wrl/client.h>
#include <implements.h>
#include <agile.h>
#include <Windows.Phone.Media.Capture.h>
#include <Windows.Phone.Media.Capture.Native.h>
#include <list>
#include <vector>
#include <mutex>

// This for RuntimeClass, RuntimeClassFlags and RuntimeClassType
using namespace Microsoft::WRL;

// This for Size
using namespace Windows::Foundation;

// This for Array
using namespace Platform;

// This for CameraSensorLocation
using namespace Windows::Phone::Media::Capture;

// This is a macro that expands to a type.  The type is complicated, and has to do with COM,
// which is why we won't talk about it much.  Essentially this sets up our completely native C++
// class to use a LOT of background code so that it can interoperate with Microsoft's APIs.
// This means that all we have to do is override the functions we are interested in, like
// OnFrameAvailable. This is a pure object-oriented approach to callbacks: create a class
// that conforms to a certain interface (ICameraCapturePreviewSink, for instance) and override
// a single method to do what you want.
#define COMParent(T) RuntimeClass<RuntimeClassFlags<RuntimeClassType::ClassicCom>, T>

// Don't get freaked out by IFACEMETHODIMP_(void), think of it as the same thing as:
// "public override void"

namespace libvideo
{
	// Forward declaration so that we can have camera objects inside of the sink objects, and yet have
	// sink objects inside of our camera object.  Time travel; you can't keep it in your head.
	ref class Camera;

	// delegate declaration for the events we're going to expose inside of Camera
	public delegate void FrameReadyEvent( unsigned int width, unsigned int height, uintptr_t dataPtr );
	public delegate void CompressedFrameReadyEvent( const Platform::Array<unsigned char>^ dataPtr, uint64_t frameTime, uint64_t frameDuration );
	//public delegate void AudioReadyEvent( const Array<float>^ data );
	public delegate void CameraStateChange( void );

	// This class is used with preview frames
	class CameraPreviewSink : public COMParent(ICameraCapturePreviewSink) {
		// This gets called whenever a preview frame is available
		IFACEMETHODIMP_(void) OnFrameAvailable( DXGI_FORMAT format, UINT width, UINT height, BYTE* pixels );

	public:
		// This public Camera pointer is for calling back to Camera when we have a frame available
		Camera^ camera;
	};

	// This class is used with full-size (sample) frames
	class CameraVideoSink : public COMParent(ICameraCaptureSampleSink) {
		// This gets called whenever a full-size (sample) frame is available
		IFACEMETHODIMP_(void) OnSampleAvailable( ULONGLONG hnsPresentationTime, ULONGLONG hnsSampleDuration, DWORD cbSample, BYTE* pSample);

	public:
		// This public Camera pointer is for calling back to Camera when we have a frame available
		Camera^ camera;
	};

    public ref class Camera sealed
    {
		// These friend declarations are necessary so that the Sinks can access the events inside of Camera
		friend class CameraPreviewSink;
		friend class CameraVideoSink;
		friend class CameraAudioSink;

    public:
		// This opens the camera for a certain capture and preview format, a certain camera sensor, and
		// capturing audio or not. See http://goo.gl/Qlo6L for "known good" dimensions for capture/preview.
        Camera( Size captureDimensions, CameraSensorLocation location );

		// Cleans stuff up
		virtual ~Camera();

		// These do exactly what they sound like
		void Start();
		void Stop();

		event FrameReadyEvent^ OnFrameReady;
		event CompressedFrameReadyEvent^ OnCompressedFrameReady;
		//event AudioReadyEvent^ OnAudioReady;
		event CameraStateChange^ Initialized;
		event CameraStateChange^ Started;
		event CameraStateChange^ Stopped;

	// These are helper functions for setup, split out for clarity
	private:
		// This checks to make sure the resolutions asked for aren't going to cause problems
		void CheckResolutions( Size capture, Size preview, CameraSensorLocation sensor );

		// This sets up the preview stream, creating the CameraPreviewSink object, etc...
		void SetupPreview();

		// This sets up the sample stream, creating the CameraVideoSink object, etc...
		void SetupVideo();

		// This sets up the audio stream, creating the CameraAudioSink object, etc...
		//void SetupAudio( void );

		void Cleanup();

	// These are internal data structures necessary for image capture
	private:
		Windows::Phone::Media::Capture::AudioVideoCaptureDevice^ AVDevice;
		ICameraCaptureDeviceNative* captureDevice;
		IAudioVideoCaptureDeviceNative* nativeAVDevice;
		CameraPreviewSink* previewSink;
		CameraVideoSink* videoSink;
		//CameraAudioSink* audioSink;

		// We need this so that Start() and Stop() don't kill themselves
		bool recording;

		// Our resolutions, as we use these quite often
		Size captureResolution, previewResolution;

		// This is a list of buffers we continually reuse, so that we don't have to allocate them every frame!
		std::vector<unsigned int *> * framePool;
		int poolIdx;

		std::list<unsigned int *> previewQueue;
		typedef struct compressedFrame {
			unsigned char * data;
			unsigned int dataLen;
			uint64_t frameTime, frameDuration;
		};
		std::list< compressedFrame > compressedQueue;
		std::mutex previewQueueMutex, compressedQueueMutex;

		// This is the handle to our frame processing thread
		IAsyncAction^ processingThreadHandle;
		void processingThread( IAsyncAction^ operation );
    };
}