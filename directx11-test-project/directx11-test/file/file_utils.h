#pragma once

#include <future>
#include <file/binary_file.h>
#include <mesh/mesh_format.h>


namespace xtest {
namespace file {



	std::future<BinaryFile> ReadBinaryFile(std::wstring filePath);

	mesh::GPFMesh ReadGPF(const std::wstring& filePath);

	void WriteGPFOnDiskFromObj(const std::wstring& inputFile, const std::wstring& outputFile);
	

} //file
} //xtest

