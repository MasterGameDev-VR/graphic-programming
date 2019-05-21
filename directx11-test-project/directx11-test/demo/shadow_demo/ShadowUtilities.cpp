#include "stdafx.h"
#include "ShadowUtilities.h"
#include <service/locator.h>



using xtest::demo::ShadowUtilities;
using Microsoft::WRL::ComPtr;
using xtest::service::Locator;

ShadowUtilities::ShadowUtilities() : 
	/*
	m_shadowDepthBuffer(nullptr),
	m_shadowDepthBufferView(nullptr),
	m_shadowShaderResourceView(nullptr),
	*/
	m_d3dDevice(Locator::GetD3DDevice())
{
	m_shadowDepthBuffer = Microsoft::WRL::ComPtr < ID3D11Texture2D>();

	m_shadowDepthBufferView = Microsoft::WRL::ComPtr < ID3D11DepthStencilView>();
	m_shadowShaderResourceView = Microsoft::WRL::ComPtr < ID3D11ShaderResourceView>();
}

void ShadowUtilities::Init() {
	//m_d3dDevice = Locator::GetD3DDevice();
	m_shadowsTextureResolution = D3D11_REQ_TEXTURECUBE_DIMENSION/16;
	m_shadowDepthBuffer.Reset();
	m_shadowDepthBufferView.Reset();
	m_shadowShaderResourceView.Reset();
	CreateShadowDepthStencilBufferAndViews();

}

//ShadowUtilities::
void ShadowUtilities::CreateShadowDepthStencilBufferAndViews()
{
	// release any previous references
	//the following two lines could be useful in case of repeated calls to this method
	//probably not
	
	shadowDepthBufferDesc = { 0 };
	shadowDepthBufferDesc.Width = m_shadowsTextureResolution;
	shadowDepthBufferDesc.Height = m_shadowsTextureResolution;
	shadowDepthBufferDesc.MipLevels = 1;
	shadowDepthBufferDesc.ArraySize = 1;
	//there was another error caused by the format
	shadowDepthBufferDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	shadowDepthBufferDesc.SampleDesc.Count = 1;		// no MSAA
	shadowDepthBufferDesc.SampleDesc.Quality = 0;	// no MSAA
	shadowDepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowDepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL| D3D11_BIND_SHADER_RESOURCE;
	//there was an error caused by the  CPU Access Flag
	//shadowDepthBufferDesc.CPUAccessFlags = 0;
	shadowDepthBufferDesc.MiscFlags = 0;


	shadowDepthStencilDesc.DepthEnable=TRUE;
	shadowDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	shadowDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	shadowDepthStencilDesc.StencilEnable = FALSE;


	shadowDepthBufferViewDesc.Flags = 0;
	shadowDepthBufferViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDepthBufferViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDepthBufferViewDesc.Texture2D.MipSlice = 0;

	
	shadowShaderResViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	shadowShaderResViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowShaderResViewDesc.Texture2D.MipLevels = 1;
	shadowShaderResViewDesc.Texture2D.MostDetailedMip = 0;

	//D3D11_SUBRESOURCE_DATA shadowDepthTextureSubresource;
	//shadowDepthTextureSubresource.pSysMem = m_shadowDepthBuffer.Get();

	//it should be good to NOT initialize this subresource!!
	// and to not give her any pitch data
	
	// create the depth buffer and its view
	//you need to know the device!!
	
}

void ShadowUtilities::SetViewPort() {
	m_shadowsViewPort.TopLeftX = 0.0f;
	m_shadowsViewPort.TopLeftY = 0.0f;
	m_shadowsViewPort.MinDepth = 0.0f;
	m_shadowsViewPort.MaxDepth = 1.0f;
	m_shadowsViewPort.Width= static_cast<float>(m_shadowsTextureResolution);
	m_shadowsViewPort.Height= static_cast<float>(m_shadowsTextureResolution);
}
/*
const ID3D11ShaderResourceView* ShadowUtilities::GetShadowShaderResourceView() {
	return m_shadowShaderResourceView;
}*/