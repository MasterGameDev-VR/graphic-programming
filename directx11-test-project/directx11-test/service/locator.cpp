#include "stdafx.h"
#include "locator.h"

using namespace xtest;
using xtest::service::Locator;


input::Mouse* Locator::s_mouseService = nullptr;
input::Keyboard* Locator::s_keyboardService = nullptr;


input::Mouse* Locator::GetMouse()
{
	return s_mouseService;
}


input::Keyboard* Locator::GetKeyboard()
{
	return s_keyboardService;
}


void Locator::ProvideService(input::Mouse* mouse)
{
	s_mouseService = mouse;
}


void Locator::ProvideService(input::Keyboard* keyboard)
{
	s_keyboardService = keyboard;
}

