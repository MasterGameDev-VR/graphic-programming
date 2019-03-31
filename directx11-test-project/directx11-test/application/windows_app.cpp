#include "stdafx.h"
#include "windows_app.h"
#include <application/resources/Resource.h>
#include <input/keyboard.h>
#include <service/locator.h>


using xtest::application::WindowsApp;
using xtest::application::WindowSettings;
using xtest::input::Keyboard;
using xtest::input::Mouse;


const DWORD WindowsApp::s_WindowedStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
const DWORD WindowsApp::s_FullscreenStyle = WS_VISIBLE | WS_POPUP; // we use  borderless fullscreen



WindowsApp::WindowsApp(HINSTANCE instance, const WindowSettings& windowSettings)
	: m_instance(instance)
	, m_mainWindow(nullptr)
	, m_preFullscrenRect()
	, m_windowSettings(windowSettings)
	, m_isWindowRectChanging(false)
	, m_hasBeenShown(false)
	, m_isFullscreen(m_windowSettings.fullScreen)
	, m_mouse(this)
	, m_keyboard()
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);
	GetModuleFileNameW(nullptr, buffer.get(), MAX_PATH);
	
	std::wstring completePath(buffer.get());
	m_rootDir = completePath.erase(completePath.find_last_of(L"\\/"));
}


void WindowsApp::Init()
{
	WNDCLASSEXW windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = &WindowsApp::s_MsgProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = m_instance;
	windowClass.hIcon = LoadIcon(m_instance, MAKEINTRESOURCE(IDI_DIRECTX11TEST));
	windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	windowClass.lpszMenuName = m_windowSettings.title;
	windowClass.lpszClassName = m_windowSettings.title;
	windowClass.hIconSm = LoadIcon(windowClass.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&windowClass);


	DWORD windowStyle;
	int windowPosX;
	int windowPosY;
	unsigned windowWidth;
	unsigned windowHeight;

	// calculate the window width and height base on the desired client area
	RECT clientArea = { 0, 0, LONG(m_windowSettings.width), LONG(m_windowSettings.height) };
	AdjustWindowRect(&clientArea, s_WindowedStyle, false);

	if (m_windowSettings.fullScreen)
	{
		windowStyle = s_FullscreenStyle;
		windowPosX = 0;
		windowPosY = 0;

		// ignore the user defined width and height
		windowWidth = GetSystemMetrics(SM_CXSCREEN);
		windowHeight = GetSystemMetrics(SM_CYSCREEN);
		m_preFullscrenRect.right = clientArea.right - clientArea.left;
		m_preFullscrenRect.bottom = clientArea.bottom - clientArea.top;
	}
	else
	{
		windowStyle = s_WindowedStyle;
		windowPosX = CW_USEDEFAULT;
		windowPosY = CW_USEDEFAULT;

		windowWidth = clientArea.right - clientArea.left;
		windowHeight = clientArea.bottom - clientArea.top;
	}

	m_mainWindow = CreateWindowW(
		m_windowSettings.title,
		m_windowSettings.title,
		windowStyle,
		windowPosX,
		windowPosY,
		windowWidth,
		windowHeight,
		nullptr,
		nullptr,
		m_instance,
		this); // we pass this instance ptr so we can retrieve it inside s_MsgProc


	// enable this window to receive raw input messages
	RAWINPUTDEVICE rawInputDevices[2];
	rawInputDevices[0].usUsagePage = 0x01;
	rawInputDevices[0].usUsage = 0x06; // keyboard
	rawInputDevices[0].dwFlags = 0;
	rawInputDevices[0].hwndTarget = 0;

	rawInputDevices[1].usUsagePage = 0x01;
	rawInputDevices[1].usUsage = 0x02; // mouse
	rawInputDevices[1].dwFlags = 0;
	rawInputDevices[1].hwndTarget = 0;

	XTEST_VERIFY(RegisterRawInputDevices(&rawInputDevices[0], sizeof(rawInputDevices)/sizeof(RAWINPUTDEVICE), sizeof(RAWINPUTDEVICE)) != FALSE);

	service::Locator::ProvideService(&m_mouse);
	service::Locator::ProvideService(&m_keyboard);
}

void WindowsApp::Show()
{
	ShowWindow(m_mainWindow, SW_SHOW);
	UpdateWindow(m_mainWindow);
	m_hasBeenShown = true;
}


const WindowSettings& WindowsApp::GetWindowSettings() const
{
	return m_windowSettings;
}


unsigned WindowsApp::GetCurrentWidth() const
{
	RECT clientArea;
	GetClientRect(m_mainWindow, &clientArea);
	return clientArea.right;
}


unsigned WindowsApp::GetCurrentHeight() const
{
	RECT clientArea;
	GetClientRect(m_mainWindow, &clientArea);
	return clientArea.bottom;
}


float WindowsApp::AspectRatio() const
{
	return float(GetCurrentWidth()) / float(GetCurrentHeight());
}


bool WindowsApp::IsFullscreen() const
{
	return m_isFullscreen;
}


void WindowsApp::SwitchToFullscreen()
{
	//let's store the previous window dimensions/position
	GetWindowRect(m_mainWindow, &m_preFullscrenRect);

	m_isFullscreen = true;
	SetWindowLongPtr(m_mainWindow, GWL_STYLE, s_FullscreenStyle);
	SetWindowPos(
		m_mainWindow,
		HWND_TOP,
		0, 
		0, 
		GetSystemMetrics(SM_CXSCREEN), 
		GetSystemMetrics(SM_CYSCREEN), 
		SWP_FRAMECHANGED
	); //this will trigger a resize
}


void WindowsApp::SwitchToWindowed()
{
	m_isFullscreen = false;
	SetWindowLongPtr(m_mainWindow, GWL_STYLE, s_WindowedStyle);
	SetWindowPos(
		m_mainWindow,
		nullptr,
		m_preFullscrenRect.left,
		m_preFullscrenRect.top,
		m_preFullscrenRect.right - m_preFullscrenRect.left,
		m_preFullscrenRect.bottom - m_preFullscrenRect.top,
		SWP_FRAMECHANGED
	); //this will trigger a resize
}


HINSTANCE WindowsApp::GetInstance() const
{
	return m_instance;
}


HWND WindowsApp::GetMainWindow() const
{
	return m_mainWindow;
}

std::wstring WindowsApp::GetRootDir() const
{
	return m_rootDir;
}


LRESULT WindowsApp::OnWmSizeCallback(WPARAM wParam)
{
	if (wParam == SIZE_MINIMIZED)
	{
		OnMinimized();
		m_mouse.ClearDownButtons();
	}
	else if (wParam == SIZE_MAXIMIZED)
	{
		OnResized();
	}
	else if (!m_isWindowRectChanging && m_hasBeenShown)
	{
		OnResized();
	}
	return 0;
}


LRESULT WindowsApp::OnWmEnterSizeMoveCallback()
{
	m_isWindowRectChanging = true;
	return 0;
}


LRESULT WindowsApp::OnWmExitSizeMoveCallback()
{
	RECT rect;
	GetClientRect(m_mainWindow, &rect);
	m_isWindowRectChanging = false;

	// client area always has (top,left) = (0,0)
	if (m_windowSettings.width != uint32(rect.right) || m_windowSettings.height != uint32(rect.bottom))
	{
		OnResized();
	}

	return 0;
}


LRESULT WindowsApp::OnWmActivateCallback(WPARAM wParam)
{
	if (LOWORD(wParam) == WA_INACTIVE)
	{
		OnInactive();
	}
	else
	{
		OnActive();
	}

	return 0;
}

LRESULT WindowsApp::OnWmInput(LPARAM lParam)
{
	UINT byteSize;
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &byteSize, sizeof(RAWINPUTHEADER));
	XTEST_ASSERT(byteSize <= sizeof(RAWINPUT));

	RAWINPUT rawInput;
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &rawInput, &byteSize, sizeof(RAWINPUTHEADER));

	if (rawInput.header.dwType == RIM_TYPEKEYBOARD)
	{
		m_keyboard.OnWmKeyboardRawInput(rawInput.header, rawInput.data.keyboard);
	}
	else if (rawInput.header.dwType == RIM_TYPEMOUSE)
	{
		m_mouse.OnWmMouseRawInput(rawInput.header, rawInput.data.mouse);
	}

	return 0;
}


LRESULT WindowsApp::OnWmAltEnterCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN))
	{
		if (m_isFullscreen)
		{
			SwitchToWindowed();
		}
		else
		{
			SwitchToFullscreen();
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


LRESULT WindowsApp::HandleWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	XTEST_UNUSED_VAR(hwnd);

	switch (message)
	{

	// window events
	case WM_SIZE:
		return OnWmSizeCallback(wParam);

	case WM_ENTERSIZEMOVE:
		return OnWmEnterSizeMoveCallback();

	case WM_EXITSIZEMOVE:
		return OnWmExitSizeMoveCallback();

	case WM_ACTIVATE:
		return OnWmActivateCallback(wParam);

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_INPUT:
		return OnWmInput(lParam);

	case WM_SYSKEYDOWN:
		return OnWmAltEnterCallback(hwnd, message, wParam, lParam);

	case WM_CHAR:
		m_keyboard.OnWmChar(wParam, lParam);
		return 0;

	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE); // don't beep when alt+enter is pressed

	case WM_KILLFOCUS:
		m_keyboard.ClearDownKeys();
		return 0;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}


// WNDCLASSEXW only accept free functions or static functions
LRESULT WindowsApp::s_MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WindowsApp* windowsApp;
	if (message == WM_NCCREATE)
	{
		// the instance ptr (this) was passed by InitMainWindow to CreateWindowW function as lParam
		LPCREATESTRUCT createStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		windowsApp = reinterpret_cast<WindowsApp*>(createStruct->lpCreateParams);

		// store the instance ptr inside the window
		XTEST_ASSERT(windowsApp, L"have you forgot to pass 'this' ptr as the last parameter in CreateWindow function call?");
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(windowsApp));
	}
	else
	{
		// retrieve the instance ptr from the window so we can call our member function to handle any message
		windowsApp = reinterpret_cast<WindowsApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (windowsApp)
	{
		return windowsApp->HandleWindowMessage(hWnd, message, wParam, lParam);
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
