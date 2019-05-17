#include "stdafx.h"
#include "ShadowUtilities.h"

using xtest::demo::ShadowUtilities;
using Microsoft::WRL::ComPtr;

void ShadowUtilities::CreateShadowDepthStencilBuffer() 
{
	// release any previous references
	//the following two lines could be useful in case of repeated calls to this method
	//probably not
	m_shadowDepthBuffer.Reset();
	m_shadowDepthBufferView.Reset();

	D3D11_TEXTURE2D_DESC shadowDepthBufferDesc;
	shadowDepthBufferDesc.Width = GetCurrentWidth();
	shadowDepthBufferDesc.Height = GetCurrentHeight();
	shadowDepthBufferDesc.MipLevels = 1;
	shadowDepthBufferDesc.ArraySize = 1;
	shadowDepthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	shadowDepthBufferDesc.SampleDesc.Count = 1;		// no MSAA
	shadowDepthBufferDesc.SampleDesc.Quality = 0;	// no MSAA
	shadowDepthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	shadowDepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL| D3D11_BIND_SHADER_RESOURCE;
	shadowDepthBufferDesc.CPUAccessFlags = 0;
	shadowDepthBufferDesc.MiscFlags = 0;

	// create the depth buffer and its view
	//you need to know the device!!
	XTEST_D3D_CHECK(m_d3dDevice->CreateTexture2D(&shadowDepthBufferDesc, nullptr, &m_shadowDepthBuffer));
	XTEST_D3D_CHECK(m_d3dDevice->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, &m_depthBufferView));
}