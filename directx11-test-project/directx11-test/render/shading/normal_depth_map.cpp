#include "stdafx.h"
#include "shadow_map.h"
#include <service/locator.h>
#include "normal_depth_map.h"


using namespace xtest::render::shading;
using namespace DirectX;

NormalDepthMap::NormalDepthMap(uint32 width, uint32 height)
	: m_width(width)
	, m_height(height)
	, m_shaderView(nullptr)
	, m_renderTargetView(nullptr)
{
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}


void NormalDepthMap::Init()
{
	// already initialized
	if (m_shaderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
	ID3D11DeviceContext* d3dContext = service::Locator::GetD3DContext();
	// create the shadow map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // modify
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));

	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(texture.Get(), 0, &m_renderTargetView));
	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), 0, &m_shaderView));

	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}


ID3D11ShaderResourceView* NormalDepthMap::AsShaderView()
{
	XTEST_ASSERT(m_shaderView, L"shadow map uninitialized");
	return m_shaderView.Get();
}



ID3D11RenderTargetView* NormalDepthMap::AsRenderTargetView()
{
	XTEST_ASSERT(m_renderTargetView, L"shadow map uninitialized");
	return m_renderTargetView.Get();
}


D3D11_VIEWPORT NormalDepthMap::Viewport() const
{
	return m_viewport;
}


uint32 NormalDepthMap::Width() const
{
	return m_width;
}

uint32 NormalDepthMap::Height() const
{
	return m_height;
}

void xtest::render::shading::NormalDepthMap::ResetTargetView(uint32 n_width, uint32 n_height)
{
	m_renderTargetView.Reset();
	m_shaderView.Reset();
	// already initialized
	if (m_shaderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
	ID3D11DeviceContext* d3dContext = service::Locator::GetD3DContext();
	// create the shadow map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = n_width;
	textureDesc.Height = n_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // modify
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));

	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(texture.Get(), 0, &m_renderTargetView));
	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), 0, &m_shaderView));

	float clearColor[] = { 0.0f, 0.0f, -1.0f, 1e5f };
	d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
}
