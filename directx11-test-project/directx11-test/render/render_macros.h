#pragma once

#include <comdef.h>
#include <string>

/**
Enables or disable rendering API debugging macros.
Define this macro with value 1 if you want to enable the API debugging macros, 0 otherwise.
If not defined, this macro has value 1 in Debug builds and 0 otherwise.
*/
#ifndef XTEST_ENABLE_API_DEBUG
#	ifndef NDEBUG
#		define XTEST_ENABLE_API_DEBUG 1 // turn on rendering API debugging
#	else
#		define XTEST_ENABLE_API_DEBUG 0 // turn off rendering API debugging
#	endif
#endif // ifndef XTEST_ENABLE_API_DEBUG


/**
 Use this macro to test if a DirectX command was run successfully, if it fails a pop-up will
 be shown describing the error message.
 */
#if XTEST_ENABLE_API_DEBUG
#	ifndef XTEST_D3D_CHECK
#		define XTEST_D3D_CHECK(x) \
			do { \
				HRESULT _hr = (x); \
				if(FAILED(_hr)) \
				{ \
					_com_error error(_hr);	\
											\
					std::wstring fileName;	\
					fileName.resize(strlen(__FILE__) +1); \
					size_t returnSize; \
					mbstowcs_s(&returnSize, &fileName[0], fileName.size(), __FILE__ , fileName.size()-1); \
					fileName.resize(fileName.size() -1);	/*drop the null end character*/ \
																							\
					std::wstring message = L"- File: " + fileName + L"[" + std::to_wstring(__LINE__) + L"]\r" + \
										   L"- Function: " + L#x + L" \r" + \
										   L"- Error: " + error.ErrorMessage(); \
					XTEST_ASSERT(false, message.c_str()); /*MessageBoxW(nullptr, message.c_str(), L"D3D Error", MB_OK | MB_ICONERROR);*/ \
				} \
			} while(false)
#	endif
#else
#	ifndef XTEST_D3D_CHECK
#		define XTEST_D3D_CHECK(x) (x)
#	endif
#endif // XTEST_ENABLE_API_DEBUG

