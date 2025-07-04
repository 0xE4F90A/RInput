/******************************************************************************
 *  File: RInput.cpp
 *
 *  Helpers for RawInputAPI
 *
 *  Copyright (c) 0xE4F90A. All rights reserved.
 *  Licensed under the MIT License.
 *****************************************************************************/
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "RInput.h"
#include <vector>
#include <array>
#include <algorithm>
#include <cstring>

namespace RInput
{
	//―――――― Context ――――――
	struct Context
	{
		HWND hwnd = nullptr;
		std::array<bool, 256> currKey{}, prevKey{};
		std::array<bool, 256> currScan{}, prevScan{};
		std::array<bool, 5>   currMouse{}, prevMouse{};
		std::vector<std::pair<std::uint16_t, bool>> keyEvents;
		std::vector<std::pair<std::uint16_t, bool>> scanEvents;
		std::vector<std::pair<std::uint8_t, bool>> mouseEvents;

		std::int32_t deltaX = 0, deltaY = 0, wheelDelta = 0;

		bool textInputEnabled = false;

		std::vector<std::uint32_t> charBuffer;
		std::vector<wchar_t> commitBuffer;

		// Callback list
		std::vector<KeyCallback> keyDownCBs, keyUpCBs;
		std::vector<MouseBtnCallback> mbDownCBs, mbUpCBs;
		std::vector<MouseMoveCallback> moveCBs;
		std::vector<MouseWheelCallback> wheelCBs;
	};

	// Helper
	static void clampClientRect(HWND hwnd, RECT& r)
	{
		GetClientRect(hwnd, &r);
		POINT ul{ r.left,r.top };
		POINT lr{ r.right,r.bottom };

		ClientToScreen(hwnd, &ul);
		ClientToScreen(hwnd, &lr);

		r = { ul.x,ul.y,lr.x,lr.y };
	}

	static int g_cursorVisible = 1;
	static void setCursorVisibleSafe(bool vis)
	{
		if (vis && g_cursorVisible <= 0)
		{
			while (ShowCursor(TRUE) < 0) {}
			g_cursorVisible = 1;
		}
		else if (!vis && g_cursorVisible > 0)
		{
			while (ShowCursor(FALSE) >= 0) {}
			g_cursorVisible = -1;
		}
	}
}
// namespace RInput

using namespace RInput;

extern "C"
{
	//―――― Create / Destroy ――――
	RINPUT_API Context* CreateContext(HWND hwnd, std::uint32_t)
	{
		auto* c = new Context{};
		c->hwnd = hwnd;

		RAWINPUTDEVICE rid[2]{};

		rid[0] = { 0x01, 0x06, RIDEV_INPUTSINK, hwnd }; // keyboard
		rid[1] = { 0x01, 0x02, RIDEV_INPUTSINK, hwnd }; // mouse

		RegisterRawInputDevices(rid, 2, sizeof(rid[0]));

		return c;
	}

	RINPUT_API void DestroyContext(Context* c)
	{
		delete c;
	}

	//―――― OnMessage ――――
	RINPUT_API void OnMessage(Context* c, HWND, UINT msg, WPARAM, LPARAM lParam)
	{
		if (msg != WM_INPUT)
			return;

		UINT sz = 0;

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &sz, sizeof(RAWINPUTHEADER)) != 0 || !sz)
			return;

		std::vector<std::uint8_t> buf(sz);

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buf.data(), &sz, sizeof(RAWINPUTHEADER)) != sz)
			return;

		auto* raw = reinterpret_cast<RAWINPUT*>(buf.data());

		if (raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			const auto& kb = raw->data.keyboard;

			UINT vk = kb.VKey == 0xFF ? MapVirtualKeyEx(kb.MakeCode, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(0)) : kb.VKey;
			bool down = !(kb.Flags & RI_KEY_BREAK);
			c->keyEvents.emplace_back(static_cast<std::uint16_t>(vk), down);

			std::uint16_t sc = static_cast<std::uint16_t>(kb.MakeCode & 0xFF);

			if (kb.Flags & RI_KEY_E0)
				sc |= 0x100;

			if (kb.Flags & RI_KEY_E1)
				sc |= 0x200;

			c->scanEvents.emplace_back(sc, down);
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE)
		{
			const auto& m = raw->data.mouse;

			c->deltaX += m.lLastX;
			c->deltaY += m.lLastY;

			if (m.usButtonFlags & RI_MOUSE_WHEEL)
				c->wheelDelta += static_cast<int16_t>(m.usButtonData);

			auto push = [&](std::uint8_t b, bool d)
				{
					c->mouseEvents.emplace_back(b, d);
				};

			if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)  push(0, true);
			if (m.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)    push(0, false);
			if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) push(1, true);
			if (m.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)   push(1, false);
			if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)push(2, true);
			if (m.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)  push(2, false);
			if (m.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)     push(3, true);
			if (m.usButtonFlags & RI_MOUSE_BUTTON_4_UP)       push(3, false);
			if (m.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)     push(4, true);
			if (m.usButtonFlags & RI_MOUSE_BUTTON_5_UP)       push(4, false);
		}
	}

	//―――― Update ――――
	RINPUT_API void Update(Context* c)
	{
		c->prevKey = c->currKey;
		c->prevScan = c->currScan;
		c->prevMouse = c->currMouse;

		for (auto& [vk, down] : c->keyEvents)
		{
			if (vk < 256)
				c->currKey[vk] = down;

			RawKey rk = static_cast<RawKey>(vk);

			auto& vec = down ? c->keyDownCBs : c->keyUpCBs;

			for (auto& cb : vec)
				cb(rk);
		}

		for (auto& [sc, d] : c->scanEvents)
			if (sc < 256)
				c->currScan[sc] = d;

		for (auto& [btn, d] : c->mouseEvents)
		{
			if (btn < 5)
				c->currMouse[btn] = d;

			MouseButton mb = static_cast<MouseButton>(btn);
			auto& vec = d ? c->mbDownCBs : c->mbUpCBs;

			for (auto& cb : vec)
				cb(mb);
		}

		if (c->deltaX || c->deltaY)
			for (auto& cb : c->moveCBs)
				cb(c->deltaX, c->deltaY);

		if (c->wheelDelta)
		{
			int clicks = c->wheelDelta / 120;
			for (auto& cb : c->wheelCBs) cb(clicks);
			c->wheelDelta = 0;
		}

		c->keyEvents.clear();
		c->scanEvents.clear();
		c->mouseEvents.clear();
	}

	//―――― Getter ――――
#define FN(name) RINPUT_API bool name

	FN(GetKeyDown)(Context* c, RawKey k)
	{
		auto i = (std::uint16_t)k;
		return c->currKey[i] && !c->prevKey[i];
	}

	FN(GetKey)    (Context* c, RawKey k)
	{
		return c->currKey[(std::uint16_t)k];
	}

	FN(GetKeyUp)  (Context* c, RawKey k)
	{
		auto i = (std::uint16_t)k;
		return !c->currKey[i] && c->prevKey[i];
	}

	FN(GetScanCodeDown)(Context* c, std::uint16_t s)
	{
		return c->currScan[s] && !c->prevScan[s];
	}

	FN(GetScanCode)    (Context* c, std::uint16_t s)
	{
		return c->currScan[s];
	}

	FN(GetScanCodeUp)  (Context* c, std::uint16_t s)
	{
		return !c->currScan[s] && c->prevScan[s];
	}

	FN(GetMouseButtonDown)(Context* c, MouseButton b)
	{
		auto i = (std::uint8_t)b;
		return c->currMouse[i] && !c->prevMouse[i];
	}

	FN(GetMouseButton)(Context* c, MouseButton b)
	{
		return c->currMouse[(std::uint8_t)b];
	}

	FN(GetMouseButtonUp)(Context* c, MouseButton b)
	{
		auto i = (std::uint8_t)b;
		return !c->currMouse[i] && c->prevMouse[i];
	}

#undef FN

	RINPUT_API Vec2 GetMousePosition(Context* c)
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(c->hwnd, &p);
		return{ p.x,p.y };
	}

	RINPUT_API Vec2 GetMouseDelta(Context* c)
	{
		Vec2 d{ c->deltaX,c->deltaY };
		c->deltaX = c->deltaY = 0;
		return d;
	}

	RINPUT_API int  GetMouseWheel(Context* c)
	{
		int n = c->wheelDelta / 120;
		c->wheelDelta = 0;
		return n;
	}

	RINPUT_API void EnableTextInput(Context* c, bool e)
	{
		c->textInputEnabled = e;
	}

	RINPUT_API void OnChar(Context* c, wchar_t ch)
	{
		if (c->textInputEnabled)
			c->charBuffer.push_back(static_cast<std::uint32_t>(ch));
	}

	RINPUT_API void OnImeChar(Context* c, wchar_t ch)
	{
		if (c->textInputEnabled) c->commitBuffer.push_back(ch);
	}

	RINPUT_API int GetCharInput(Context* c, std::uint32_t* out, int max)
	{
		int n = (int)std::min<std::size_t>(max, c->charBuffer.size());
		std::copy_n(c->charBuffer.begin(), n, out);
		c->charBuffer.erase(c->charBuffer.begin(), c->charBuffer.begin() + n);
		return n;
	}

	RINPUT_API int GetCommittedString(Context* c, wchar_t* out, int max)
	{
		int n = (int)std::min<std::size_t>(max, c->commitBuffer.size());
		std::copy_n(c->commitBuffer.begin(), n, out);
		c->commitBuffer.erase(c->commitBuffer.begin(), c->commitBuffer.begin() + n);
		return n;
	}

	//―――― Cursor――――
	RINPUT_API void SetCursorVisible(Context*, bool v)
	{
		setCursorVisibleSafe(v);
	}

	RINPUT_API void ClipCursorToWindow(Context* c, bool on)
	{
		if (on)
		{
			RECT r;
			clampClientRect(c->hwnd, r);
			ClipCursor(&r);
		}
		else  ClipCursor(nullptr);
	}
	RINPUT_API void WarpCursor(Context* c, int x, int y)
	{
		POINT p{ x,y };
		ClientToScreen(c->hwnd, &p);
		SetCursorPos(p.x, p.y);
	}

	//―――― Callbackregistration ――――
	RINPUT_API void RegisterKeyDownCallback(Context* c, KeyCallback cb)
	{
		if (cb) c->keyDownCBs.push_back(cb);
	}

	RINPUT_API void RegisterKeyUpCallback(Context* c, KeyCallback cb)
	{
		if (cb) c->keyUpCBs.push_back(cb);
	}

	RINPUT_API void RegisterMouseDownCallback(Context* c, MouseBtnCallback cb)
	{
		if (cb) c->mbDownCBs.push_back(cb);
	}

	RINPUT_API void RegisterMouseUpCallback(Context* c, MouseBtnCallback cb)
	{
		if (cb) c->mbUpCBs.push_back(cb);
	}

	RINPUT_API void RegisterMouseMoveCallback(Context* c, MouseMoveCallback cb)
	{
		if (cb) c->moveCBs.push_back(cb);
	}

	RINPUT_API void RegisterMouseWheelCallback(Context* c, MouseWheelCallback cb)
	{
		if (cb) c->wheelCBs.push_back(cb);
	}

	//―――― Device list ――――
	RINPUT_API int GetRegisteredDevice(Context*, DeviceInfo* out, int max)
	{
		UINT n = 0;

		if (GetRawInputDeviceList(nullptr, &n, sizeof(RAWINPUTDEVICELIST)) != 0)
			return 0;

		std::vector<RAWINPUTDEVICELIST> lst(n);

		if (GetRawInputDeviceList(lst.data(), &n, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) \
			return 0;

		int cnt = 0;
		for (UINT i = 0; i < n && cnt < max; ++i)
		{
			UINT sz = 0; \
				GetRawInputDeviceInfo(lst[i].hDevice, RIDI_DEVICENAME, nullptr, &sz);

			if (!sz) continue;

			std::vector<wchar_t> name(sz + 1);
			GetRawInputDeviceInfo(lst[i].hDevice, RIDI_DEVICENAME, name.data(), &sz);

			RID_DEVICE_INFO info{ sizeof(info) };
			UINT infosz = info.cbSize;
			GetRawInputDeviceInfo(lst[i].hDevice, RIDI_DEVICEINFO, &info, &infosz);

			wcscpy_s(out[cnt].name, _countof(out[cnt].name), name.data());

			if (info.dwType == RIM_TYPEHID)
			{
				out[cnt].vendorID = static_cast<std::uint16_t>(info.hid.dwVendorId);
				out[cnt].productID = static_cast<std::uint16_t>(info.hid.dwProductId);
			}
			else
				out[cnt].vendorID = out[cnt].productID = 0;

			++cnt;
		}
		return cnt;
	}

	//―――― Utility ――――
	RINPUT_API bool AnyKeyDown(Context* c)
	{
		for (int i = 0; i < 256; ++i)
			if (c->currKey[i] && !c->prevKey[i])
				return true;

		return false;
	}
	RINPUT_API void ResetInput(Context* c)
	{
		c->charBuffer.clear();
		c->commitBuffer.clear();
		c->keyEvents.clear();
		c->scanEvents.clear();
		c->mouseEvents.clear();
		c->deltaX = c->deltaY = c->wheelDelta = 0;
	}

}
// extern "C"
