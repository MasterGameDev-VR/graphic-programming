#include "stdafx.h"
#include "color_map.h"
#include <service/locator.h>


using namespace xtest::render::shading;
using namespace DirectX;

ColorMap::ColorMap(): m_colorRenderView(nullptr)

{


	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}


void ColorMap::SetWidthHeight(uint32 width, uint32 height) {
	m_width = width;
	m_height = height;

	m_viewport.Width = static_cast<float>(m_width);
	m_viewport.Height = static_cast<float>(m_height);

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
	colorTextureDesc.Width = (UINT)m_width;
	colorTextureDesc.Height = (UINT)m_height;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&colorTextureDesc, nullptr, &colorTexture));
	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(colorTexture.Get(), &colorRenderViewDesc, &m_colorRenderView));
	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(colorTexture.Get(), &shaderViewDesc, &m_colorShaderView));

}


void ColorMap::Init(uint32 width, uint32 height) {
	// already initialized
	if (m_colorRenderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
	// create the color buffer texture
	
	colorTextureDesc.Width = (UINT)width;
	colorTextureDesc.Height = (UINT)height;
	colorTextureDesc.MipLevels = 1;
	colorTextureDesc.ArraySize = 1;
	colorTextureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	colorTextureDesc.SampleDesc.Count = 1;
	colorTextureDesc.SampleDesc.Quality = 0;
	colorTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	colorTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; //should it be bind both as render target and shader resource? I say yes
	colorTextureDesc.CPUAccessFlags = 0;
	colorTextureDesc.MiscFlags = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&colorTextureDesc, nullptr, &colorTexture));

	//create the view used by the shader
	colorRenderViewDesc.Format = colorTextureDesc.Format;
	colorRenderViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	colorRenderViewDesc.Texture2D.MipSlice = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(colorTexture.Get(), &colorRenderViewDesc, &m_colorRenderView));
	//create the view used by the shader
	shaderViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 24bit red channel (depth), 8 bit unused (stencil)
	shaderViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderViewDesc.Texture2D.MipLevels = 1;
	shaderViewDesc.Texture2D.MostDetailedMip = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(colorTexture.Get(), &shaderViewDesc, &m_colorShaderView));

}

ID3D11RenderTargetView* ColorMap::AsColorRenderTargetView()
{
	XTEST_ASSERT(m_colorRenderView, L"color map uninitialized");
	return m_colorRenderView.Get();
}
ID3D11ShaderResourceView* ColorMap::AsColorShaderView() {
	XTEST_ASSERT(m_colorShaderView, L"shadow map uninitialized");
	return m_colorShaderView.Get();
}

D3D11_VIEWPORT ColorMap::Viewport() const
{
	return m_viewport;
}