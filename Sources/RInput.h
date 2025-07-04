/******************************************************************************
 *  File: RInput.h
 *
 *  Helpers for RawInputAPI
 *
 *  Copyright (c) 0xE4F90A. All rights reserved.
 *  Licensed under the MIT License.
 *****************************************************************************/
#ifndef RINPUT_INCLUDE_GUARD
#define RINPUT_INCLUDE_GUARD
 // ---------------------------------------
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <Windows.h>
#include <cstdint>

#ifdef RINPUT_EXPORTS
#  define RINPUT_API __declspec(dllexport)
#else
#  define RINPUT_API __declspec(dllimport)
#endif

namespace RInput
{
	// KeyCode
	enum class RawKey : std::uint16_t
	{
		Unknown = 0,

		// --- Control / Edit ---
		Backspace = 0x08,
		Tab = 0x09,
		Clear = 0x0C,
		Enter = 0x0D,
		Shift = 0x10,
		Control = 0x11,
		Alt = 0x12,
		Pause = 0x13,
		CapsLock = 0x14,
		Escape = 0x1B,
		Space = 0x20,
		PageUp = 0x21,
		PageDown = 0x22,
		End = 0x23,
		Home = 0x24,
		Left = 0x25,
		Up = 0x26,
		Right = 0x27,
		Down = 0x28,
		Select = 0x29,
		Print = 0x2A,
		Execute = 0x2B,
		PrintScreen = 0x2C,
		Insert = 0x2D,
		DeleteKey = 0x2E,
		Help = 0x2F,

		// --- 0-9 ---
		Num0 = '0',
		Num1 = '1',
		Num2 = '2',
		Num3 = '3',
		Num4 = '4',
		Num5 = '5',
		Num6 = '6',
		Num7 = '7',
		Num8 = '8',
		Num9 = '9',

		// --- A-Z ---
		A = 'A',
		B = 'B',
		C = 'C',
		D = 'D',
		E = 'E',
		F = 'F',
		G = 'G',
		H = 'H',
		I = 'I',
		J = 'J',
		K = 'K',
		L = 'L',
		M = 'M',
		N = 'N',
		O = 'O',
		P = 'P',
		Q = 'Q',
		R = 'R',
		S = 'S',
		T = 'T',
		U = 'U',
		V = 'V',
		W = 'W',
		X = 'X',
		Y = 'Y',
		Z = 'Z',

		// --- Windows & Incorporated ---
		LWin = 0x5B,
		RWin = 0x5C,
		Apps = 0x5D,
		Sleep = 0x5F,

		// --- NumPad ---
		NumPad0 = 0x60,
		NumPad1 = 0x61,
		NumPad2 = 0x62,
		NumPad3 = 0x63,
		NumPad4 = 0x64,
		NumPad5 = 0x65,
		NumPad6 = 0x66,
		NumPad7 = 0x67,
		NumPad8 = 0x68,
		NumPad9 = 0x69,
		NumPadMul = 0x6A,
		NumPadAdd = 0x6B,
		NumPadSep = 0x6C,
		NumPadSub = 0x6D,
		NumPadDec = 0x6E,
		NumPadDiv = 0x6F,

		// --- F1-F24 ---
		F1 = 0x70,
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
		F13 = 0x7C,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,

		// --- Lock ---
		NumLock = 0x90,
		ScrollLock = 0x91,

		// --- Extended modifiers key---
		LShift = 0xA0,
		RShift = 0xA1,
		LControl = 0xA2,
		RControl = 0xA3,
		LAlt = 0xA4,
		RAlt = 0xA5,

		// --- Browser / Media key ---
		BrowserBack = 0xA6,
		BrowserForward = 0xA7,
		BrowserRefresh = 0xA8,
		BrowserStop = 0xA9,
		BrowserSearch = 0xAA,
		BrowserFavorites = 0xAB,
		BrowserHome = 0xAC,
		VolumeMute = 0xAD,
		VolumeDown = 0xAE,
		VolumeUp = 0xAF,
		MediaNextTrack = 0xB0,
		MediaPrevTrack = 0xB1,
		MediaStop = 0xB2,
		MediaPlayPause = 0xB3,
		LaunchMail = 0xB4,
		LaunchMediaSel = 0xB5,
		LaunchApp1 = 0xB6,
		LaunchApp2 = 0xB7,

		// --- OEM (US Keyboard) ---
		OemSemicolon = 0xBA,	// ;:
		OemPlus = 0xBB,	// =+
		OemComma = 0xBC,	// ,<
		OemMinus = 0xBD,	// -_
		OemPeriod = 0xBE,	// .>
		OemSlash = 0xBF,	// /?
		OemTilde = 0xC0,	// `~
		OemOpenBracket = 0xDB,	// [{
		OemBackslash = 0xDC,	// \|
		OemCloseBracket = 0xDD,	// ]}
		OemQuote = 0xDE,	// '"
		Oem8 = 0xDF,
		Oem102 = 0xE2,	// "<>" or "\|"

		// --- IME / etc ---
		ProcessKey = 0xE5,
		ImeKana = 0x15,	// Kana / Kanji
		ImeJunja = 0x17,
		ImeFinal = 0x18,
		ImeConvert = 0x1C,
		ImeNonConvert = 0x1D,
		ImeAccept = 0x1E,
		ImeModeChange = 0x1F,
	};

	enum class MouseButton : std::uint8_t
	{
		Left = 0,
		Right,
		Middle,
		X1,
		X2
	};

	struct Vec2
	{
		int x;
		int y;
	};

	struct Context;	// forward

	// Callback
	using KeyCallback = void(*)(RawKey);
	using MouseBtnCallback = void(*)(MouseButton);
	using MouseMoveCallback = void(*)(int dx, int dy);
	using MouseWheelCallback = void(*)(int delta);

	struct DeviceInfo
	{
		wchar_t       name[128];
		std::uint16_t vendorID;
		std::uint16_t productID;
	};

}
// namespace RInput

extern "C"
{
	// Base
	RINPUT_API RInput::Context* CreateContext(HWND, std::uint32_t flags = 0);
	RINPUT_API void             DestroyContext(RInput::Context*);
	RINPUT_API void             OnMessage(RInput::Context*, HWND, UINT, WPARAM, LPARAM);
	RINPUT_API void             Update(RInput::Context*);

	// keyboard
	RINPUT_API bool	GetKeyDown(RInput::Context*, RInput::RawKey);
	RINPUT_API bool	GetKey(RInput::Context*, RInput::RawKey);
	RINPUT_API bool	GetKeyUp(RInput::Context*, RInput::RawKey);

	// ScanCode
	RINPUT_API bool	GetScanCodeDown(RInput::Context*, std::uint16_t);
	RINPUT_API bool	GetScanCode(RInput::Context*, std::uint16_t);
	RINPUT_API bool	GetScanCodeUp(RInput::Context*, std::uint16_t);

	// Mouse
	RINPUT_API bool			GetMouseButtonDown(RInput::Context*, RInput::MouseButton);
	RINPUT_API bool			GetMouseButton(RInput::Context*, RInput::MouseButton);
	RINPUT_API bool			GetMouseButtonUp(RInput::Context*, RInput::MouseButton);
	RINPUT_API RInput::Vec2	GetMousePosition(RInput::Context*);
	RINPUT_API RInput::Vec2	GetMouseDelta(RInput::Context*);
	RINPUT_API int			GetMouseWheel(RInput::Context*);

	// TextInput
	RINPUT_API void	EnableTextInput(RInput::Context*, bool);
	RINPUT_API void	OnChar(RInput::Context*, wchar_t);
	RINPUT_API void	OnImeChar(RInput::Context*, wchar_t);
	RINPUT_API int	GetCharInput(RInput::Context*, std::uint32_t* out, int max);
	RINPUT_API int	GetCommittedString(RInput::Context*, wchar_t* out, int max);

	// Cursor
	RINPUT_API void	SetCursorVisible(RInput::Context*, bool);
	RINPUT_API void	ClipCursorToWindow(RInput::Context*, bool);
	RINPUT_API void	WarpCursor(RInput::Context*, int, int);

	// Callback
	RINPUT_API void	RegisterKeyDownCallback(RInput::Context*, RInput::KeyCallback);
	RINPUT_API void	RegisterKeyUpCallback(RInput::Context*, RInput::KeyCallback);
	RINPUT_API void	RegisterMouseDownCallback(RInput::Context*, RInput::MouseBtnCallback);
	RINPUT_API void	RegisterMouseUpCallback(RInput::Context*, RInput::MouseBtnCallback);
	RINPUT_API void	RegisterMouseMoveCallback(RInput::Context*, RInput::MouseMoveCallback);
	RINPUT_API void	RegisterMouseWheelCallback(RInput::Context*, RInput::MouseWheelCallback);

	// Device list
	RINPUT_API int GetRegisteredDevice(RInput::Context*, RInput::DeviceInfo*, int);

	// Utility
	RINPUT_API bool	AnyKeyDown(RInput::Context*);
	RINPUT_API void	ResetInput(RInput::Context*);
}
// extern "C"
#endif
