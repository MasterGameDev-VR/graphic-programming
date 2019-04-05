#include "stdafx.h"
#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <mesh/mesh_generator.h>
#include <unordered_map>
#include <set>
#include <external_libs/tiny_obj_loader/tiny_obj_loader.h>


using xtest::file::BinaryFile;


namespace
{
	struct GPFMeshHeader
	{
		char name[24] = { 0 };
		uint32 vertexCount = 0;
		uint32 indexCount = 0;
		uint32 vertexOffset = 0;
		uint32 indexOffset = 0;
		char unused[24] = { 0 };
	};
}


std::future<BinaryFile> xtest::file::ReadBinaryFile(std::wstring filePath)
{
	return std::async(std::launch::async, [filePath]()
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

		BinaryFile binaryFile(std::move(byteArray), byteSize);
		return binaryFile;
	});
}


xtest::mesh::GPFMesh xtest::file::ReadGPF(const std::wstring& filePath)
{
	std::future<file::BinaryFile> fileFuture = file::ReadBinaryFile(filePath);
	file::BinaryFile gpf = fileFuture.get();
	mesh::GPFMesh gpfMesh;


	// reading gpf file:
	//
	// 		+---------------------------------------------------------------------------------------------------------------------- - +
	// 		| mesh  | name | vertex | index  | vertex | index  | Unused | ... |   mesh 1   | mesh 2   | ... | mesh 1  | mesh 2  | ... |
	// 		| count |      | count  | count  | offset | offset |        |     |   Vertices | Vertices |     | Indices | Indices |     |
	// 		+---------------------------------------------------------------------------------------------------------------------- - +
	// 	       4b     24b      4b       4b      4b      4b        24b
	// 			    ^                                                   ^
	// 			    |                                                   |
	// 			    +-------------------------------------------------- +
	// 			                       gpfMeshHeader
	// 			                            64b

	const int32* meshCountPtr = reinterpret_cast<const int32*>(gpf.Data());
	int32 meshCount = *meshCountPtr;
	meshCountPtr++;
	
	const GPFMeshHeader* meshHeaderPtr = reinterpret_cast<const GPFMeshHeader*>(meshCountPtr);
	uint32 totalVertexCount = 0;
	uint32 totalIndexCount = 0;

	for (int32 meshHeaderIndex = 0; meshHeaderIndex < meshCount; meshHeaderIndex++)
	{
		GPFMeshHeader meshHeader = *meshHeaderPtr;
		meshHeaderPtr++;

		mesh::GPFMesh::MeshDescriptor meshDesc;
		meshDesc.indexCount = meshHeader.indexCount;
		meshDesc.vertexCount = meshHeader.vertexCount;
		meshDesc.indexOffset = meshHeader.indexOffset;
		meshDesc.vertexOffset = meshHeader.vertexOffset;
		
		gpfMesh.meshDescriptorMapByName[std::string(meshHeader.name)] = meshDesc;
		totalVertexCount += meshDesc.vertexCount;
		totalIndexCount += meshDesc.indexCount;
	}


	gpfMesh.meshData.vertices.resize(totalVertexCount);
	gpfMesh.meshData.indices.resize(totalIndexCount);
	

	const char* data = reinterpret_cast<const char*>(meshHeaderPtr);
	std::memcpy(&gpfMesh.meshData.vertices[0], data, sizeof(mesh::MeshData::Vertex) * gpfMesh.meshData.vertices.size());

	data += sizeof(mesh::MeshData::Vertex) * gpfMesh.meshData.vertices.size();
	std::memcpy(&gpfMesh.meshData.indices[0], data, sizeof(int32) * gpfMesh.meshData.indices.size());

	return gpfMesh;
}


void xtest::file::WriteGPFOnDiskFromObj(const std::wstring& inputFile, const std::wstring & outputFile)
{

	auto HashFn = [](const mesh::MeshData::Vertex& vertex) 
	{
		return std::hash<float>{}(vertex.position.x)
			^ (std::hash<float>{}(vertex.position.y) << 1)
			^ (std::hash<float>{}(vertex.position.z) << 1)
			^ (std::hash<float>{}(vertex.normal.x) << 1)
			^ (std::hash<float>{}(vertex.normal.y) << 1)
			^ (std::hash<float>{}(vertex.normal.z) << 1)
			^ (std::hash<float>{}(vertex.tangentU.x) << 1)
			^ (std::hash<float>{}(vertex.tangentU.y) << 1)
			^ (std::hash<float>{}(vertex.tangentU.z) << 1)
			^ (std::hash<float>{}(vertex.uv.x) << 1)
			^ (std::hash<float>{}(vertex.uv.y) << 1);
	};
	
	typedef std::unordered_map<mesh::MeshData::Vertex, uint32, decltype(HashFn)> VertexIndexMap;



 	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	//workaround: tiny object doesn't support multi byte string
	std::string inputFile_asString(inputFile.begin(), inputFile.end());

	bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputFile_asString.c_str());
	XTEST_ASSERT(result, L"tiny object loader failed to parse the file");

	if (result)
	{
	    mesh::MeshData meshData;
		std::vector<GPFMeshHeader> gpfMeshHeaders;
		uint32 gpfVertexOffset = 0;
		uint32 gpfIndexOffset = 0;
		
		for (size_t shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
		{
			VertexIndexMap indexMapByVertex(shapes[shapeIndex].mesh.indices.size(), HashFn);
			GPFMeshHeader gpfMeshHeader;
			uint32 gpfVertexCount = 0;
			uint32 gpfIndexCount = 0;
			uint32 meshDataIndex = 0;
			

			size_t tyniobjIndexOffset = 0;
			for (size_t faceIndex = 0; faceIndex < shapes[shapeIndex].mesh.num_face_vertices.size(); faceIndex++) 
			{
				const size_t faceVertexCount = shapes[shapeIndex].mesh.num_face_vertices[faceIndex];
				for (size_t tyniobjVertexIndex = 0; tyniobjVertexIndex < faceVertexCount; tyniobjVertexIndex++) 
				{
					// access to vertex data
					tinyobj::index_t idx = shapes[shapeIndex].mesh.indices[tyniobjIndexOffset + tyniobjVertexIndex];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

					// merge vertex data
					mesh::MeshData::Vertex meshDataVertex = { { vx, vy, vz },{ nx, ny, nz },{ 0.f, 0.f, 0.f },{ tx, ty } };

					// keeps vertex unique per shape, but store all vertex buffers and index buffer together
					VertexIndexMap::iterator iter = indexMapByVertex.find(meshDataVertex);
					if (iter == indexMapByVertex.end())
					{
						meshData.vertices.push_back(meshDataVertex);
						meshData.indices.push_back(meshDataIndex);
						indexMapByVertex[meshDataVertex] = meshDataIndex++;
						gpfVertexCount++;
						gpfIndexCount++;
					}
					else
					{
						meshData.indices.push_back(iter->second);
						gpfIndexCount++;
					}
				}
				tyniobjIndexOffset += faceVertexCount;
			}

			// register mesh header data
			// fixed name width for our format (24 in total, null terminator included)
			const char* shapeName_cstr = shapes[shapeIndex].name.c_str();
			std::memcpy(&gpfMeshHeader.name, shapeName_cstr, std::min(sizeof(gpfMeshHeader.name) - 1, std::strlen(shapeName_cstr)));

			// vertex index data
			gpfMeshHeader.vertexCount = gpfVertexCount;
			gpfMeshHeader.indexCount = gpfIndexCount;
			gpfMeshHeader.vertexOffset = gpfVertexOffset;
			gpfMeshHeader.indexOffset = gpfIndexOffset;


			gpfMeshHeaders.push_back(gpfMeshHeader);
			gpfVertexOffset += gpfVertexCount;
			gpfIndexOffset += gpfIndexCount;
		}



		// write gpf file on disk following this format:
		//
		// 		+---------------------------------------------------------------------------------------------------------------------- - +
	    // 		| mesh  | name | vertex | index  | vertex | index  | Unused | ... |   mesh 1   | mesh 2   | ... | mesh 1  | mesh 2  | ... |
		// 		| count |      | count  | count  | offset | offset |        |     |   Vertices | Vertices |     | Indices | Indices |     |
		// 		+---------------------------------------------------------------------------------------------------------------------- - +
		// 	       4b     24b      4b       4b      4b      4b        24b
		// 			    ^                                                   ^
		// 			    |                                                   |
		// 			    +-------------------------------------------------- +
		// 			                       gpfMeshHeader
		// 			                            64b

		std::ofstream outPutFile(outputFile.c_str(), std::ifstream::binary);

		int32 meshCount = int32(gpfMeshHeaders.size());
		outPutFile.write(reinterpret_cast<char*>(&meshCount), sizeof(int32));

		for (GPFMeshHeader& gpfMeshHeader : gpfMeshHeaders)
		{
			outPutFile.write(reinterpret_cast<char*>(&gpfMeshHeader), sizeof(GPFMeshHeader));
		}
		outPutFile.write(reinterpret_cast<char*>(&meshData.vertices[0]), sizeof(mesh::MeshData::Vertex) * meshData.vertices.size());
		outPutFile.write(reinterpret_cast<char*>(&meshData.indices[0]), sizeof(int32) * meshData.indices.size());
	}
}

