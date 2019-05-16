#pragma once

#include <input/mouse.h>
#include <input/keyboard.h>
#include <file/resource_loader.h>

namespace xtest{
namespace service {

	// class used to share functionalities across classes
	class Locator
	{
	public:

		static input::Mouse* GetMouse();
		static input::Keyboard* GetKeyboard();
		static file::ResourceLoader* GetResourceLoader();
		static ID3D11Device* GetD3DDevice();
		static ID3D11DeviceContext* GetD3DContext();


		static void ProvideService(input::Mouse* mouse);
		static void ProvideService(input::Keyboard* keyboard);
		static void ProvideResourceLoader(file::ResourceLoader* loader);
		static void ProvideD3DDevice(ID3D11Device* d3dDevice);
		static void ProvideD3DContext(ID3D11DeviceContext* d3dContext);

	private:

		static input::Mouse* s_mouseService;
		static input::Keyboard* s_keyboardService;
		static file::ResourceLoader* s_resourceLoader;
		static ID3D11Device* s_d3dDevice;
		static ID3D11DeviceContext* s_d3dContext;

	};

} //service
} //xtest
