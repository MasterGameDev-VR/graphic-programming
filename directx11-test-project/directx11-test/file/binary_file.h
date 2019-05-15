#pragma once

#include <future>

namespace xtest {
namespace file {

	class BinaryFile
	{
		// the only way to create a non-empty BinaryFile
		friend std::future<BinaryFile> ReadBinaryFile(std::wstring filePath);
		friend class ResourceLoader;

	public:

		BinaryFile();
		BinaryFile(BinaryFile&& other);
		BinaryFile& BinaryFile::operator=(BinaryFile&& other);

		virtual ~BinaryFile();


		BinaryFile(const BinaryFile&) = delete;
		BinaryFile& operator=(const BinaryFile&) = delete;


		const char* Data() const;
		uint64 ByteSize() const;

	private:

		BinaryFile(std::unique_ptr<char[]> byteArray, uint64 size);

		std::unique_ptr<char[]> m_byteArray;
		uint64 m_byteSize;

	};

} // file
} // xtest

