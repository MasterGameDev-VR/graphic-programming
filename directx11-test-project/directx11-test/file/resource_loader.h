#pragma once

#include <file/binary_file.h>
#include <mesh/mesh_format.h>

namespace xtest {
namespace file {

	class ResourceLoader
	{
	public:

		struct LoadedTexture
		{
			ID3D11Resource* d3dTexture;
			ID3D11ShaderResourceView* d3dShaderView;
		};


		ResourceLoader();
		ResourceLoader(ResourceLoader&&) = default;
		ResourceLoader& operator=(ResourceLoader&&) = default;

		ResourceLoader(const ResourceLoader&) = delete;
		ResourceLoader& operator=(const ResourceLoader&) = delete;


		const BinaryFile* LoadBinaryFile(const std::wstring& filePath);
		const mesh::GPFMesh* LoadGPFMesh(const std::wstring& filePath);
		LoadedTexture LoadTexture(const std::wstring& filePath);


	private:
		typedef std::unordered_map<std::wstring, BinaryFile> ByteCodeMap;
		typedef Microsoft::WRL::ComPtr<ID3D11Resource> D3DTexturePtr;
		typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> D3DTextureView;
		typedef std::unordered_map<std::wstring, std::pair<D3DTexturePtr, D3DTextureView>> TextureMap;
		typedef std::unordered_map <std::wstring, mesh::GPFMesh> GPFMeshMap;

		ByteCodeMap m_byteCodeMapByPath;
		TextureMap m_d3dTextureMapByPath;
		GPFMeshMap m_gpfMeshMapByPath;

	};

} //file
} //xtest