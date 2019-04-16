#pragma once

#include <time/time_point.h>


namespace xtest {

	namespace application {	class WindowsApp; }

namespace input {

	enum class MouseButton
	{
		left_button,
		right_button,
		middle_button,
		x_button_1,
		x_button_2,
		COUNT,
	};


	enum class CursorMode
	{
		free_mode,
		clip_mode,
		COUNT
	};


	struct ButtonStatus
	{
		time::TimePoint startingTime = time::TimePoint();
		bool isDown = false;
	};


	struct ScrollStatus
	{
		time::TimePoint startingTime = time::TimePoint();
		uint32 deltaScroll = 0;
		bool isScrollingUp = false;
	};


	class MouseListener
	{
	public:

		virtual void OnButtonStatusChange(MouseButton button, const ButtonStatus& status) { XTEST_UNUSED_VAR(button); XTEST_UNUSED_VAR(status); }
		virtual void OnWheelScroll(ScrollStatus scroll) { XTEST_UNUSED_VAR(scroll); }
		virtual void OnMouseMove(const DirectX::XMINT2& movement, const DirectX::XMINT2& currentPos) { XTEST_UNUSED_VAR(movement); XTEST_UNUSED_VAR(currentPos); }
	};


	class Mouse
	{
		friend class application::WindowsApp;
		
	public:

		Mouse(application::WindowsApp* ownerApplication);
		~Mouse() {}

		Mouse(Mouse&&) = delete;
		Mouse(const Mouse&) = delete;
		Mouse& operator=(Mouse&&) = delete;
		Mouse& operator=(const Mouse&) = delete;


		const ButtonStatus& GetButtonStatus(MouseButton button) const;

		void AddListener(MouseListener* listener);
		void RemoveListener(MouseListener* listener);
		void ClearDownButtons();

		void SetCursorMode(CursorMode mode);
		CursorMode GetCursorMode() const;

		void SetCursorPosition(const DirectX::XMINT2& position);
		const DirectX::XMINT2& GetCursorPosition();
		bool IsInClientArea() const;

		void SetCursorVisible(bool visible);
		bool IsCursorVisible() const;


	private:

		void OnWmMouseRawInput(const RAWINPUTHEADER& header, const RAWMOUSE& data);
		void UpdateButtonStatus(MouseButton button, bool isDown);

		std::array<ButtonStatus, static_cast<size_t>(MouseButton::COUNT)> m_buttonStatus;
		std::vector<MouseListener*> m_listeners;
		CursorMode m_mode;
		DirectX::XMINT2 m_cursorPos;
		DirectX::XMINT2 m_lastMovement;
		bool m_isCursorVisible;
		application::WindowsApp* m_ownerApplication;
	};

} // input
} // mouse

