#include "stdafx.h"
#include "keyboard.h"
#include <algorithm>
#include <application/windows_app.h>


using xtest::input::Keyboard;
using xtest::input::KeyboardListener;
using xtest::input::KeyStatus;
using xtest::input::Key;


std::unordered_map<uint32, Key> Keyboard::s_windowsVKToKey =
{
	{ 0x30, Key::zero },
	{ 0x31, Key::one },
	{ 0x32, Key::two },
	{ 0x33, Key::three },
	{ 0x34, Key::four },
	{ 0x35, Key::five },
	{ 0x36, Key::six },
	{ 0x37, Key::seven },
	{ 0x38, Key::eight },
	{ 0x39, Key::nine },
	{ 0x41, Key::A },
	{ 0x42, Key::B },
	{ 0x43, Key::C },
	{ 0x44, Key::D },
	{ 0x45, Key::E },
	{ 0x46, Key::F },
	{ 0x47, Key::G },
	{ 0x48, Key::H },
	{ 0x49, Key::I },
	{ 0x4A, Key::J },
	{ 0x4B, Key::K },
	{ 0x4C, Key::L },
	{ 0x4D, Key::M },
	{ 0x4E, Key::N },
	{ 0x4F, Key::O },
	{ 0x50, Key::P },
	{ 0x51, Key::Q },
	{ 0x52, Key::R },
	{ 0x53, Key::S },
	{ 0x54, Key::T },
	{ 0x55, Key::U },
	{ 0x56, Key::V },
	{ 0x57, Key::W },
	{ 0x58, Key::X },
	{ 0x59, Key::Y },
	{ 0x5A, Key::Z },
	{ VK_BACK, Key::back },
	{ VK_TAB, Key::tab },
	{ VK_CLEAR, Key::clear },
	{ VK_RETURN, Key::enter },
	{ VK_SHIFT, Key::shift },
	{ VK_LSHIFT, Key::left_shift },
	{ VK_RSHIFT, Key::right_shift },
	{ VK_CONTROL, Key::ctrl },
	{ VK_LCONTROL, Key::left_ctrl },
	{ VK_RCONTROL, Key::right_ctrl },
	{ VK_MENU, Key::alt },
	{ VK_LMENU, Key::left_alt },
	{ VK_RMENU, Key::right_alt },
	{ VK_PAUSE, Key::pause },
	{ VK_CAPITAL, Key::caps },
	{ VK_ESCAPE, Key::esc },
	{ VK_SPACE, Key::space_bar },
	{ VK_PRIOR, Key::page_up },
	{ VK_NEXT, Key::page_down },
	{ VK_END, Key::end },
	{ VK_HOME, Key::home },
	{ VK_LEFT, Key::left_arrow },
	{ VK_UP, Key::up_arrow },
	{ VK_RIGHT, Key::right_arrow },
	{ VK_DOWN, Key::down_arrow },
	{ VK_SNAPSHOT, Key::print },
	{ VK_INSERT, Key::ins },
	{ VK_DELETE, Key::del },
	{ VK_NUMPAD0, Key::numpad_zero },
	{ VK_NUMPAD1, Key::numpad_one },
	{ VK_NUMPAD2, Key::numpad_two },
	{ VK_NUMPAD3, Key::numpad_three },
	{ VK_NUMPAD4, Key::numpad_four },
	{ VK_NUMPAD5, Key::numpad_five },
	{ VK_NUMPAD6, Key::numpad_six },
	{ VK_NUMPAD7, Key::numpad_seven },
	{ VK_NUMPAD8, Key::numpad_eight },
	{ VK_NUMPAD9, Key::numpad_nine },
	{ VK_MULTIPLY, Key::asterisk },
	{ VK_ADD, Key::plus },
	{ VK_SEPARATOR, Key::pipe },
	{ VK_SUBTRACT, Key::dash },
	{ VK_DECIMAL, Key::dot },
	{ VK_DIVIDE, Key::forward_slash },
	{ VK_F1, Key::F1 },
	{ VK_F2, Key::F2 },
	{ VK_F3, Key::F3 },
	{ VK_F4, Key::F4 },
	{ VK_F5, Key::F5 },
	{ VK_F6, Key::F6 },
	{ VK_F7, Key::F7 },
	{ VK_F8, Key::F8 },
	{ VK_F9, Key::F9 },
	{ VK_F10, Key::F10 },
	{ VK_F11, Key::F11 },
	{ VK_F12, Key::F12 },
	{ VK_NUMLOCK, Key::num_lock },
	{ VK_SCROLL, Key::scroll },
	{ VK_OEM_1, Key::oem1 },
	{ VK_OEM_2, Key::oem2 },
	{ VK_OEM_3, Key::oem3 },
	{ VK_OEM_4, Key::oem4 },
	{ VK_OEM_5, Key::oem5 },
	{ VK_OEM_6, Key::oem6 },
	{ VK_OEM_7, Key::oem7 },
	{ VK_OEM_PLUS, Key::oem_plus },
	{ VK_OEM_MINUS, Key::oem_dash },
	{ VK_OEM_PERIOD, Key::oem_dot },
	{ VK_OEM_COMMA, Key::oem_comma },
	{ VK_OEM_102, Key::angle_bracket }
};



Keyboard::Keyboard()
	: m_keyStatus()
	, m_listeners()
	, m_isInTextMode(false)
{}


const KeyStatus& Keyboard::GetKeyStatus(Key key) const
{
	return m_keyStatus[static_cast<size_t>(key)];
}


void Keyboard::AddListener(KeyboardListener* listener, std::initializer_list<Key> keys /*= {Key::ALL_KEYS}*/)
{
	XTEST_ASSERT(listener);

	if (std::find(keys.begin(), keys.end(), Key::ALL_KEYS) != keys.end())
	{
		for (std::vector<KeyboardListener*>& keyListeners : m_listeners)
		{
			keyListeners.push_back(listener);
		}
	}
	else
	{
		for (Key key : keys)
		{
			m_listeners[static_cast<size_t>(key)].push_back(listener);
		}
	}
}


void Keyboard::RemoveListener(KeyboardListener* listener)
{
	XTEST_ASSERT(listener);

	for (std::vector<KeyboardListener*>& keyListeners : m_listeners)
	{
		std::vector<KeyboardListener*>::iterator listenerIter = std::find(keyListeners.begin(), keyListeners.end(), listener);
		if (listenerIter != keyListeners.end())
		{
			keyListeners.erase(listenerIter);
		}
	}
}


void Keyboard::ClearDownKeys()
{
	for (int statusIndex = 0; statusIndex < m_keyStatus.size(); statusIndex++)
	{
		KeyStatus& keyStatus = m_keyStatus[statusIndex];

		if (keyStatus.isDown)
		{
			keyStatus.isDown = false;
			keyStatus.startingTime = time::TimePoint::Now();

			Key key = static_cast<Key>(statusIndex);
			size_t key_asIndex = static_cast<size_t>(key);
			std::for_each(m_listeners[key_asIndex].begin(), m_listeners[key_asIndex].end(), [key, &keyStatus](KeyboardListener* listener) { listener->OnKeyStatusChange(key, keyStatus); });
		}
	}
}


void Keyboard::EnableTextMode()
{
	m_isInTextMode = true;
}


void Keyboard::DisableTextMode()
{
	m_isInTextMode = false;
}

bool Keyboard::IsInTextMode() const
{
	return m_isInTextMode;
}


void Keyboard::OnWmKeyboardRawInput(const RAWINPUTHEADER& header, const RAWKEYBOARD& data)
{
	XTEST_UNUSED_VAR(header);

	auto vKPairWithKey = s_windowsVKToKey.find(data.VKey);
	if (vKPairWithKey != s_windowsVKToKey.end())
	{
		// let's check if is not up
		bool isDown = !(data.Flags & RI_KEY_BREAK); 
		Key key = vKPairWithKey->second;
		size_t key_asIndex = static_cast<size_t>(key);
		KeyStatus& keyStatus = m_keyStatus[key_asIndex];

		// we discard key repeat
		if (keyStatus.isDown != isDown)
		{
			keyStatus.isDown = isDown;
			keyStatus.startingTime = time::TimePoint::Now();

			std::for_each(m_listeners[key_asIndex].begin(), 
				m_listeners[key_asIndex].end(), 
				[key, &keyStatus](KeyboardListener* listener) { listener->OnKeyStatusChange(key, keyStatus); });
		}
	}
}


void Keyboard::OnWmChar(WPARAM wParam, LPARAM lParam)
{
	if (m_isInTextMode)
	{
		int scanCode = (lParam >> 16) & 0xFF;
		Key key = s_windowsVKToKey.at(MapVirtualKey(scanCode, MAPVK_VSC_TO_VK));
		size_t key_asIndex = static_cast<size_t>(key);
		wchar_t character = (wchar_t)wParam;

		// chars are sent even when repeated
		std::for_each(m_listeners[key_asIndex].begin(), m_listeners[key_asIndex].end(), [key, character](KeyboardListener* listener) { listener->OnKeyChar(key, character); });
	}
}


