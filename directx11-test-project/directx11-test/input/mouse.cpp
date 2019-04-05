#include "stdafx.h"
#include "mouse.h"
#include <application/windows_app.h>


using xtest::input::Mouse;
using xtest::input::MouseButton;
using xtest::input::ButtonStatus;
using xtest::input::MouseListener;
using xtest::input::CursorMode;


Mouse::Mouse(xtest::application::WindowsApp* ownerApplication)
	: m_buttonStatus()
	, m_listeners()
	, m_mode(CursorMode::free_mode)
	, m_cursorPos(0,0)
	, m_lastMovement(0,0)
	, m_isCursorVisible(true)
	, m_ownerApplication(ownerApplication)
{}


const ButtonStatus& Mouse::GetButtonStatus(MouseButton button) const
{
	return m_buttonStatus[static_cast<size_t>(button)];
}


void Mouse::AddListener(MouseListener* listener)
{
	XTEST_ASSERT(listener);
	
	if (std::find(m_listeners.begin(), m_listeners.end(), listener) == m_listeners.end())
	{
		m_listeners.push_back(listener);
	}
}


void Mouse::RemoveListener(MouseListener* listener)
{
	XTEST_ASSERT(listener);
	std::vector<MouseListener*>::iterator listenerIter = std::find(m_listeners.begin(), m_listeners.end(), listener);
	if (listenerIter != m_listeners.end())
	{
		m_listeners.erase(listenerIter);
	}
}


void Mouse::ClearDownButtons()
{
	for (int statusIndex = 0; statusIndex < m_buttonStatus.size(); statusIndex++)
	{
		ButtonStatus& buttonStatus = m_buttonStatus[statusIndex];

		if (buttonStatus.isDown)
		{
			buttonStatus.isDown = false;
			buttonStatus.startingTime = time::TimePoint::Now();

			MouseButton button = static_cast<MouseButton>(statusIndex);
			std::for_each(m_listeners.begin(), m_listeners.end(), [button, &buttonStatus](MouseListener* listener) { listener->OnButtonStatusChange(button, buttonStatus); });
		}
	}
}


void Mouse::SetCursorMode(CursorMode mode)
{
	m_mode = mode;

	if (m_mode == CursorMode::clip_mode)
	{
		HWND mainWindow = m_ownerApplication->GetMainWindow();

		RECT clientArea;
		GetClientRect(mainWindow, &clientArea);

		// convert the client area to screen coordinates
		POINT absoluteTopLeft = { clientArea.left, clientArea.top };
		POINT absoluteBottomRight = { clientArea.right, clientArea.bottom };
		ClientToScreen(mainWindow, &absoluteTopLeft);
		ClientToScreen(mainWindow, &absoluteBottomRight);
		SetRect(&clientArea, absoluteTopLeft.x, absoluteTopLeft.y, absoluteBottomRight.x, absoluteBottomRight.y);

		// confine the cursor to the client area
		ClipCursor(&clientArea);
	}
	else
	{
		ClipCursor(nullptr);
	}

}


CursorMode Mouse::GetCursorMode() const
{
	return m_mode;
}


void Mouse::SetCursorPosition(const DirectX::XMINT2& position)
{
	SetCursorPos(position.x, position.y);

	// update the movement
	DirectX::XMVECTOR targetPos = DirectX::XMLoadSInt2(&position);
	DirectX::XMVECTOR currentPos = DirectX::XMLoadSInt2(&m_cursorPos);
	DirectX::XMStoreSInt2(&m_lastMovement, DirectX::XMVectorSubtract(targetPos, currentPos));

	//update the internal position
	m_cursorPos = position;

	std::for_each(m_listeners.begin(), m_listeners.end(), [this](MouseListener* listener) { listener->OnMouseMove(m_lastMovement, m_cursorPos); });
}


const DirectX::XMINT2& Mouse::GetCursorPosition()
{
	return m_cursorPos;
}

bool Mouse::IsInClientArea() const
{
	HWND mainWindow = m_ownerApplication->GetMainWindow();

	// translate the current position in client area coordinates
	POINT mousePos;
	mousePos.x = m_cursorPos.x;
	mousePos.y = m_cursorPos.y;
	ScreenToClient(mainWindow, &mousePos);

	RECT clientArea;
	GetClientRect(mainWindow, &clientArea);

	return mousePos.x > clientArea.left && mousePos.x < clientArea.right && mousePos.y > clientArea.top && mousePos.y < clientArea.bottom;
}


void Mouse::SetCursorVisible(bool visible)
{
	m_isCursorVisible = visible;
	ShowCursor(visible);
}


bool Mouse::IsCursorVisible() const
{
	return m_isCursorVisible;
}


void Mouse::OnWmMouseRawInput(const RAWINPUTHEADER& header, const RAWMOUSE& data)
{
	XTEST_UNUSED_VAR(header);

	// mouse movement
	if (data.lLastX != 0 || data.lLastY != 0)
	{
		XTEST_ASSERT(!(data.usFlags & MOUSE_VIRTUAL_DESKTOP), L"MOUSE_VIRTUAL_DESKTOP mode not yet supported");
		XTEST_ASSERT(!(data.usFlags & MOUSE_MOVE_ABSOLUTE), L"MOUSE_MOVE_ABSOLUTE mode not yet supported");

		POINT cursorPosition;
		GetCursorPos(&cursorPosition);
		m_cursorPos = { cursorPosition.x, cursorPosition.y };
		m_lastMovement = { data.lLastX, data.lLastY };

		std::for_each(m_listeners.begin(), m_listeners.end(), [this](MouseListener* listener) { listener->OnMouseMove(m_lastMovement, m_cursorPos); });
	}


	// mouse buttons
	if (data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
	{
		UpdateButtonStatus(MouseButton::left_button, false);
	}
	if (data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
	{
		UpdateButtonStatus(MouseButton::left_button, true);
	}
	if (data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
	{
		UpdateButtonStatus(MouseButton::middle_button, false);
	}
	if (data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
	{
		UpdateButtonStatus(MouseButton::middle_button, true);
	}
	if (data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
	{
		UpdateButtonStatus(MouseButton::right_button, false);
	}
	if (data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
	{
		UpdateButtonStatus(MouseButton::right_button, true);
	}
	if (data.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
	{
		UpdateButtonStatus(MouseButton::x_button_1, false);
	}
	if (data.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
	{
		UpdateButtonStatus(MouseButton::x_button_1, true);
	}
	if (data.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
	{
		UpdateButtonStatus(MouseButton::x_button_2, false);
	}
	if (data.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
	{
		UpdateButtonStatus(MouseButton::x_button_2, true);
	}

	// mouse wheel
	if (data.usButtonFlags & RI_MOUSE_WHEEL)
	{
		short signedScrollDelta = (short)(data.usButtonData);

		ScrollStatus status;
		status.isScrollingUp = signedScrollDelta > 0;
		status.deltaScroll = std::abs(signedScrollDelta);
		status.startingTime = time::TimePoint::Now();

		std::for_each(m_listeners.begin(), m_listeners.end(), [status](MouseListener* listener) { listener->OnWheelScroll(status); });
	}
}


void Mouse::UpdateButtonStatus(MouseButton button, bool isDown)
{
	ButtonStatus& status = m_buttonStatus[static_cast<size_t>(button)];
	status.isDown = isDown;
	status.startingTime = time::TimePoint::Now();

	std::for_each(m_listeners.begin(), m_listeners.end(), [button, &status](MouseListener* listener) { listener->OnButtonStatusChange(button, status); });
}


