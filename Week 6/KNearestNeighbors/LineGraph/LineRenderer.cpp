#include "pch.h"
#include "LineRenderer.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
 
LineRenderer::LineRenderer() {
	loaded = false;
	lineVerts = NULL;
	N = 0;
	yMin = -1.0f;
	yMax = 1.0f;
	color = XMFLOAT3( 1.0f, 1.0f, 1.0f );
	buffer_lock = CreateMutexEx( NULL, FALSE, NULL, MUTEX_ALL_ACCESS );
}

LineRenderer::~LineRenderer() {
	if( lineVerts )
		delete[] lineVerts;
	N = 0;
}

void LineRenderer::CreateDeviceResources() {
    Direct3DBase::CreateDeviceResources();

	auto loadVSTask = DX::ReadDataAsync("SimpleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync("SimplePixelShader.cso");

	auto createVSTask = loadVSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreateVertexShader(
				fileData->Data,
				fileData->Length,
				nullptr,
				&m_vertexShader
				)
			);

		const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = 
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_d3dDevice->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				fileData->Data,
				fileData->Length,
				&m_inputLayout
				)
			);
	});

	auto createPSTask = loadPSTask.then([this](Platform::Array<byte>^ fileData) {
		DX::ThrowIfFailed(
			m_d3dDevice->CreatePixelShader(
				fileData->Data,
				fileData->Length,
				nullptr,
				&m_pixelShader
				)
			);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_d3dDevice->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
				)
			);
	});
	
	auto createLineTask = (createPSTask && createVSTask).then([this] () {
		loaded = true;	
	});
	float data[480];
	memset( data, 0, sizeof(float)*480 );
	this->setArray( data, 480 );
}

void LineRenderer::lockBuffers() {
	WaitForSingleObjectEx( buffer_lock, INFINITE, false );
}

void LineRenderer::unlockBuffers() {
	ReleaseMutex( buffer_lock );
}
 
void LineRenderer::CreateWindowSizeDependentResources() {
    Direct3DBase::CreateWindowSizeDependentResources();
	makeConstantBuffers();
}

void LineRenderer::makeConstantBuffers() {
	XMStoreFloat4x4(&m_constantBufferData.projection, XMMatrixTranspose( XMMatrixOrthographicRH( 2.0f, yMax - yMin, 0.01f, 100.0f )));
	XMVECTOR eye = XMVectorSet(0.0f, (yMin+yMax)/2.0f, 1.0f, 0.0f);
	XMVECTOR at = XMVectorSet(0.0f, (yMin+yMax)/2.0f, 0.0f, 0.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(0)));
	constantBufferDirty = true;
}

void LineRenderer::setYLim( float yMin, float yMax ) {
	lockBuffers();
	this->yMin = yMin;
	this->yMax = yMax;
	this->makeConstantBuffers();
	unlockBuffers();
}

void LineRenderer::setArray(float * data, unsigned int N) {
	// Recreate lineVerts, if need be
	lockBuffers();
	if( N != this->N ) {
		if( this->lineVerts )
			delete[] this->lineVerts;

		this->lineVerts = new VertexPosition[N];
		for( unsigned int i=0; i<N; ++i ) {
			lineVerts[i].pos.x = i/(N/2.0f) - 1;
			lineVerts[i].pos.z = 0.0f;
			lineVerts[i].color = color;
		}
		this->vbSizeDirty = true;
	}

	// Copy data over
	for( unsigned int i=0; i<N; ++i ) {
		// Check for NaN and +- Inf
		if( _isnan(data[i]) || !_finite(data[i]) ) {
			if( data[i] > 0.0f )
				lineVerts[i].pos.y = yMax*1.1f;
			if( data[i] < 0.0f )
				lineVerts[i].pos.y = yMin*1.1f;
		} else {
			lineVerts[i].pos.y = data[i];
		}
	}

	this->N = N;
	this->vbDirty = true;
	unlockBuffers();
}

void LineRenderer::getArray( VertexPosition ** userVerts, unsigned int * userN ) {
	*userVerts = this->lineVerts;
	*userN = this->N;
}


void LineRenderer::updateVertexBuffer() {
	if( vbDirty ) {
		if( vbSizeDirty ) {
			D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
			vertexBufferData.pSysMem = lineVerts;
			vertexBufferData.SysMemPitch = 0;
			vertexBufferData.SysMemSlicePitch = 0;

			CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(VertexPosition)*N, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
			HRESULT hr = m_d3dDevice->CreateBuffer( &vertexBufferDesc, &vertexBufferData, &m_vertexBuffer );
			DX::ThrowIfFailed( hr );
			vbSizeDirty = false;
		}
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_d3dContext->Map( m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy( mappedResource.pData, lineVerts, sizeof(VertexPosition)*N );
		m_d3dContext->Unmap( m_vertexBuffer.Get(), 0 );
		vbDirty = false;
	}
}

void LineRenderer::appendToArray( float sample ) {
	if( this->lineVerts ) {
		for( unsigned int i=0; i<N-1; ++i ) {
			this->lineVerts[i].pos.y = this->lineVerts[i+1].pos.y;
		}
		this->lineVerts[N-1].pos.y = sample;
		this->vbDirty = true;
	}
}

void LineRenderer::setColor( float r, float g, float b ) {
	this->color = XMFLOAT3( r, g, b );
	if( this->lineVerts ) {
		for( unsigned int i=0; i<N; ++i ) {
			lineVerts[i].color = this->color;
		}
	}
	this->vbDirty = true;
}
 
void LineRenderer::Render()
{
	// If nothing is dirty, don't do anything!
	if( !vbDirty && !constantBufferDirty )
		return;

    const float black[] = { 0.0f, 0.0f, 0.0f, 0.000f };
    m_d3dContext->ClearRenderTargetView(
        m_renderTargetView.Get(),
        black
    );
 
    m_d3dContext->ClearDepthStencilView(
        m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH,
        1.0f,
        0
    );

	if (!loaded) {
		return;
	}

	lockBuffers();
	if( constantBufferDirty ) {
		// Throw the constant matrices up onto the GPU only if we need to
		m_d3dContext->UpdateSubresource( m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0 );
		constantBufferDirty = false;
	}
	
	this->updateVertexBuffer();
 
    m_d3dContext->OMSetRenderTargets(
        1,
        m_renderTargetView.GetAddressOf(),
        m_depthStencilView.Get()
    );

	UINT stride = sizeof(VertexPosition);
	UINT offset = 0;
	m_d3dContext->IASetVertexBuffers(
		0,
		1,
		m_vertexBuffer.GetAddressOf(),
		&stride,
		&offset
		);

	m_d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP );

	m_d3dContext->IASetInputLayout(m_inputLayout.Get());

	m_d3dContext->VSSetShader(
		m_vertexShader.Get(),
		nullptr,
		0
		);

	m_d3dContext->VSSetConstantBuffers(
		0,
		1,
		m_constantBuffer.GetAddressOf()
		);

	m_d3dContext->PSSetShader(
		m_pixelShader.Get(),
		nullptr,
		0
		);

	m_d3dContext->Draw( this->N, 0 );
	unlockBuffers();
}