// RInputSample - All API demo
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>

// RInput Header `$(SolutionDir)/IncludeLib`
#include <RInput.h>

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

// RInput Library `$(SolutionDir)/IncludeLib`
#pragma comment(lib,"RInput.lib")

// Globals
static RInput::Context* g_ctx = nullptr;
static std::wstring     g_textLine;             // WM_CHAR
static int              g_wheelAcc = 0;         // Wheel data
static bool             g_showCursor = true;
static bool             g_clipCursor = false;
static POINT            g_lastDelta{ 0,0 };     // GetMouseDelta result
static bool             g_anyKey = false;       // AnyKeyDown result

constexpr int WIDTH = 500;
constexpr int HEIGHT = 500;


// Utilities
static void Log(const std::wstring& s)
{
	OutputDebugStringW((s + L"\n").c_str());
	std::wcout << s.c_str();
}

// Callbacks
void OnKeyDown(RInput::RawKey k)
{
	if (k == RInput::RawKey::F2)
	{
		g_showCursor = !g_showCursor;
		SetCursorVisible(g_ctx, g_showCursor);
	}
	else if (k == RInput::RawKey::F3)
	{
		g_clipCursor = !g_clipCursor;
		ClipCursorToWindow(g_ctx, g_clipCursor);
	}
	else if (k == RInput::RawKey::F4)
	{
		RECT rc{};
		GetClientRect(WindowFromDC(GetDC(NULL)), &rc);
		WarpCursor(g_ctx, WIDTH / 2, HEIGHT / 2);
	}
	else if (k == RInput::RawKey::F6)
	{
		ResetInput(g_ctx);
		g_textLine.clear();
		g_wheelAcc = 0;
	}

	std::wstringstream ss{};
	ss << L"[KeyDown] " << (std::uint16_t)k;
	Log(ss.str());
}
void OnKeyUp(RInput::RawKey k)
{
	std::wstringstream ss{};
	ss << L"[KeyUp] " << (std::uint16_t)k;
	Log(ss.str());
}

void OnMouseDown(RInput::MouseButton b)
{
	std::wstringstream ss{};
	ss << L"[MouseDown] " << (int)b;
	Log(ss.str());
}

void OnMouseUp(RInput::MouseButton b)
{
	std::wstringstream ss{};
	ss << L"[MouseUp] " << (int)b;
	Log(ss.str());
}

void OnMouseMove(int dx, int dy)
{
	g_lastDelta.x = dx;
	g_lastDelta.y = dy;
}

void OnWheel(int d)
{
	g_wheelAcc += d;
}

// Draw helper
static void DrawLine(HDC h, int& y, const wchar_t* fmt, ...)
{
	wchar_t buf[512]{};
	va_list vl{};
	va_start(vl, fmt);
	_vsnwprintf_s(buf, 512, fmt, vl);
	va_end(vl);
	TextOutW(h, 10, y, buf, (int)wcslen(buf));
	y += 18;
}

static void DrawWrappedText(HDC hdc, int& y, const std::wstring& str, int maxWidthPx, int lineH = 18, int marginX = 10)
{
	std::size_t head = 0;

	while (head < str.size())
	{
		std::size_t len = 1;

		for (; head + len <= str.size(); ++len)
		{
			SIZE sz{};
			GetTextExtentPoint32W(hdc, &str[head], (int)len, &sz);

			if (sz.cx > maxWidthPx - marginX * 2)
			{
				--len;
				if (len == 0) len = 1;
				break;
			}
		}
		TextOutW(hdc, marginX, y, &str[head], (int)len);
		y += lineH;
		head += len;
	}
}

// Window Proc
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		g_ctx = CreateContext(hWnd, 0);

		RegisterKeyDownCallback(g_ctx, OnKeyDown);
		RegisterKeyUpCallback(g_ctx, OnKeyUp);
		RegisterMouseDownCallback(g_ctx, OnMouseDown);
		RegisterMouseUpCallback(g_ctx, OnMouseUp);
		RegisterMouseMoveCallback(g_ctx, OnMouseMove);
		RegisterMouseWheelCallback(g_ctx, OnWheel);
		EnableTextInput(g_ctx, true);

		// Show Device List
		RInput::DeviceInfo infos[32]{};
		int n = GetRegisteredDevice(g_ctx, infos, 32);

		for (int i = 0; i < n; ++i)
		{
			std::wstringstream ss{};
			ss << L"[Device] " << infos[i].name
				<< L" VID:" << std::hex << infos[i].vendorID
				<< L" PID:" << infos[i].productID;
			Log(ss.str());
		}

		return 0;
	}

	case WM_INPUT:
	{
		OnMessage(g_ctx, hWnd, msg, wp, lp);
		return 0;
	}
	case WM_CHAR:
	{
		OnChar(g_ctx, (wchar_t)wp);
		g_textLine.push_back((wchar_t)wp);
		return 0;
	}
	case WM_IME_CHAR:
	{
		OnImeChar(g_ctx, (wchar_t)wp);
		return 0;
	}
	case WM_TIMER:
	{
		Update(g_ctx);
		g_anyKey = AnyKeyDown(g_ctx);
		InvalidateRect(hWnd, nullptr, FALSE);
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		HDC hdc = BeginPaint(hWnd, &ps);
		RECT rc{};
		GetClientRect(hWnd, &rc);
		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));   // clear

		int y = 10; SetBkMode(hdc, TRANSPARENT);

		// Keyboard
		DrawLine(hdc, y, L"===== Keyboard =====");
		DrawLine(hdc, y, L"AnyKeyDown: %d", g_anyKey);

		for (char c = 'A'; c <= 'Z'; ++c)
			if (GetKey(g_ctx, (RInput::RawKey)c))
				DrawLine(hdc, y, L"%c Hold", c);

		if (GetKey(g_ctx, RInput::RawKey::Space))
			DrawLine(hdc, y, L"Space Hold");

		// Mouse
		auto pos = GetMousePosition(g_ctx);
		DrawLine(hdc, y, L"===== Mouse =====");
		DrawLine(hdc, y, L"Pos: [%d, %d]    Delta: [%d, %d]", pos.x, pos.y, g_lastDelta.x, g_lastDelta.y);
		DrawLine(hdc, y, L"Buttons: [L: %d,   R: %d,   M: %d,   B: %d,   F: %d]",
			GetMouseButton(g_ctx, RInput::MouseButton::Left),
			GetMouseButton(g_ctx, RInput::MouseButton::Right),
			GetMouseButton(g_ctx, RInput::MouseButton::Middle),
			GetMouseButton(g_ctx, RInput::MouseButton::X1),
			GetMouseButton(g_ctx, RInput::MouseButton::X2));
		DrawLine(hdc, y, L"Cursor: %d,   Clip: %d", g_showCursor, g_clipCursor);
		DrawLine(hdc, y, L"Wheel total: %d (F2: Cursor,   F3: Clip,   F4: Warp,   F6: Reset)", g_wheelAcc);

		// Text
		DrawLine(hdc, y, L"===== TextInput =====");
		GetClientRect(hWnd, &rc);
		DrawWrappedText(hdc, y, g_textLine, rc.right - rc.left);

		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		DestroyContext(g_ctx);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

// wWinMain
int __stdcall wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ int)
{
	WNDCLASSEX wc
	{
		sizeof(wc), CS_DBLCLKS, WndProc, 0, 0, hInstance, nullptr, LoadCursor(nullptr, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1), nullptr, _T("RInputWnd"), nullptr
	};

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow(wc.lpszClassName, _T("RInput Library Test App"),
		WS_OVERLAPPEDWINDOW, 200, 200, WIDTH, HEIGHT, nullptr, nullptr, hInstance, nullptr);

	SetTimer(hWnd, 1, 33, nullptr);	 // 30 fps

	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);

	MSG msg{};
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}