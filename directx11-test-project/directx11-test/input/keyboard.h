#pragma once

#include <time/time_point.h>
#include <unordered_map>


namespace xtest {

	namespace application { class WindowsApp; }

namespace input {


	enum class Key
	{
		zero,
		one,
		two,
		three,
		four,
		five,
		six,
		seven,
		eight,
		nine,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		back,
		tab,
		clear,
		enter,
		shift,
		left_shift,
		right_shift,
		ctrl,
		left_ctrl,
		right_ctrl,
		alt,
		left_alt,
		right_alt,
		pause,
		caps,
		esc,
		space_bar,
		page_up,
		page_down,
		end,
		home,
		left_arrow,
		up_arrow,
		right_arrow,
		down_arrow,
		print,
		ins,
		del,
		numpad_zero,
		numpad_one,
		numpad_two,
		numpad_three,
		numpad_four,
		numpad_five,
		numpad_six,
		numpad_seven,
		numpad_eight,
		numpad_nine,
		asterisk,
		plus,
		pipe,
		dash,
		dot,
		forward_slash,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		num_lock,
		scroll,
		oem1,
		oem2,
		oem3,
		oem4,
		oem5,
		oem6,
		oem7,
		oem_plus,
		oem_dash,
		oem_dot,
		oem_comma,
		angle_bracket,
		ALL_KEYS
	};


	struct KeyStatus
	{
		bool isDown = false;
		time::TimePoint startingTime = time::TimePoint();
	};


	class KeyboardListener
	{
	public:

		virtual void OnKeyStatusChange(Key key, const KeyStatus& status) { XTEST_UNUSED_VAR(key); XTEST_UNUSED_VAR(status); }
		virtual void OnKeyChar(Key key, wchar_t keyChar) { XTEST_UNUSED_VAR(key); XTEST_UNUSED_VAR(keyChar); }
	};


	class Keyboard
	{
		friend class application::WindowsApp;

	public:

		Keyboard();
		~Keyboard() {}

		Keyboard(Keyboard&&) = delete;
		Keyboard(const Keyboard&) = delete;
		Keyboard& operator=(Keyboard&&) = delete;
		Keyboard& operator=(const Keyboard&) = delete;

		const KeyStatus& GetKeyStatus(Key key) const;

		void AddListener(KeyboardListener* listener, std::initializer_list<Key> keys = {Key::ALL_KEYS});
		void RemoveListener(KeyboardListener* listener);
		void ClearDownKeys();

		void EnableTextMode();
		void DisableTextMode();
		bool IsInTextMode() const;


	private:

		static std::unordered_map<uint32, Key> s_windowsVKToKey;

		void OnWmKeyboardRawInput(const RAWINPUTHEADER& header, const RAWKEYBOARD& data);
		void OnWmChar(WPARAM wParam, LPARAM lParam);

		std::array<KeyStatus, static_cast<size_t>(Key::ALL_KEYS)> m_keyStatus;
		std::array<std::vector<KeyboardListener*>, static_cast<size_t>(Key::ALL_KEYS)> m_listeners;
		bool m_isInTextMode;

	};

}// input
}// xtest

