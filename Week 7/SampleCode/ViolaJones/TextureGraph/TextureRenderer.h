#pragma once
#include "Direct3DBase.h"
#include <mutex>

using namespace Microsoft::WRL;

namespace TextureGraph
{	
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};


	struct VertexPosition
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT2 tex;
	};

	// This class no longer renders a simple spinning Cube.
	ref class TextureRenderer sealed : public Direct3DBase
	{
	public:
		TextureRenderer();
		virtual ~TextureRenderer();

		// Direct3DBase methods.
		virtual void CreateDeviceResources() override;
		virtual void CreateWindowSizeDependentResources() override;
		virtual void Render() override;

	internal:
		// Method for updating the displayed texture
		void setTexture( unsigned int width, unsigned int height, const int * data );
		void setTexturePtr( unsigned int width, unsigned int height, uintptr_t dataPtr );

	private:
		void uploadVertexBuffer();
		void uploadTextureBuffer();
		void makeConstantBuffers();

		bool loaded, constantBufferDirty;
		bool textureSizeDirty, textureDirty;
		int width, height;
		int * data;

		std::mutex texMutex;

		ComPtr<ID3D11InputLayout> m_inputLayout;
		ComPtr<ID3D11VertexShader> m_vertexShader;
		ComPtr<ID3D11PixelShader> m_pixelShader;
		ComPtr<ID3D11Buffer> m_vertexBuffer;
		ComPtr<ID3D11Buffer> m_constantBuffer;
		ComPtr<ID3D11Texture2D> m_textureBuffer;
		ComPtr<ID3D11ShaderResourceView> m_textureResource;
		ComPtr<ID3D11SamplerState> m_samplerState;
		ModelViewProjectionConstantBuffer m_constantBufferData;
	};
}