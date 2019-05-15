#pragma once

#include <locale>
#include <codecvt>


namespace xtest {
namespace common {

	inline std::string ToString(const std::wstring& wstr)
	{
		//note: multibyte characters on windows are using utf16
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.to_bytes(wstr);
	}

	inline std::wstring ToWString(const std::string& str)
	{
		//note: multibyte characters on windows are using utf16
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		return converter.from_bytes(str);
	}

} // common
} // xtest

