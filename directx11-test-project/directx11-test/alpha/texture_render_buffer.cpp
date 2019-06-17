#include "stdafx.h"
#include "texture_render_buffer.h"
#include <service/locator.h>

alpha::TextureRenderBuffer::TextureRenderBuffer()
	: m_shaderView(nullptr)
	, m_renderTargetView(nullptr)
{
}

void alpha::TextureRenderBuffer::Init(uint32 width, uint32 height)
{
	if (m_shaderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = xtest::service::Locator::GetD3DDevice();


	// create texture to render on
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));



	// create the view used to draw on
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;


	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(texture.Get(), &renderTargetViewDesc, &m_renderTargetView));



	//create the view used by the shader
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), &shaderResourceViewDesc, &m_shaderView));
	
}


void alpha::TextureRenderBuffer::Resize(uint32 width, uint32 height) {

	m_renderTargetView.Reset();
	m_shaderView.Reset();


	ID3D11Device* d3dDevice = xtest::service::Locator::GetD3DDevice();


	// create texture to render on
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));



	// create the view used to draw on
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	ZeroMemory(&renderTargetViewDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;


	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(texture.Get(), &renderTargetViewDesc, &m_renderTargetView));

	//create the view used by the shader
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), &shaderResourceViewDesc, &m_shaderView));
}

ID3D11ShaderResourceView * alpha::TextureRenderBuffer::AsShaderView()
{
	XTEST_ASSERT(m_shaderView, L"shadow map uninitialized");
	return m_shaderView.Get();
}

ID3D11RenderTargetView * alpha::TextureRenderBuffer::AsRenderTargetView()
{
	XTEST_ASSERT(m_shaderView, L"shadow map uninitialized");
	return m_renderTargetView.Get();
}

