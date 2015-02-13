#include "pch.h"
#include "LineGraphInterop.h"
#include "Direct3DContentProvider.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Microsoft::WRL;
using namespace Windows::Phone::Graphics::Interop;
using namespace Windows::Phone::Input::Interop;

namespace LineGraph
{

LineGraphInterop::LineGraphInterop()
{
}

IDrawingSurfaceContentProvider^ LineGraphInterop::CreateContentProvider()
{
	ComPtr<Direct3DContentProvider> provider = Make<Direct3DContentProvider>(this);
	return reinterpret_cast<IDrawingSurfaceContentProvider^>(provider.Detach());
}

// IDrawingSurfaceManipulationHandler
void LineGraphInterop::SetManipulationHost(DrawingSurfaceManipulationHost^ manipulationHost)
{
	manipulationHost->PointerPressed +=
		ref new TypedEventHandler<DrawingSurfaceManipulationHost^, PointerEventArgs^>(this, &LineGraphInterop::OnPointerPressed);

	manipulationHost->PointerMoved +=
		ref new TypedEventHandler<DrawingSurfaceManipulationHost^, PointerEventArgs^>(this, &LineGraphInterop::OnPointerMoved);

	manipulationHost->PointerReleased +=
		ref new TypedEventHandler<DrawingSurfaceManipulationHost^, PointerEventArgs^>(this, &LineGraphInterop::OnPointerReleased);
}

void LineGraphInterop::RenderResolution::set(Windows::Foundation::Size renderResolution)
{
	if (renderResolution.Width  != m_renderResolution.Width ||
		renderResolution.Height != m_renderResolution.Height)
	{
		m_renderResolution = renderResolution;

		if (m_renderer)
		{
			m_renderer->UpdateForRenderResolutionChange(m_renderResolution.Width, m_renderResolution.Height);
			RecreateSynchronizedTexture();
		}
	}
}

// Event Handlers
void LineGraphInterop::OnPointerPressed(DrawingSurfaceManipulationHost^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void LineGraphInterop::OnPointerMoved(DrawingSurfaceManipulationHost^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void LineGraphInterop::OnPointerReleased(DrawingSurfaceManipulationHost^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

// Interface With Direct3DContentProvider
HRESULT LineGraphInterop::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host)
{
	m_renderer = ref new LineRenderer();
	m_renderer->Initialize();
	m_renderer->UpdateForWindowSizeChange(WindowBounds.Width, WindowBounds.Height);
	m_renderer->UpdateForRenderResolutionChange(m_renderResolution.Width, m_renderResolution.Height);

	// Fire off Initialized event
	Initialized();

	return S_OK;
}

void LineGraphInterop::Disconnect()
{
	m_renderer = nullptr;
}

HRESULT LineGraphInterop::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty)
{
	*contentDirty = true;

	return S_OK;
}

void LineGraphInterop::setArray( const Platform::Array<float>^ data ) {
	if( m_renderer ) {
		m_renderer->setArray( data->Data, data->Length );
	}
	RequestAdditionalFrame();
}

Platform::Array<float>^ LineGraphInterop::getArray() {
	if( m_renderer ) {
		VertexPosition * verts;
		unsigned int N;
		m_renderer->getArray( &verts, &N);

		Platform::Array<float>^ data = ref new Platform::Array<float>(N);
		for( unsigned int i=0; i<N; ++i ) {
			data->Data[i] = verts[i].pos.y;
		}
		return data;
	}
	return nullptr;
}

void LineGraphInterop::appendToArray( float sample ) {
	if( m_renderer ) {
		m_renderer->appendToArray( sample );
	}
	RequestAdditionalFrame();
}

void LineGraphInterop::setColor( float r, float g, float b ) {
	if( m_renderer ) {
		m_renderer->setColor( r, g, b );
	}
	RequestAdditionalFrame();
}

void LineGraphInterop::setYLimits( float yMin, float yMax ) {
	if( m_renderer ) {
		m_renderer->setYLim( yMin, yMax );
	}
	RequestAdditionalFrame();
}

HRESULT LineGraphInterop::GetTexture(_In_ const DrawingSurfaceSizeF* size, _Out_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Out_ DrawingSurfaceRectF* textureSubRectangle)
{
	m_renderer->Render();

	return S_OK;
}

ID3D11Texture2D* LineGraphInterop::GetTexture()
{
	return m_renderer->GetTexture();
}

}
