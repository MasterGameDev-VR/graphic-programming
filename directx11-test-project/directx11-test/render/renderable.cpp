#include "stdafx.h"
#include "renderable.h"
#include <external_libs/directxtk/WICTextureLoader.h>
#include <service/locator.h>
#include <common/utils.h>

using namespace xtest;
using xtest::render::Renderable;


Renderable::Renderable(const mesh::GPFMesh& mesh)
	: m_mesh(mesh)
	, m_W()
	, m_d3dVertexBuffer(nullptr)
	, m_d3dIndexBuffer(nullptr)
	, m_textureMapByUsageMapByMeshName()
	, m_texcoordMatrixMapByMeshName()
	, m_meshNames()
{
	for (mesh::GPFMesh::DescriptorMap::const_reference meshNamePairWithDescriptor : m_mesh.meshDescriptorMapByName)
	{
		m_meshNames.push_back(meshNamePairWithDescriptor.first);
		XMStoreFloat4x4(&m_texcoordMatrixMapByMeshName[meshNamePairWithDescriptor.first], DirectX::XMMatrixIdentity());
	}
}


Renderable::Renderable(
	const mesh::MeshData& mesh,
	const mesh::MeshMaterial& material)
	: m_mesh()
	, m_W()
	, m_d3dVertexBuffer(nullptr)
	, m_d3dIndexBuffer(nullptr)
	, m_textureMapByUsageMapByMeshName()
	, m_texcoordMatrixMapByMeshName()
	, m_meshNames()
{
	// build a GPFmesh from MeshData and MeshMaterial to uniform the code
	mesh::GPFMesh::MeshDescriptor descriptor;
	descriptor.indexCount = uint32(mesh.indices.size());
	descriptor.vertexCount = uint32(mesh.vertices.size());
	descriptor.indexOffset = 0;
	descriptor.vertexOffset = 0;
	descriptor.material = material;

	m_mesh.meshData = mesh;
	m_mesh.meshDescriptorMapByName[""] = descriptor;

	for (mesh::GPFMesh::DescriptorMap::const_reference meshNamePairWithDescriptor : m_mesh.meshDescriptorMapByName)
	{
		m_meshNames.push_back(meshNamePairWithDescriptor.first);
		m_texcoordMatrixMapByMeshName[meshNamePairWithDescriptor.first] = {};
	}

}


void Renderable::Init()
{
	// already initialized
	if (m_d3dVertexBuffer && m_d3dIndexBuffer)
	{
		return;
	}

	
	// vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.ByteWidth = UINT(sizeof(mesh::MeshData::Vertex) * m_mesh.meshData.vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &m_mesh.meshData.vertices[0];
	XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexInitData, &m_d3dVertexBuffer));


	// index buffer
	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.ByteWidth = UINT(sizeof(uint32) * m_mesh.meshData.indices.size());
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitdata;
	indexInitdata.pSysMem = &m_mesh.meshData.indices[0];
	XTEST_D3D_CHECK(service::Locator::GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexInitdata, &m_d3dIndexBuffer));


	// load textures
	for (mesh::GPFMesh::DescriptorMap::const_reference namePairWithDescriptor : m_mesh.meshDescriptorMapByName)
	{
		const std::string& meshName = namePairWithDescriptor.first;
		const mesh::MeshMaterial& material = namePairWithDescriptor.second.material;
		m_textureMapByUsageMapByMeshName[meshName][shading::TextureUsage::color] = service::Locator::GetResourceLoader()->LoadTexture(material.diffuseMap).d3dShaderView;
		m_textureMapByUsageMapByMeshName[meshName][shading::TextureUsage::normal] = service::Locator::GetResourceLoader()->LoadTexture(material.normalMap).d3dShaderView;
		m_textureMapByUsageMapByMeshName[meshName][shading::TextureUsage::glossiness] = service::Locator::GetResourceLoader()->LoadTexture(material.glossMap).d3dShaderView;
	}
}



void Renderable::SetTransform(const DirectX::FXMMATRIX& W)
{
	XMStoreFloat4x4(&m_W, W);
}

void Renderable::SetTexcoordTransform(const DirectX::FXMMATRIX& T, const std::string& meshName)
{
	XTEST_ASSERT(std::find(m_meshNames.begin(), m_meshNames.end(), std::string(meshName)) != m_meshNames.end(), L"mesh name '%s' not found", common::ToWString(meshName).c_str());
	XMStoreFloat4x4(&m_texcoordMatrixMapByMeshName[meshName], T);
}

const DirectX::XMFLOAT4X4& Renderable::GetTransform() const
{
	return m_W;
}

const DirectX::XMFLOAT4X4& Renderable::GetTexcoordTransform(const std::string& meshName) const
{
	return m_texcoordMatrixMapByMeshName.at(meshName);
}

const mesh::MeshMaterial& Renderable::GetMaterial(const std::string& meshName) const
{
	return m_mesh.meshDescriptorMapByName.at(meshName).material;
}

const ID3D11ShaderResourceView* Renderable::GetTextureView(render::shading::TextureUsage textureUsage, const std::string& meshName) const
{
	return m_textureMapByUsageMapByMeshName.at(meshName).at(textureUsage).Get();
}

const std::vector<std::string>& Renderable::GetMeshNames() const
{
	return m_meshNames;
}



void Renderable::Bind()
{
	XTEST_ASSERT(m_d3dVertexBuffer && m_d3dIndexBuffer, L"uninitialized renderable");

	UINT stride = sizeof(mesh::MeshData::Vertex);
	UINT offset = 0;
	service::Locator::GetD3DContext()->IASetVertexBuffers(0, 1, m_d3dVertexBuffer.GetAddressOf(), &stride, &offset);
	service::Locator::GetD3DContext()->IASetIndexBuffer(m_d3dIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}



void Renderable::Draw(const std::string& meshName)
{
	Bind();
	const mesh::GPFMesh::MeshDescriptor& descriptor = m_mesh.meshDescriptorMapByName.at(meshName);
	service::Locator::GetD3DContext()->DrawIndexed(descriptor.indexCount, descriptor.indexOffset, descriptor.vertexOffset);
}


