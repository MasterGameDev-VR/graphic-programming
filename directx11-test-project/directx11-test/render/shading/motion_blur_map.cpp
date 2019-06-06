#include "stdafx.h"
#include "motion_blur_map.h"
#include <service/locator.h>

using namespace xtest::render::shading;
using namespace DirectX;
using namespace xtest;
using namespace xtest::camera;


MotionBlurMap::MotionBlurMap(uint32 resolution):
	//  m_width(width)
	//, m_height(height)
	  m_resolution(resolution)
	, m_motionBlurView(nullptr)
	, m_depthStencilView(nullptr)
	, m_V()
	, m_P()
	, m_VPT()
{
	//è necessario settare questi due dati con la matrice identità
	XMStoreFloat4x4(&m_data.WVP_previousFrame, XMMatrixIdentity());
	XMStoreFloat4x4(&m_data.WVP_currentFrame, XMMatrixIdentity());

	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<float>(m_resolution);
	m_viewport.Height = static_cast<float>(m_resolution);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}


void MotionBlurMap::Init()
{
	// already initialized
	if (m_motionBlurView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();


	// create the shadow map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = m_resolution;
	textureDesc.Height = m_resolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32_FLOAT; // typeless is required since the shader view and the depth stencil view will interpret it differently
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));



	// create the view used by the output merger state
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	depthStencilViewDesc.Flags = 0;
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, &m_depthStencilView));

	//create the view used by the shader
	D3D11_SHADER_RESOURCE_VIEW_DESC motionBlurViewDesc;
	motionBlurViewDesc.Format = DXGI_FORMAT_R32G32_FLOAT; // 24bit red channel (depth), 8 bit unused (stencil)
	motionBlurViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	motionBlurViewDesc.Texture2D.MipLevels = 1;
	motionBlurViewDesc.Texture2D.MostDetailedMip = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), &motionBlurViewDesc, &m_motionBlurView));

}

//qui è necessario risolvere un problema riguardante il dato da inviare al buffer:
//per il fatto che esso deve contenere due matrici W, ognuna relativa a uno dei due frame consecutivi
//è necessario mettere l'oggetto fuori dal blocco del metodo ToPerObjectMotionBlurData

//le matrici V e P le prendo da m_camera? in questo caso non ho nessun membro m_camera
//XMMATRIX V = m_camera.GetViewMatrix();
//XMMATRIX P = m_camera.GetProjectionMatrix();
//XMMATRIX WVP = W * m_shadowMap.LightViewMatrix() * m_shadowMap.LightProjMatrix();


//in questo caso non c'è da calcolare delle matrici in base alla direzione della luce
//inoltre non vanno passate nuovamente all'esterno della classe con metodi simili a LightViewMatrix
//e a LightProjMatrix
//quindi non ho usato un valore di flag simile a m_isDirty
void MotionBlurMap::SetViewAndProjectionMatrices(const SphericalCamera& camera)
{
	XMStoreFloat4x4(&m_V, camera.GetViewMatrix());
	XMStoreFloat4x4(&m_P, camera.GetProjectionMatrix());

}

ID3D11ShaderResourceView* MotionBlurMap::AsMotionBlurView()
{
	XTEST_ASSERT(m_motionBlurView, L"shadow map uninitialized");
	return m_motionBlurView.Get();
}


ID3D11DepthStencilView* MotionBlurMap::AsDepthStencilView()
{
	XTEST_ASSERT(m_depthStencilView, L"shadow map uninitialized");
	return m_depthStencilView.Get();
}
D3D11_VIEWPORT MotionBlurMap::Viewport() const
{
	return m_viewport;
}

MotionBlurMap::PerObjectMotionBlurMapData MotionBlurMap::ToPerObjectMotionBlurMapData(const render::Renderable& renderable, const std::string& meshName)
{
	XTEST_UNUSED_VAR(meshName);
	m_data.dataSetFilled = false;

	//in teoria basta passare una sola matrice W per volta
	//questa, moltiplicata per V e P verrà salvata in WVP_currentFrame e poi passerà a previousFrame
	XMMATRIX W = XMLoadFloat4x4(&renderable.GetTransform());
	XMMATRIX WVP = W * XMLoadFloat4x4(&m_V)*  XMLoadFloat4x4(&m_P);

	m_data.WVP_previousFrame = m_data.WVP_currentFrame;
	XMStoreFloat4x4(&m_data.WVP_currentFrame, XMMatrixTranspose(WVP));
	XMFLOAT4X4 identityMatrix;
	XMStoreFloat4x4(&identityMatrix, XMMatrixIdentity());
	//questa 'if' serve per dire al vertex shader che sono pronte le matrici relative ai due frame consecutivi
	if (m_data.WVP_previousFrame.m != identityMatrix.m) {
		m_data.dataSetFilled = true;
	}
	return m_data;
}
