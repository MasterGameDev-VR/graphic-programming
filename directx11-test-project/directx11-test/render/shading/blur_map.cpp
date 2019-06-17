#include "stdafx.h"
#include "blur_map.h"
#include <service/locator.h>
#include "DirectXMath.h"

using namespace DirectX;

xtest::render::shading::BlurMap::BlurMap(uint32 width, uint32 height)
{
	m_width = width;
	m_height = height;
}

void xtest::render::shading::BlurMap::Init()
{
	// already initialized
	if (m_shaderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
	// create the ssao map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = m_width;
	textureDesc.Height = m_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), 0, &m_shaderView));

	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(texture.Get(), 0, &m_renderTargetView));


	//BIND VERTEX INPUT

	SSAOData::VertexInAmbientOcclusion *v1 = new SSAOData::VertexInAmbientOcclusion(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f));
	SSAOData::VertexInAmbientOcclusion *v2 = new SSAOData::VertexInAmbientOcclusion(DirectX::XMFLOAT3(-1.0f, +1.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f));
	SSAOData::VertexInAmbientOcclusion *v3 = new SSAOData::VertexInAmbientOcclusion(DirectX::XMFLOAT3(+1.0f, +1.0f, 0.0f), DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f));
	SSAOData::VertexInAmbientOcclusion *v4 = new SSAOData::VertexInAmbientOcclusion(DirectX::XMFLOAT3(+1.0f, -1.0f, 0.0f), DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f));

	m_vs_data.vertices.push_back(*v1);
	m_vs_data.vertices.push_back(*v2);
	m_vs_data.vertices.push_back(*v3);
	m_vs_data.vertices.push_back(*v4);

	m_vs_data.indices.push_back(0);
	m_vs_data.indices.push_back(1);
	m_vs_data.indices.push_back(2);
	m_vs_data.indices.push_back(0);
	m_vs_data.indices.push_back(2);
	m_vs_data.indices.push_back(3);

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = UINT(sizeof(SSAOData::VertexInAmbientOcclusion) * m_vs_data.vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &m_vs_data.vertices[0];
	XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_d3dVertexBuffer));

	// index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_vs_data.indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdata;
	indexInitdata.pSysMem = &m_vs_data.indices[0];
	XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_d3dIndexBuffer));

	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;
	m_viewport.Width = float(m_width);
	m_viewport.Height = float(m_height);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

}

ID3D11ShaderResourceView * xtest::render::shading::BlurMap::AsShaderView()
{
	XTEST_ASSERT(m_shaderView, L"Blur map uninitialized");
	return m_shaderView.Get();
}

ID3D11RenderTargetView * xtest::render::shading::BlurMap::AsRenderTargetView()
{
	XTEST_ASSERT(m_renderTargetView, L"Blur map uninitialized");
	return m_renderTargetView.Get();
}

D3D11_VIEWPORT xtest::render::shading::BlurMap::Viewport() const
{
	return m_viewport;
}

void xtest::render::shading::BlurMap::SetViewport(D3D11_VIEWPORT & view)
{
	m_viewport = view;
}

void xtest::render::shading::BlurMap::ResetTargetView(uint32 n_width, uint32 n_height)
{
	m_renderTargetView.Reset();
	m_shaderView.Reset();
	// already initialized
	if (m_shaderView)
	{
		return;
	}

	ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
	// create the ssao map texture
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = n_width;
	textureDesc.Height = n_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R16_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
	XTEST_D3D_CHECK(d3dDevice->CreateTexture2D(&textureDesc, nullptr, &texture));

	XTEST_D3D_CHECK(d3dDevice->CreateShaderResourceView(texture.Get(), 0, &m_shaderView));

	XTEST_D3D_CHECK(d3dDevice->CreateRenderTargetView(texture.Get(), 0, &m_renderTargetView));
}



uint32 xtest::render::shading::BlurMap::Width() const
{
	return m_width;
}
uint32 xtest::render::shading::BlurMap::Height() const
{
	return m_height;
}

float xtest::render::shading::BlurMap::RandomFloat(float min, float max)
{
	float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
	return min + scale * (max - min);      /* [min, max] */
}

void xtest::render::shading::BlurMap::BuildFrustumFarCorners(float aspect, float fovy, float farZ)
{
	float halfHeight = farZ * tanf(0.5f * fovy);
	float halfWidth = aspect * halfHeight;

	m_frustumFarCorner[0] = DirectX::XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_frustumFarCorner[1] = DirectX::XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_frustumFarCorner[2] = DirectX::XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	m_frustumFarCorner[3] = DirectX::XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);

}

DirectX::XMFLOAT4* xtest::render::shading::BlurMap::GetFrustumFarCorner()
{
	return m_frustumFarCorner;
}


void xtest::render::shading::BlurMap::Bind()
{
	XTEST_ASSERT(m_d3dVertexBuffer && m_d3dIndexBuffer, L"uninitialized renderable");

	UINT stride = sizeof(xtest::render::shading::SSAOData::VertexInAmbientOcclusion);
	UINT offset = 0;
	service::Locator::GetD3DContext()->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	service::Locator::GetD3DContext()->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void xtest::render::shading::BlurMap::Draw()
{
	Bind();
	service::Locator::GetD3DContext()->DrawIndexed(UINT(m_vs_data.indices.size()), 0, 0);
}
