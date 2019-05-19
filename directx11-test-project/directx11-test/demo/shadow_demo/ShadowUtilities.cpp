#include "stdafx.h"
#include "ShadowUtilities.h"
#include <service/locator.h>

using xtest::demo::ShadowUtilities;
using Microsoft::WRL::ComPtr;
using xtest::service::Locator;
/*
ShadowUtilities::ShadowUtilities(ID3D11Device* i_d3dDevice) 
{

}
*/
void ShadowUtilities::Init() {
	m_d3dDevice = Locator::GetD3DDevice();
	m_shadowsTextureResolution = D3D11_REQ_TEXTURECUBE_DIMENSION/4;
}

//ShadowUtilities::
void ShadowUtilities::CreateShadowDepthStencilBufferAndViews()
{
	// release any previous references
	//the following two lines could be useful in case of repeated calls to this method
	//probably not
	m_shadowDepthBuffer.Reset();
	m_shadowDepthBufferView.Reset();

	D3D11_TEXTURE2D_DESC shadowDepthBufferDesc;
	shadowDepthBufferDesc.Width = m_shadowsTextureResolution;
	shadowDepthBufferDesc.Height = m_shadowsTextureResolution;
	shadowDepthBufferDesc.MipLevels = 1;
	shadowDepthBufferDesc.ArraySize = 1;
	shadowDepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDepthBufferDesc.SampleDesc.Count = 1;		// no MSAA
	shadowDepthBufferDesc.SampleDesc.Quality = 0;	// no MSAA
	shadowDepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowDepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL| D3D11_BIND_SHADER_RESOURCE;
	shadowDepthBufferDesc.CPUAccessFlags = 0;
	shadowDepthBufferDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDepthBufferViewDesc;

	shadowDepthBufferViewDesc.Flags = 0;
	shadowDepthBufferViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDepthBufferViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDepthBufferViewDesc.Texture2D.MipSlice = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC shadowShaderResViewDesc;
	shadowShaderResViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowShaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowShaderResViewDesc.Texture2D.MipLevels = 1;
	shadowShaderResViewDesc.Texture2D.MostDetailedMip = 0;

	// create the depth buffer and its view
	//you need to know the device!!
	XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&shadowDepthBufferDesc, nullptr, &m_shadowDepthBuffer));
	XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_shadowDepthBuffer.Get(),&shadowDepthBufferViewDesc, &m_shadowDepthBufferView));
	XTEST_D3D_CHECK(m_d3dDevice->CreateShaderResourceView(m_shadowDepthBuffer.Get(), &shadowShaderResViewDesc, &m_shadowShaderResourceView));
}

void ShadowUtilities::SetViewPort() {
	m_shadowsViewPort.TopLeftX = 0.0f;
	m_shadowsViewPort.TopLeftY = 0.0f;
	m_shadowsViewPort.MinDepth = 0.0f;
	m_shadowsViewPort.MaxDepth = 1.0f;
	m_shadowsViewPort.Width= static_cast<float>(m_shadowsTextureResolution);
	m_shadowsViewPort.Height= static_cast<float>(m_shadowsTextureResolution);
}