#pragma once

#include <application/window_settings.h>
#include <input/mouse.h>
#include <input/keyboard.h>

namespace xtest {
namespace application {


	class WindowsApp
	{
	public:

		WindowsApp(HINSTANCE instance, const WindowSettings& windowSettings);
		virtual ~WindowsApp() {};

		WindowsApp(WindowsApp&&) = delete;
		WindowsApp(const WindowsApp&) = delete;
		WindowsApp& operator=(WindowsApp&&) = delete;
		WindowsApp& operator=(const WindowsApp&) = delete;


		virtual void Init();
		void Show();

		const WindowSettings& GetWindowSettings() const;
		unsigned GetCurrentWidth() const;
		unsigned GetCurrentHeight() const;
		float AspectRatio() const;

		bool IsFullscreen() const;
		virtual void SwitchToFullscreen();
		virtual void SwitchToWindowed();

		HINSTANCE GetInstance() const;
		HWND GetMainWindow() const;
		std::wstring GetRootDir() const;

	protected:

		virtual void OnMinimized() {};
		virtual void OnResized() {};
		virtual void OnActive() {};
		virtual void OnInactive() {};
		virtual void OnShow() {};

		// HandleWindowMessage must never contain a call to DefWindowProc, instead should return handlerResult.isHandle = false
		virtual LRESULT HandleWindowMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	private:

		static LRESULT CALLBACK s_MsgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		LRESULT OnWmSizeCallback(WPARAM wParam);
		LRESULT OnWmEnterSizeMoveCallback();
		LRESULT OnWmExitSizeMoveCallback();
		LRESULT OnWmActivateCallback(WPARAM wParam);
		LRESULT OnWmInput(LPARAM lParam);
		LRESULT OnWmAltEnterCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


		static const DWORD s_WindowedStyle;
		static const DWORD s_FullscreenStyle;

		HINSTANCE m_instance;
		HWND m_mainWindow;
		RECT m_preFullscrenRect;
		WindowSettings m_windowSettings;
		bool m_isWindowRectChanging;
		bool m_hasBeenShown;
		bool m_isFullscreen;
		std::wstring m_rootDir;
		input::Mouse m_mouse;
		input::Keyboard m_keyboard;
	};


} // application
} // xtest

