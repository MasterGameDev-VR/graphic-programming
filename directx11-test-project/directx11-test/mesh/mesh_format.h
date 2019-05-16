#pragma once


namespace xtest {
namespace mesh {


	struct MeshData
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT4 tangentU;
			DirectX::XMFLOAT2 uv;

			bool operator==(const Vertex& other) const;
			
		};

		std::vector<Vertex> vertices;
		std::vector<uint32> indices;
	};


	struct MeshMaterial
	{
		DirectX::XMFLOAT4 ambient;
		DirectX::XMFLOAT4 diffuse;
		DirectX::XMFLOAT4 specular;

		std::wstring diffuseMap;
		std::wstring normalMap;
		std::wstring glossMap;
	};


	struct GPFMesh
	{
		struct MeshDescriptor
		{
			uint32 vertexCount;
			uint32 vertexOffset;
			uint32 indexCount;
			uint32 indexOffset;
			MeshMaterial material;
		};

		typedef std::map<std::string, MeshDescriptor> DescriptorMap;

		DescriptorMap meshDescriptorMapByName;
		MeshData meshData;
	};



} // mesh
} // xtest
