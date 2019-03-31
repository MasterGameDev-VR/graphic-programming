#pragma once


namespace xtest {
namespace mesh {


	struct MeshData
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT3 normal;
			DirectX::XMFLOAT3 tangentU;
			DirectX::XMFLOAT2 uv;

			bool operator==(const Vertex& other) const;
			
		};

		std::vector<Vertex> vertices;
		std::vector<uint32> indices;
	};


	struct GPFMesh
	{
		struct MeshDescriptor
		{
			uint32 vertexCount;
			uint32 vertexOffset;
			uint32 indexCount;
			uint32 indexOffset;
		};

		std::map<std::string, MeshDescriptor> meshDescriptorMapByName;
		MeshData meshData;
	};



} // mesh
} // xtest
