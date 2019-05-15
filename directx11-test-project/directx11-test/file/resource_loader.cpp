#include "stdafx.h"
#include "resource_loader.h"
#include <file/file_utils.h>
#include <iostream>
#include <fstream>
#include <external_libs/directxtk/WICTextureLoader.h>
#include <service/locator.h>


using namespace xtest::file;


ResourceLoader::ResourceLoader()
	: m_byteCodeMapByPath()
	, m_d3dTextureMapByPath()
	, m_gpfMeshMapByPath()
{}


const BinaryFile* ResourceLoader::LoadBinaryFile(const std::wstring& filePath)
{
	if (m_byteCodeMapByPath.find(filePath) == m_byteCodeMapByPath.end())
	{
		// open the file at the end
		std::ifstream fileStream(filePath, std::ios::binary | std::ios::in | std::ios::ate);
		XTEST_ASSERT(fileStream.is_open(), L"unable to open the file:'%s'", filePath.c_str());

		std::streampos byteSize = fileStream.tellg(); // extract the byte size
		std::unique_ptr<char[]> byteArray(new char[byteSize]);

		// go back to the start of the file
		fileStream.seekg(0, std::ios::beg);
		fileStream.read(byteArray.get(), byteSize);
		fileStream.close();

		m_byteCodeMapByPath[filePath] = BinaryFile(std::move(byteArray), byteSize);
	}

	return &m_byteCodeMapByPath.at(filePath);
}


const xtest::mesh::GPFMesh* xtest::file::ResourceLoader::LoadGPFMesh(const std::wstring& filePath)
{
	if (m_gpfMeshMapByPath.find(filePath) == m_gpfMeshMapByPath.end())
	{
		m_gpfMeshMapByPath[filePath] = file::ReadGPF(filePath);
	}
	return &m_gpfMeshMapByPath.at(filePath);
}



ResourceLoader::LoadedTexture ResourceLoader::LoadTexture(const std::wstring& filePath)
{
	// for now we only load texture with usage default and bind flag shader resource
	if (m_d3dTextureMapByPath.find(filePath) == m_d3dTextureMapByPath.end())
	{
		ID3D11Device* d3dDevice = service::Locator::GetD3DDevice();
		ID3D11DeviceContext* d3dContext = service::Locator::GetD3DContext();
		Microsoft::WRL::ComPtr<ID3D11Resource> d3dTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> d3dShaderView;
		XTEST_D3D_CHECK(DirectX::CreateWICTextureFromFile(d3dDevice, d3dContext, filePath.c_str(), &d3dTexture, &d3dShaderView));

		m_d3dTextureMapByPath[filePath] = std::make_pair(d3dTexture, d3dShaderView);
	}

	std::pair<D3DTexturePtr, D3DTextureView>& texturePairWithView = m_d3dTextureMapByPath.at(filePath);
	return { texturePairWithView.first.Get(), texturePairWithView.second.Get() };
}
