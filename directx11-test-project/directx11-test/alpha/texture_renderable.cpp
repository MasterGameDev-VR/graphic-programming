#include "stdafx.h"
#include "texture_renderable.h"
#include <alpha/alpha_vertex_input_types.h>

void alpha::TextureRenderable::Init()
{
	if (m_d3dVertexBuffer && m_d3dIndexBuffer)
	{
		return;
	}
	
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	TextureVertex vertices[4];
	ZeroMemory(vertices, sizeof(vertices));

	vertices[0].position = DirectX::XMFLOAT3(-1.f, 1.f, 0.0f);  // Top left
	vertices[0].uv = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(1.f, -1.f, 0.0f);  // Bottom right
	vertices[1].uv = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(-1.f, -1.f, 0.0f);  // Bottom left
	vertices[2].uv = DirectX::XMFLOAT2(0.0f, 1.0f);

	vertices[3].position = DirectX::XMFLOAT3(1.f, 1.f, 0.0f);  // Top right
	vertices[3].uv = DirectX::XMFLOAT2(1.0f, 0.0f);

	uint32 indices[] =
	{
		0, 1, 2,
		0, 3, 1
	};

	m_vertexCount = 4;
	m_indexCount = 6;

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(TextureVertex) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	XTEST_D3D_CHECK(xtest::service::Locator::GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexData, &m_d3dVertexBuffer));


	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint32) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	XTEST_D3D_CHECK(xtest::service::Locator::GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_d3dIndexBuffer));
}

void alpha::TextureRenderable::Bind()
{
	XTEST_ASSERT(m_d3dVertexBuffer && m_d3dIndexBuffer, L"uninitialized renderable");

	UINT stride = sizeof(TextureVertex);
	UINT offset = 0;
	xtest::service::Locator::GetD3DContext()->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	xtest::service::Locator::GetD3DContext()->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}

/*
void alpha::TextureRenderable::Update(float textureWidth, float textureHieght, float positionX, float positionY)
{
	float left, right, top, bottom;
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	TextureVertex* verticesPtr;

	// Calculate the screen coordinates of the left side of the bitmap.
	left = (float)((textureWidth / 2) * -1) + (float)positionX;

	// Calculate the screen coordinates of the right side of the bitmap.
	right = left + (float)textureWidth;

	// Calculate the screen coordinates of the top of the bitmap.
	top = (float)(textureHieght / 2) - (float)positionY;

	// Calculate the screen coordinates of the bottom of the bitmap.
	bottom = top - (float)textureHieght;

	TextureVertex vertices[6];

	vertices[0].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[0].uv = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[1].uv = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = DirectX::XMFLOAT3(left, bottom, 0.0f);  // Bottom left.
	vertices[2].uv = DirectX::XMFLOAT2(0.0f, 1.0f);

	// Second triangle.
	vertices[3].position = DirectX::XMFLOAT3(left, top, 0.0f);  // Top left.
	vertices[3].uv = DirectX::XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = DirectX::XMFLOAT3(right, top, 0.0f);  // Top right.
	vertices[4].uv = DirectX::XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = DirectX::XMFLOAT3(right, bottom, 0.0f);  // Bottom right.
	vertices[5].uv = DirectX::XMFLOAT2(1.0f, 1.0f);


	XTEST_D3D_CHECK(xtest::service::Locator::GetD3DContext()->Map(m_d3dVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
	memcpy(mappedResource.pData, vertices, (sizeof(TextureVertex) * m_vertexCount));
	xtest::service::Locator::GetD3DContext()->Unmap(m_d3dVertexBuffer.Get(), 0);
}
*/

void alpha::TextureRenderable::Draw()
{
	Bind();
	xtest::service::Locator::GetD3DContext()->DrawIndexed(m_indexCount, 0, 0);
}
