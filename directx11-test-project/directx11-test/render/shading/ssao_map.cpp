#include "stdafx.h"
#include "ssao_map.h"
#include <service/locator.h>
#include "DirectXMath.h"

using namespace DirectX;

xtest::render::shading::SSAOData::VertexInAmbientOcclusion::VertexInAmbientOcclusion(DirectX::XMFLOAT3 inPos, DirectX::XMFLOAT3 inToFarPlaneIndex, DirectX::XMFLOAT2 inUv)
{
	pos = inPos;
	toFarPlaneIndex = inToFarPlaneIndex;
	uv = inUv;
}

bool xtest::render::shading::SSAOData::VertexInAmbientOcclusion::operator==(const VertexInAmbientOcclusion & other) const
{
	return pos.x == other.pos.x
		&& pos.y == other.pos.y
		&& pos.z == other.pos.z
		&& toFarPlaneIndex.x == other.toFarPlaneIndex.x
		&& toFarPlaneIndex.y == other.toFarPlaneIndex.y
		&& toFarPlaneIndex.z == other.toFarPlaneIndex.z
		&& uv.x == other.uv.x
		&& uv.y == other.uv.y;
}

xtest::render::shading::SSAOMap::SSAOMap(uint32 width, uint32 height)
{
	m_width = width;
	m_height = height;
}

void xtest::render::shading::SSAOMap::Init()
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

	BuildKernelVectors();
}

ID3D11ShaderResourceView * xtest::render::shading::SSAOMap::AsShaderView()
{
	XTEST_ASSERT(m_shaderView, L"SSAO map uninitialized");
	return m_shaderView.Get();
}

ID3D11RenderTargetView * xtest::render::shading::SSAOMap::AsRenderTargetView()
{
	XTEST_ASSERT(m_renderTargetView, L"SSAO map uninitialized");
	return m_renderTargetView.Get();
}


D3D11_VIEWPORT xtest::render::shading::SSAOMap::Viewport() const
{
	return m_viewport;
}

void xtest::render::shading::SSAOMap::SetViewport(D3D11_VIEWPORT & view)
{
	m_viewport = view;
}

void xtest::render::shading::SSAOMap::ResetTargetView(uint32 n_width, uint32 n_height)
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



uint32 xtest::render::shading::SSAOMap::Width() const
{
	return m_width;
}
uint32 xtest::render::shading::SSAOMap::Height() const
{
	return m_height;
}

float xtest::render::shading::SSAOMap::RandomFloat(float min, float max)
{
		float scale = rand() / (float)RAND_MAX; /* [0, 1.0] */
		return min + scale * (max - min);      /* [min, max] */
}

void xtest::render::shading::SSAOMap::BuildKernelVectors()
{
	/*
	We vary the x and y direction in tangent space between -1.0 and 1.0 and vary the z direction
	of the samples between 0.0 and 1.0 (if we varied the z direction between -1.0 and 1.0 as well
	we'd have a sphere sample kernel). As the sample kernel will be oriented along the surface normal,
	the resulting sample vectors will all end up in the hemisphere.

	Currently, all samples are randomly distributed in the sample kernel, but we'd rather place
	a larger weight on occlusions close to the actual fragment as to distribute the kernel samples
	closer to the origin. We can do this with an accelerating interpolation function:
	*/
	for (unsigned int i = 0; i < SSAOData::SAMPLE_COUNT; ++i)
	{
		m_offsets[i] = DirectX::XMFLOAT4(
			(SSAOMap::RandomFloat(0.0f, 1.0f) * 2.0f - 1.0f),
			(SSAOMap::RandomFloat(0.0f, 1.0f) * 2.0f - 1.0f),
			SSAOMap::RandomFloat(0.0f, 1.0f),
			0.0f
		);

		float s = SSAOMap::RandomFloat(0.1f, 1.0f);
		float scale = (float)i / SSAOData::SAMPLE_COUNT;
		scale = SSAOMap::lerp(0.1f, 1.0f, scale * scale);
		s *= scale;
		DirectX::XMVECTOR v = s * XMVector4Normalize(DirectX::XMLoadFloat4(&m_offsets[i]));

		DirectX::XMStoreFloat4(&m_offsets[i], v);
	}
}

void xtest::render::shading::SSAOMap::BuildFrustumFarCorners(float aspect,float fovy, float farZ)
{
	float halfHeight = farZ * tanf(0.5f * fovy);
	float halfWidth = aspect * halfHeight;

	m_frustumFarCorner[0] = DirectX::XMFLOAT4(-halfWidth, -halfHeight, farZ, 0.0f);
	m_frustumFarCorner[1] = DirectX::XMFLOAT4(-halfWidth, +halfHeight, farZ, 0.0f);
	m_frustumFarCorner[2] = DirectX::XMFLOAT4(+halfWidth, +halfHeight, farZ, 0.0f);
	m_frustumFarCorner[3] = DirectX::XMFLOAT4(+halfWidth, -halfHeight, farZ, 0.0f);

}

DirectX::XMFLOAT4* xtest::render::shading::SSAOMap::GetFrustumFarCorner()
{
	return m_frustumFarCorner;
}

DirectX::XMFLOAT4* xtest::render::shading::SSAOMap::GetKernelVectors()
{
	return m_offsets;
}

float  xtest::render::shading::SSAOMap::lerp(float a, float b, float f)
{
	return a + f * (b - a);
}

void xtest::render::shading::SSAOMap::Bind()
{
	XTEST_ASSERT(m_d3dVertexBuffer && m_d3dIndexBuffer, L"uninitialized renderable");

	UINT stride = sizeof(SSAOData::VertexInAmbientOcclusion);
	UINT offset = 0;
	service::Locator::GetD3DContext()->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	service::Locator::GetD3DContext()->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

void xtest::render::shading::SSAOMap::Draw()
{
	Bind();
	service::Locator::GetD3DContext()->DrawIndexed(UINT(m_vs_data.indices.size()), 0, 0);
}

void xtest::render::shading::SSAOMap::Release()
{
	m_renderTargetView->Release();
}