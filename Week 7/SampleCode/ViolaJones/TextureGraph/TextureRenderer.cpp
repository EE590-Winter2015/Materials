#include "pch.h"
#include "TextureRenderer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <mutex>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace TextureGraph;
 
TextureRenderer::TextureRenderer() {
	loaded = false;
	textureSizeDirty = false;
	textureDirty = false;
	width = height = 0;
}

TextureRenderer::~TextureRenderer() {
	if( this->data )
		delete this->data;
}

void TextureRenderer::CreateDeviceResources() {
    Direct3DBase::CreateDeviceResources();

	auto loadVSTask = DX::ReadDataAsync("SimpleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync("SimplePixelShader.cso");

	auto createVSTask = loadVSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed( m_d3dDevice->CreateVertexShader( fileData->Data, fileData->Length, nullptr, &m_vertexShader ) );

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed( m_d3dDevice->CreateInputLayout( vertexDesc, ARRAYSIZE(vertexDesc), fileData->Data, fileData->Length, &m_inputLayout ) );
	});

	auto createPSTask = loadPSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed( m_d3dDevice->CreatePixelShader( fileData->Data, fileData->Length, nullptr, &m_pixelShader ) );
		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed( m_d3dDevice->CreateBuffer( &constantBufferDesc, nullptr, &m_constantBuffer ) );
	});
	
	auto createTextureTask = (createPSTask && createVSTask).then([this] () {
		loaded = true;

		// Create plane and upload it to the GPU
		uploadVertexBuffer(); 

		// Set a default texture.  LOL.
		int black = 0;
		setTexture( 1, 1, &black );

		// Then upload it!
		uploadTextureBuffer();
	});
}
 
void TextureRenderer::CreateWindowSizeDependentResources() {
    Direct3DBase::CreateWindowSizeDependentResources();
	makeConstantBuffers();
}

void TextureRenderer::makeConstantBuffers() {
	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose( XMMatrixOrthographicRH( 2.0f, 2.0f, 0.01f, 100.0f )));
	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(0)));
	constantBufferDirty = true;
}

void TextureRenderer::setTexturePtr( unsigned int width, unsigned int height, uintptr_t dataPtr ) {
	setTexture( width, height, (const int *)dataPtr );
}

void TextureRenderer::setTexture( unsigned int width, unsigned int height, const int * data ) {
	texMutex.lock();
	if( this->width != width || this->height != height ) {
		if( this->data )
			delete this->data;
		this->width = width;
		this->height = height;
		this->data = new int[width*height];
		this->textureSizeDirty = true;
	}

	memcpy( this->data, data, sizeof(int)*width*height );
	this->textureDirty = true;
	texMutex.unlock();
}

void TextureRenderer::uploadTextureBuffer() {
	if( !loaded  ) {
		return;
	}
	texMutex.lock();
	if( textureSizeDirty ) {
		D3D11_TEXTURE2D_DESC textDesc;
		textDesc.Width = width;
		textDesc.Height = height;
		textDesc.MipLevels = textDesc.ArraySize = 1;
		textDesc.SampleDesc.Count = 1;
		textDesc.SampleDesc.Quality = 0;
		textDesc.Usage = D3D11_USAGE_DYNAMIC;
		textDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textDesc.MiscFlags = 0;
		textDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

		D3D11_SUBRESOURCE_DATA textureBufferData = {0};
		textureBufferData.pSysMem = data;
		textureBufferData.SysMemPitch = textDesc.Width*sizeof(int);
		textureBufferData.SysMemSlicePitch = 0;

		DX::ThrowIfFailed( m_d3dDevice->CreateTexture2D( &textDesc, &textureBufferData, &m_textureBuffer ) );

		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		DX::ThrowIfFailed( m_d3dDevice->CreateSamplerState( &samplerDesc, &m_samplerState ) );

		D3D11_SHADER_RESOURCE_VIEW_DESC	srvDesc;
		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		DX::ThrowIfFailed( m_d3dDevice->CreateShaderResourceView( m_textureBuffer.Get(), &srvDesc, &m_textureResource ) );
		this->textureSizeDirty = false;
	} else if( this->textureDirty ) {
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_d3dContext->Map( m_textureBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy( mappedResource.pData, data, sizeof(int)*width*height );
		m_d3dContext->Unmap( m_textureBuffer.Get(), 0 );
	}
	this->textureDirty = false;
	texMutex.unlock();
}

void TextureRenderer::uploadVertexBuffer() {
	if( !loaded ) {
		return;
	}
	VertexPosition planeVerts[4];

	planeVerts[0].pos = XMFLOAT3(-1, -1, 0); planeVerts[0].tex = XMFLOAT2( 1, 1 );
	planeVerts[1].pos = XMFLOAT3(-1,  1, 0); planeVerts[1].tex = XMFLOAT2( 0, 1 );
	planeVerts[2].pos = XMFLOAT3( 1, -1, 0); planeVerts[2].tex = XMFLOAT2( 1, 0 );
	planeVerts[3].pos = XMFLOAT3( 1,  1, 0); planeVerts[3].tex = XMFLOAT2( 0, 0 );

	D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
	vertexBufferData.pSysMem = planeVerts;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;

	CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosition)*4, D3D11_BIND_VERTEX_BUFFER );
	HRESULT hr = m_d3dDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_vertexBuffer );
	DX::ThrowIfFailed( hr );
	/*
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_d3dContext->Map( m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy( mappedResource.pData, planeVerts, sizeof(VertexPosition)*4 );
	m_d3dContext->Unmap( m_vertexBuffer.Get(), 0 );
	*/
}
 
void TextureRenderer::Render()
{
    const float black[] = { 0.0f, 0.0f, 0.3f, 0.0f };
    m_d3dContext->ClearRenderTargetView( m_renderTargetView.Get(), black );
 
    m_d3dContext->ClearDepthStencilView( m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

	if (!loaded) {
		return;
	}

	if( constantBufferDirty ) {
		// Throw the constant matrices up onto the GPU only if we need to
		m_d3dContext->UpdateSubresource( m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0 );
		constantBufferDirty = false;
	}
	
	// Change this to updateTextureBuffer, eventually
	this->uploadTextureBuffer();
 
    m_d3dContext->OMSetRenderTargets( 1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get() );

	UINT stride = sizeof(VertexPosition);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers( 0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset );
	m_d3dContext->IASetPrimitiveTopology( D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	m_d3dContext->IASetInputLayout(m_inputLayout.Get());
	m_d3dContext->VSSetShader( m_vertexShader.Get(), nullptr, 0 );
	m_d3dContext->VSSetConstantBuffers( 0, 1, m_constantBuffer.GetAddressOf() );
	m_d3dContext->PSSetShader( m_pixelShader.Get(), nullptr, 0 );
	m_d3dContext->PSSetShaderResources( 0, 1, m_textureResource.GetAddressOf() );
	m_d3dContext->PSSetSamplers(0, 1, m_samplerState.GetAddressOf() );

	m_d3dContext->Draw( 4, 0 );
}