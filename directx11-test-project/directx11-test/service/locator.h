#pragma once

#include <input/mouse.h>
#include <input/keyboard.h>

namespace xtest{
namespace service {

	class Locator
	{
	public:

		static input::Mouse* GetMouse();
		static input::Keyboard* GetKeyboard();

		static void ProvideService(input::Mouse* mouse);
		static void ProvideService(input::Keyboard* keyboard);

	private:

		static input::Mouse* s_mouseService;
		static input::Keyboard* s_keyboardService;

	};

} //service
} //xtest
