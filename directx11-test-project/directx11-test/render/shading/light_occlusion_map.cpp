#include "stdafx.h"
#include "light_occlusion_map.h"
#include <mesh/mesh_generator.h>
#include <service/locator.h>


using namespace xtest::render::shading;
using namespace DirectX;

LightOcclusionMap::LightOcclusionMap(uint32 resolution)
	: m_renderTexture(nullptr)
	, m_renderTargetView(nullptr)
	, m_shaderView(nullptr)
	, m_resolution(resolution)
	, m_bSphere()
	, m_lightPlaceHolder()
	, m_lightDir()
	, m_isDirty(true)
{
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<float>(m_resolution);
	m_viewport.Height = static_cast<float>(m_resolution);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}


void LightOcclusionMap::Init()
{
	// already initialized
	if (m_shaderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();


	// create the occlusion map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = m_resolution;
	textureDesc.Height = m_resolution;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &m_renderTexture));


	D3D11_RENDER_TARGET_VIEW_DESC targetDesc;
	targetDesc.Format = textureDesc.Format;
	targetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	targetDesc.Texture2D.MipSlice = 0;

	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(m_renderTexture.Get(), &targetDesc, &m_renderTargetView));


	D3D11_SHADER_RESOURCE_VIEW_DESC shaderDesc;
	shaderDesc.Format = textureDesc.Format;
	shaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderDesc.Texture2D.MostDetailedMip = 0;
	shaderDesc.Texture2D.MipLevels = 1;

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(m_renderTexture.Get(), &shaderDesc, &m_shaderView));


	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = m_resolution;
	depthDesc.Height = m_resolution;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;		// no MSAA
	depthDesc.SampleDesc.Quality = 0;	// no MSAA
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	// create the depth buffer and its view
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&depthDesc, nullptr, &m_depthBuffer));
	XTEST_D3D_CHECK(d3dDevice->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, &m_depthBufferView));



	// init a white sphere as placeholder
	m_lightPlaceHolder = {
			mesh::GenerateSphere(1.f, 40, 40),
			mesh::MeshMaterial()
	};
	m_lightPlaceHolder.Init();
}


void LightOcclusionMap::SetTargetBoundingSphere(const xtest::scene::BoundingSphere& boundingSphere)
{
	XTEST_ASSERT(boundingSphere.GetRadius() > 0.f);
	m_bSphere = boundingSphere;
	m_isDirty = true;
}


void LightOcclusionMap::SetLight(const DirectX::XMFLOAT3& dirLight, const DirectX::XMFLOAT3& up)
{
	m_lightDir = dirLight;
	XMStoreFloat3(&m_lightDir, XMVector3Normalize(XMLoadFloat3(&m_lightDir)));
	m_isDirty = true;
}


ID3D11RenderTargetView* LightOcclusionMap::AsRenderTargetView()
{
	XTEST_ASSERT(m_shaderView, L"occlusion map uninitialized");
	return m_renderTargetView.Get();
}


ID3D11ShaderResourceView* LightOcclusionMap::AsShaderView()
{
	XTEST_ASSERT(m_shaderView, L"occlusion map uninitialized");
	return m_shaderView.Get();
}


D3D11_VIEWPORT LightOcclusionMap::Viewport() const
{
	return m_viewport;
}


ID3D11DepthStencilView* LightOcclusionMap::DepthBufferView()
{
	return m_depthBufferView.Get();
}

uint32 LightOcclusionMap::Resolution() const
{
	return m_resolution;
}

void LightOcclusionMap::CalcMatrices()
{
	XTEST_ASSERT(m_bSphere.GetRadius() > 0.f, L"no bounding sphere was set");
	XTEST_ASSERT(XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_lightDir))) > 0.f, L"no light direction was set");

	// place it to the edge of the sphere box
	XMFLOAT3 bSpherePosW = m_bSphere.GetPosition();
	XMVECTOR bSpherePos = XMLoadFloat3(&bSpherePosW);
	XMVECTOR lightDir = XMLoadFloat3(&m_lightDir);
	XMVECTOR lightPos = bSpherePos + (-m_bSphere.GetRadius() * lightDir);

	XMFLOAT3 placeHolderPos;
	XMStoreFloat3(&placeHolderPos, lightPos);
	m_lightPlaceHolder.SetTransform(XMMatrixTranslation(placeHolderPos.x, placeHolderPos.y, placeHolderPos.z));

	m_isDirty = false;
}


xtest::render::Renderable LightOcclusionMap::LightPlaceHolder()
{
	if (m_isDirty)
	{
		CalcMatrices();
	}
	return m_lightPlaceHolder;
}

DirectX::XMMATRIX LightOcclusionMap::TMatrix() const
{
	// used to map NDC coordinates to the texture space [-1, 1] -> [0,1]
	return
	{
		0.5f,  0.f , 0.f, 0.f,
		0.0f, -0.5f, 0.f, 0.f,
		0.0f,  0.f , 1.f, 0.f,
		0.5f,  0.5f, 0.f, 1.f,
	};
}