#pragma once
#include "Direct3DBase.h"

struct ModelViewProjectionConstantBuffer
{
	DirectX::XMFLOAT4X4 model;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
};


struct VertexPosition
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

// This class no longer renders a simple spinning Cube.
ref class LineRenderer sealed : public Direct3DBase
{
public:
    LineRenderer();
	virtual ~LineRenderer();
 
    // Direct3DBase methods.
    virtual void CreateDeviceResources() override;
    virtual void CreateWindowSizeDependentResources() override;
    virtual void Render() override;
 
internal:
    // Method for updating time-dependent objects.
    void setArray( float * data, unsigned int N );
	void getArray( VertexPosition ** userVerts, unsigned int * userN );
	void appendToArray( float sample );
	void setColor( float r, float g, float b );
	void setYLim( float yMin, float yMax );

private:
	void updateVertexBuffer();
	void makeConstantBuffers();

	VertexPosition * lineVerts;
	float * data;
	unsigned int N;
	DirectX::XMFLOAT3 color;
	bool loaded, vbDirty, vbSizeDirty, constantBufferDirty;
	float yMin, yMax;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	ModelViewProjectionConstantBuffer m_constantBufferData;

	// Whoops, we need a lock on the buffers when we're drawing!
	HANDLE buffer_lock;
	void lockBuffers();
	void unlockBuffers();
};