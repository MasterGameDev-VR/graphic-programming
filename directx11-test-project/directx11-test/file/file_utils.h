#pragma once

#include <future>
#include <file/binary_file.h>
#include <mesh/mesh_format.h>


namespace xtest {
namespace file {
	
	/************************************************************************/
	/* Use these methods only to generate a new GPF outside the demo files. */
	/* Always prefer ResourceLoader methods in your demos to load any file. */
	/************************************************************************/


	std::future<BinaryFile> ReadBinaryFile(std::wstring filePath);

	mesh::GPFMesh ReadGPF(const std::wstring& filePath);

	// note: if isLeftHanded is false that means the source data is right-handed and needs to be transform to our left-handed system
	void WriteGPFOnDiskFromObj(const std::wstring& inputFile, const std::wstring& outputFile, bool isLeftHanded);

	void WriteGPFMaterial(const std::map<std::string, mesh::MeshMaterial>& materialMapByShapeName, const std::wstring& outputFile);

	void WriteEmptyGPFMaterialForMesh(const mesh::GPFMesh& mesh, const std::wstring& outputFile);

} //file
} //xtest

