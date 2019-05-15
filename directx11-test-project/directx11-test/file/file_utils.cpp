#include "stdafx.h"
#include "file_utils.h"
#include <iostream>
#include <fstream>
#include <mesh/mesh_generator.h>
#include <unordered_map>
#include <set>
#include <external_libs/tiny_obj_loader/tiny_obj_loader.h>
#include <external_libs/mikktspace/mikktspace.h>
#include <external_libs/nlohmann_json/json.hpp>
#include <common/utils.h>


using namespace xtest::file;
using namespace xtest::mesh;

namespace {

	struct GPFMeshHeader
	{
		char name[24] = { 0 };
		uint32 vertexCount = 0;
		uint32 indexCount = 0;
		uint32 vertexOffset = 0;
		uint32 indexOffset = 0;
		char unused[24] = { 0 };
	};


	// mikkTSpace callbacks
	int MikkGetNumFacesFn(const SMikkTSpaceContext* pContext)
	{
		// we only support triangulated meshes
		std::vector<MeshData::Vertex>* vertices = static_cast<std::vector<MeshData::Vertex>*>(pContext->m_pUserData);
		return int(vertices->size() / 3);
	}

	int MikkGetNumVerticesOfFaceFn(const SMikkTSpaceContext* pContext, const int iFace)
	{
		XTEST_UNUSED_VAR(pContext);
		XTEST_UNUSED_VAR(iFace);
		return 3; // we only support triangulated meshes
	}

	void MikkGetPositionFn(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
	{
		std::vector<MeshData::Vertex>* vertices = static_cast<std::vector<MeshData::Vertex>*>(pContext->m_pUserData);
		MeshData::Vertex& vertex = vertices->at(iFace*3 + iVert);
		fvPosOut[0] = vertex.position.x;
		fvPosOut[1] = vertex.position.y;
		fvPosOut[2] = vertex.position.z;
	}

	void MikketNormalFn(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
	{
		std::vector<MeshData::Vertex>* vertices = static_cast<std::vector<MeshData::Vertex>*>(pContext->m_pUserData);
		MeshData::Vertex& vertex = vertices->at(iFace * 3 + iVert);
		fvNormOut[0] = vertex.normal.x;
		fvNormOut[1] = vertex.normal.y;
		fvNormOut[2] = vertex.normal.z;
	}

	void MikkGetTexCoordFn(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
	{
		std::vector<MeshData::Vertex>* vertices = static_cast<std::vector<MeshData::Vertex>*>(pContext->m_pUserData);
		MeshData::Vertex& vertex = vertices->at(iFace * 3 + iVert);
		fvTexcOut[0] = vertex.uv.x;
		fvTexcOut[1] = vertex.uv.y;
	}

	void MikkSetTSpaceBasicFn(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
	{
		std::vector<MeshData::Vertex>* vertices = static_cast<std::vector<MeshData::Vertex>*>(pContext->m_pUserData);
		MeshData::Vertex& vertex = vertices->at(iFace * 3 + iVert);
		vertex.tangentU.x = fvTangent[0];
		vertex.tangentU.y = fvTangent[1];
		vertex.tangentU.z = fvTangent[2];
		vertex.tangentU.w = fSign;
	}	

} // unnamed



// nlohmann json must be present in the same namespace of the object to serialize/deserialize
namespace xtest {
namespace mesh {

	// nlohmann json serialization function
	inline void to_json(nlohmann::json& j, const xtest::mesh::MeshMaterial& m) {
		j = nlohmann::json{
			{ "ambient", std::array<float, 4>{m.ambient.x, m.ambient.y, m.ambient.z, m.ambient.w} },
			{ "diffuse", std::array<float, 4>{m.diffuse.x, m.diffuse.y, m.diffuse.z, m.diffuse.w} },
			{ "specular", std::array<float, 4>{m.specular.x, m.specular.y, m.specular.z, m.specular.w} },
			{ "diffuseMap", xtest::common::ToString(m.diffuseMap) },
			{ "normalMap", xtest::common::ToString(m.normalMap) },
			{ "glossMap", xtest::common::ToString(m.glossMap) },
		};
	}

	// nlohmann json deserialization function
	inline void from_json(const nlohmann::json& j, xtest::mesh::MeshMaterial& m) {

		std::array<float, 4> ambient;
		std::array<float, 4> diffuse;
		std::array<float, 4> specular;
		std::string diffuseMap;
		std::string normalMap;
		std::string glossMap;
		j.at("ambient").get_to(ambient);
		j.at("diffuse").get_to(diffuse);
		j.at("specular").get_to(specular);
		j.at("diffuseMap").get_to(diffuseMap);
		j.at("normalMap").get_to(normalMap);
		j.at("glossMap").get_to(glossMap);

		m.ambient = { ambient[0], ambient[1], ambient[2], ambient[3] };
		m.diffuse = { diffuse[0], diffuse[1], diffuse[2], diffuse[3] };
		m.specular = { specular[0], specular[1], specular[2], specular[3] };
		m.diffuseMap = xtest::common::ToWString(diffuseMap);
		m.normalMap = xtest::common::ToWString(normalMap);
		m.glossMap = xtest::common::ToWString(glossMap);
	}

} // file
} // xtest




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
	// extension check
	size_t extensionStrPos = filePath.find_last_of(L".");
	XTEST_ASSERT(extensionStrPos != std::string::npos, L"no extension found");
	std::wstring extension = filePath.substr(extensionStrPos + 1, filePath.size() - extensionStrPos);
	XTEST_ASSERT(extension.compare(L"gpf") == 0, L"the specified file must be of type .gpf");


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



	// look for a material file in the same directory,
	// must have the same name but gpm extension
	std::wstring materialPath = filePath.substr(0, extensionStrPos).append(L".gpm");
 	std::ifstream inputFileStream(materialPath);
	
	if(!inputFileStream.fail())
	{
		// deserialize the material
		nlohmann::json json;
		inputFileStream >> json;
		inputFileStream.close();

		size_t gpfFileNameStrPos = filePath.find_last_of(L"\\/");
		std::wstring gpfDirectory = filePath.substr(0, gpfFileNameStrPos +1);

		// search for mesh names
		std::map<std::string, mesh::MeshMaterial> materialMapByShapeName = json.get<std::map<std::string, mesh::MeshMaterial>>();
		for (GPFMesh::DescriptorMap::const_reference namePairWithDescriptor : gpfMesh.meshDescriptorMapByName)
		{
			const std::string& meshName = namePairWithDescriptor.first;
			auto namePairWithMaterialIter = materialMapByShapeName.find(meshName);
			if (namePairWithMaterialIter != materialMapByShapeName.end())
			{
				// store the material data
				mesh::MeshMaterial& gpfMaterial = gpfMesh.meshDescriptorMapByName[meshName].material;
				gpfMaterial = namePairWithMaterialIter->second;

				// expand from relative to absolute path
				gpfMaterial.diffuseMap.insert(0, gpfDirectory);
				gpfMaterial.normalMap.insert(0, gpfDirectory);
				gpfMaterial.glossMap.insert(0, gpfDirectory);
			}
		}
	}

	return gpfMesh;
}



void xtest::file::WriteGPFOnDiskFromObj(const std::wstring& inputFile, const std::wstring& outputFile, bool isLeftHanded)
{
	// extension check
	size_t extensionStrPos = outputFile.find_last_of(L".");
	XTEST_ASSERT(extensionStrPos != std::string::npos, L"no extension found");
	std::wstring extension = outputFile.substr(extensionStrPos + 1, outputFile.size() - extensionStrPos);
	XTEST_ASSERT(extension.compare(L"gpf") == 0, L"the specified output file must be of type .gpf");


	// hash function needed in order to use a MeshData::Vertex as a key of an unordered_map
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
	typedef std::unordered_map<std::string, std::vector<mesh::MeshData::Vertex>> ShapeVerticesMap;


 	

	// load obj form file
	std::string inputFile_asString = common::ToString(inputFile);
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool result = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, inputFile_asString.c_str());
	XTEST_ASSERT(result, L"tiny object loader failed to parse the file: %s", inputFile.c_str());

	if (!result)
	{
		return;
	}



	// extract data from tinyobj structures
	ShapeVerticesMap verticesMapByShapeName;
	for (size_t shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++)
	{
		std::vector<mesh::MeshData::Vertex> shapeVertices;
		size_t tyniobjIndexOffset = 0;

		for (size_t faceIndex = 0; faceIndex < shapes[shapeIndex].mesh.num_face_vertices.size(); faceIndex++) 
		{
			const size_t faceVertexCount = shapes[shapeIndex].mesh.num_face_vertices[faceIndex];
			for (size_t tyniobjVertexIndex = 0; tyniobjVertexIndex < faceVertexCount; tyniobjVertexIndex++) 
			{
				XTEST_ASSERT(faceVertexCount == 3, L"triangles only are supported");

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

				// tangents will be calculated by the next step
				mesh::MeshData::Vertex meshDataVertex = { { vx, vy, vz },{ nx, ny, nz },{ 0.f, 0.f, 0.f, 1.f },{ tx, ty } };


				// convert to left-handed system if necessary
				if (!isLeftHanded)
				{
					meshDataVertex.normal.z *= -1.f;
					meshDataVertex.position.z *= -1.f;
					meshDataVertex.uv.y = 1.f - meshDataVertex.uv.y;
					shapeVertices.push_back(meshDataVertex);

					if ((tyniobjVertexIndex + 1) % 3 == 0) // reverse vertex order
					{
						std::swap(shapeVertices[shapeVertices.size() - 1], shapeVertices[shapeVertices.size() - 2]);
					}
				}
				else
				{
					shapeVertices.push_back(meshDataVertex);
				}
			}
			tyniobjIndexOffset += faceVertexCount;
		}
		verticesMapByShapeName[shapes[shapeIndex].name] = shapeVertices;
	}

	XTEST_ASSERT(verticesMapByShapeName.size() > 0);





	// calculate mikkTSpace tangent bases
	SMikkTSpaceInterface mikkInterface;
	mikkInterface.m_getNumFaces = &MikkGetNumFacesFn;
	mikkInterface.m_getNumVerticesOfFace = &MikkGetNumVerticesOfFaceFn;
	mikkInterface.m_getPosition = &MikkGetPositionFn;
	mikkInterface.m_getNormal = &MikketNormalFn;
	mikkInterface.m_getTexCoord = &MikkGetTexCoordFn;
	mikkInterface.m_setTSpaceBasic = &MikkSetTSpaceBasicFn;
	mikkInterface.m_setTSpace = nullptr;
	for (ShapeVerticesMap::reference namePairWithVertices : verticesMapByShapeName)
	{
		SMikkTSpaceContext mikkContext;
		mikkContext.m_pInterface = &mikkInterface;
		mikkContext.m_pUserData = &namePairWithVertices.second;
		genTangSpaceDefault(&mikkContext);
	}
	
	

	// populate mesh data and gpf mesh headers
	mesh::MeshData meshData;
	std::vector<GPFMeshHeader> gpfMeshHeaders;
	uint32 vertexOffset = 0;
	uint32 indexOffset = 0;
	for (ShapeVerticesMap::const_reference namePairWithVertices : verticesMapByShapeName)
	{
		VertexIndexMap indexMapByVertex(namePairWithVertices.second.size(), HashFn);
		GPFMeshHeader gpfMeshHeader;
		uint32 index = 0;
		uint32 vertexCount = 0;
		uint32 indexCount = 0;

		// eliminate duplicates via indices creation
		for (const MeshData::Vertex& vertex : namePairWithVertices.second)
		{
			VertexIndexMap::iterator VertexPairWithIndexIter = indexMapByVertex.find(vertex);
			if (VertexPairWithIndexIter == indexMapByVertex.end())
			{
				meshData.vertices.push_back(vertex);
				meshData.indices.push_back(index);
				indexMapByVertex[vertex] = index++;
				vertexCount++;
				indexCount++;
			}
			else
			{
				meshData.indices.push_back(VertexPairWithIndexIter->second);
				indexCount++;
			}
		}
		// gpf format has a fixed width for the shape name (24 byte in total, null terminator included)
		const char* shapeName_cstr = namePairWithVertices.first.c_str();
		std::memcpy(&gpfMeshHeader.name, shapeName_cstr, std::min(sizeof(gpfMeshHeader.name) - 1, std::strlen(shapeName_cstr)));

		gpfMeshHeader.vertexCount = vertexCount;
		gpfMeshHeader.indexCount = indexCount;
		gpfMeshHeader.vertexOffset = vertexOffset;
		gpfMeshHeader.indexOffset = indexOffset;

		gpfMeshHeaders.push_back(gpfMeshHeader);
		vertexOffset += vertexCount;
		indexOffset += indexCount;
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
	//
	std::ofstream outputFileStream(outputFile.c_str(), std::ifstream::binary);
	XTEST_ASSERT(!outputFileStream.fail(), L"unable to write file on disk, %s", outputFile.c_str());

	int32 meshCount = int32(gpfMeshHeaders.size());
	outputFileStream.write(reinterpret_cast<char*>(&meshCount), sizeof(int32));

	for (GPFMeshHeader& gpfMeshHeader : gpfMeshHeaders)
	{
		outputFileStream.write(reinterpret_cast<char*>(&gpfMeshHeader), sizeof(GPFMeshHeader));
	}
	outputFileStream.write(reinterpret_cast<char*>(&meshData.vertices[0]), sizeof(mesh::MeshData::Vertex) * meshData.vertices.size());
	outputFileStream.write(reinterpret_cast<char*>(&meshData.indices[0]), sizeof(int32) * meshData.indices.size());
	outputFileStream.close();
}



void xtest::file::WriteGPFMaterial(const std::map<std::string, mesh::MeshMaterial>& materialMapByShapeName, const std::wstring& outputFile)
{
	// extension check
	size_t extensionStrPos = outputFile.find_last_of(L".");
	XTEST_ASSERT(extensionStrPos != std::string::npos, L"no extension found");
	std::wstring extension = outputFile.substr(extensionStrPos + 1, outputFile.size() - extensionStrPos);
	XTEST_ASSERT(extension.compare(L"gpm") == 0, L"the specified file must be of type .gpm");


	nlohmann::json json = materialMapByShapeName;
	std::string jsonSerialized = json.dump();

	std::ofstream outputFileStream(outputFile.c_str());
	XTEST_ASSERT(!outputFileStream.fail(), L"unable to write file on disk, %s", outputFile.c_str());
	outputFileStream << jsonSerialized;
	outputFileStream.close();
}



void xtest::file::WriteEmptyGPFMaterialForMesh(const mesh::GPFMesh& mesh, const std::wstring& outputFile)
{
	std::map<std::string, mesh::MeshMaterial> materialMapByMeshName;
	for (mesh::GPFMesh::DescriptorMap::const_reference namePairWithDescriptor : mesh.meshDescriptorMapByName)
	{
		materialMapByMeshName[namePairWithDescriptor.first] = mesh::MeshMaterial();
	}

	WriteGPFMaterial(materialMapByMeshName, outputFile);
}
