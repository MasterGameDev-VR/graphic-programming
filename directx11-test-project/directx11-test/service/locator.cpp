#include "stdafx.h"
#include "locator.h"

using namespace xtest;
using xtest::service::Locator;


input::Mouse* Locator::s_mouseService = nullptr;
input::Keyboard* Locator::s_keyboardService = nullptr;
file::ResourceLoader* Locator::s_resourceLoader = nullptr;
ID3D11Device* Locator::s_d3dDevice = nullptr;
ID3D11DeviceContext* Locator::s_d3dContext = nullptr;


input::Mouse* Locator::GetMouse()
{
	return s_mouseService;
}

input::Keyboard* Locator::GetKeyboard()
{
	return s_keyboardService;
}

file::ResourceLoader* Locator::GetResourceLoader()
{
	return s_resourceLoader;
}

ID3D11Device* Locator::GetD3DDevice()
{
	return s_d3dDevice;
}

ID3D11DeviceContext* Locator::GetD3DContext()
{
	return s_d3dContext;
}


void Locator::ProvideService(input::Mouse* mouse)
{
	s_mouseService = mouse;
}

void Locator::ProvideService(input::Keyboard* keyboard)
{
	s_keyboardService = keyboard;
}

void Locator::ProvideResourceLoader(file::ResourceLoader* loader)
{
	s_resourceLoader = loader;
}

void Locator::ProvideD3DDevice(ID3D11Device* d3dDevice)
{
	s_d3dDevice = d3dDevice;
}

void Locator::ProvideD3DContext(ID3D11DeviceContext* d3dContext)
{
	s_d3dContext = d3dContext;
}

