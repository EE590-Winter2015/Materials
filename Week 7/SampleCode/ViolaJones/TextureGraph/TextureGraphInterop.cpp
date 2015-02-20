#include "pch.h"
#include "TextureGraphInterop.h"
#include "Direct3DContentProvider.h"

using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Microsoft::WRL;
using namespace Windows::Phone::Graphics::Interop;
using namespace Windows::Phone::Input::Interop;

namespace TextureGraph
{

TextureGraphInterop::TextureGraphInterop() :
	m_timer(ref new BasicTimer())
{
}

IDrawingSurfaceContentProvider^ TextureGraphInterop::CreateContentProvider()
{
	ComPtr<Direct3DContentProvider> provider = Make<Direct3DContentProvider>(this);
	return reinterpret_cast<IDrawingSurfaceContentProvider^>(provider.Detach());
}

// IDrawingSurfaceManipulationHandler
void TextureGraphInterop::SetManipulationHost(DrawingSurfaceManipulationHost^ manipulationHost)
{
	manipulationHost->PointerPressed +=
		ref new TypedEventHandler<DrawingSurfaceManipulationHost^, PointerEventArgs^>(this, &TextureGraphInterop::OnPointerPressed);

	manipulationHost->PointerMoved +=
		ref new TypedEventHandler<DrawingSurfaceManipulationHost^, PointerEventArgs^>(this, &TextureGraphInterop::OnPointerMoved);

	manipulationHost->PointerReleased +=
		ref new TypedEventHandler<DrawingSurfaceManipulationHost^, PointerEventArgs^>(this, &TextureGraphInterop::OnPointerReleased);
}

void TextureGraphInterop::RenderResolution::set(Windows::Foundation::Size renderResolution)
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
void TextureGraphInterop::OnPointerPressed(DrawingSurfaceManipulationHost^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void TextureGraphInterop::OnPointerMoved(DrawingSurfaceManipulationHost^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void TextureGraphInterop::OnPointerReleased(DrawingSurfaceManipulationHost^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

// Interface With Direct3DContentProvider
HRESULT TextureGraphInterop::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host)
{
	m_renderer = ref new TextureRenderer();
	m_renderer->Initialize();
	m_renderer->UpdateForWindowSizeChange(WindowBounds.Width, WindowBounds.Height);
	m_renderer->UpdateForRenderResolutionChange(m_renderResolution.Width, m_renderResolution.Height);

	// Restart timer after renderer has finished initializing.
	m_timer->Reset();

	return S_OK;
}

void TextureGraphInterop::Disconnect()
{
	m_renderer = nullptr;
}

HRESULT TextureGraphInterop::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Out_ BOOL* contentDirty)
{
	*contentDirty = true;

	return S_OK;
}


void TextureGraphInterop::setTexture( unsigned int width, unsigned int height, const Platform::Array<int>^ data ) {
	if( m_renderer )
		m_renderer->setTexture( width, height, data->Data );
}

void TextureGraphInterop::setTexturePtr( unsigned int width, unsigned int height, unsigned int dataPtr ) {
	if( m_renderer )
		m_renderer->setTexturePtr( width, height, dataPtr );
}

HRESULT TextureGraphInterop::GetTexture(_In_ const DrawingSurfaceSizeF* size, _Out_ IDrawingSurfaceSynchronizedTextureNative** synchronizedTexture, _Out_ DrawingSurfaceRectF* textureSubRectangle)
{
	m_timer->Update();
	m_renderer->Render();

	RequestAdditionalFrame();

	return S_OK;
}

ID3D11Texture2D* TextureGraphInterop::GetTexture()
{
	return m_renderer->GetTexture();
}

}
