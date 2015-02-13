#pragma once

#include "pch.h"
#include "BasicTimer.h"
#include "LineRenderer.h"
#include <DrawingSurfaceNative.h>

namespace LineGraph
{
	public delegate void RequestAdditionalFrameHandler();
	public delegate void RecreateSynchronizedTextureHandler();
	public delegate void LineGraphReady();

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class LineGraphInterop sealed : public Windows::Phone::Input::Interop::IDrawingSurfaceManipulationHandler
	{
	public:
		LineGraphInterop();

		Windows::Phone::Graphics::Interop::IDrawingSurfaceContentProvider^ CreateContentProvider();

		virtual void SetManipulationHost(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ manipulationHost);

		event RequestAdditionalFrameHandler^ RequestAdditionalFrame;
		event RecreateSynchronizedTextureHandler^ RecreateSynchronizedTexture;
		event LineGraphReady^ Initialized;

		property Windows::Foundation::Size WindowBounds;
		property Windows::Foundation::Size NativeResolution;
		property Windows::Foundation::Size RenderResolution
		{
			Windows::Foundation::Size get(){ return m_renderResolution; }
			void set(Windows::Foundation::Size renderResolution);
		}

		////////////////////////////////////////////////////////////////////////////////////////////////////////
		// EE 590:
		// These are the methods you will be using!  Everything else in this class is necessary, but boilerplate

		// Overwrites the entire array you're plotting, use with audio where the entire buffer changes every frame
		void setArray( const Platform::Array<float>^ buff );

		// Appends to the internal array (of size what was last set, default 480 samples, you can change via setArray)
		void appendToArray( float sample );

		// Returns the internal array as a Platform::Array<float>^
		Platform::Array<float>^ getArray();

		// Sets the color of the line
		void setColor( float r, float g, float b );

		// Set the y limits of the plot (defaults to -1, 1)
		void setYLimits( float yMin, float yMax );
		////////////////////////////////////////////////////////////////////////////////////////////////////////
	protected:
		// Event Handlers
		void OnPointerPressed(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerMoved(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);

	internal:
		HRESULT STDMETHODCALLTYPE Connect(_In_ IDrawingSurfaceRuntimeHostNative* host);
		void STDMETHODCALLTYPE Disconnect();
		HRESULT STDMETHODCALLTYPE PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty);
		HRESULT STDMETHODCALLTYPE GetTexture(_In_ const DrawingSurfaceSizeF* size, _Out_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Out_ DrawingSurfaceRectF* textureSubRectangle);
		ID3D11Texture2D* GetTexture();

	private:
		LineRenderer^ m_renderer;
		Windows::Foundation::Size m_renderResolution;
	};

}
