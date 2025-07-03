# RInput Library
### This is a library that makes is easy to use the RawInput API fo input APIs.
# RInput – Thin C++ Wrapper for the Windows **Raw Input API**

|                |                           |
|----------------|---------------------------|
| **Author**     | 0xE4F90A                  |
| **License**    | MIT (see `LICENSE`)       |
| **Platforms**  | Windows 7 + / x86 & x64   |
| **Language**   | ISO C++17 (no STL ABI tweaks) |

RInput exposes the full power of Raw Input (low-level keyboard + mouse) through a
single DLL.  
No global state, no window-proc spam—just call `Update()` once per frame and query
what you need.

---

## What can you do?
* **Keyboard / Scan-code / Mouse-button** state with _Down / Press / Up_ helpers
* High-resolution **mouse delta** & **wheel clicks**
* Simple **text / IME** input buffer
* **Callbacks** for key, mouse, wheel, move events
* Cursor show / hide, warping, clipping
* Device enumeration (VID / PID)
* Header-only public API (`RInput.h`) – link against `RInput.lib`, ship `RInput.dll`
* **No globals** inside the library – you get isolated `Context` handles

---

## 📑 Public API Cheat-Sheet

| Category | Function(s) | What it does |
|----------|-------------|--------------|
| **Context** | `CreateContext`, `DestroyContext`, `OnMessage`, `Update` | Owns all state. Pass `WM_INPUT` to `OnMessage`, call `Update()` once a frame. |
| **Keyboard (VK)** | `GetKeyDown / GetKey / GetKeyUp` | _Down / Held / Up_ for any `RawKey` |
| **Scan-code** | `GetScanCodeDown / ... / Up` | Same but physical scancode (inc. `E0/E1` bit) |
| **Mouse button** | `GetMouseButtonDown / ... / Up` | Five buttons (L/R/M/X1/X2) |
| **Mouse pos / delta / wheel** | `GetMousePosition` *(client)*, `GetMouseDelta`, `GetMouseWheel` | Delta & wheel return **and clear** the accumulated value |
| **Text / IME** | `EnableTextInput`, `OnChar`, `OnImeChar`, `GetCharInput`, `GetCommittedString` | UTF-32 or UTF-16 buffers |
| **Cursor utils** | `SetCursorVisible`, `ClipCursorToWindow`, `WarpCursor` | Quality-of-life helpers |
| **Callbacks** | `RegisterKeyDownCallback`, … etc. | Additive lists (call `ResetInput` to clear) |
| **Misc** | `GetRegisteredDevices`, `AnyKeyDown`, `ResetInput` | Enumeration, poll, flush |

Detailed docs live in [`docs/API.md`](docs/API.md).

---

## How to use (integrate into your WindowProc)

## `RInputSample.sln` in action
### 1-1. Library Loading
```cpp
#include <RInput.h>
#pragma comment(lib,"RInput.lib")

static RInput::Context* g_ctx = nullptr;
// Holds the handle obtained by `CreateContext`
```
`Context` is a chunk of input state.</br>
If your app has multiple windows, you can have one per window.

### 1-2. Create Context(WM_CREATE)
```cpp
g_ctx = CreateContext(hWnd, 0);
EnableTextInput(g_ctx, true);
```
#### 1. CreateContext
- Register RawInput device to `hWnd`
- Clear the internal state to zero
#### 2. `EnableTextInput(true)` starts buffering `WM_CHAR`
**Next, register various callbacks:**
```cpp
RegisterKeyDownCallback(g_ctx, OnKeyDown);
RegisterMouseWheelCallback(g_ctx, OnWheel);
```
The callback is push type, so multiple entries can be registered. </br>
There is no API to cancel, but you can empty the list with `ResetInput()`.

### 1-3. Message transfer
| Message | What call | Meaning |
|---------|-----------|---------|
| **`WM_INPUT`**      | `OnMessage()` | RawInput Parse packets and queue |
| **`WM_CHAR`**       | `OnChar()`    | 1 character (UTF-16) to character buffer|
| **`WM_IME_CHAR`**   | `OnImeChar()` | IME committed characters to commit buffer |

### 1-4. Process every frame (WM_TIMER)
```cpp
Update(g_ctx);  // Confirm state from previous frame to current frame
g_anyKey = AnyKeyDown(g_ctx);
InvalidateRect(hWnd, nullptr, FALSE);  // Redraw request
```
In `Update()`,
- `GetKeyDown / Up` etc. are enabled
- The callback is called immediately.
- The cumulative value used for `GetMouseDelta / Wheel` has been determined.

### 1-5. Input operation demo
| Operation key | Sample Behavior | Called RInput API |
|---|---|---|
| **F2** | Cursor Show / Hide Toggle | `SetCursorVisible` |
| **F3** | Clip cursor to window | `ClipCursorToWindow` |
| **F4** | Warp cursor to window center | `WarpCursor` |
| **F6** | Reset all input states | `ResetInput` |

"Mouse movement" and "wheel value" are displayed in real time at the top left of the screen.

## FAQ
| | |
|--|--|
| Q. Does it support XInput gamepads? | A. No. This is for RawInput only, so please use XInput separately.  |
| Q. I want to cancel the callback | A. There is currently no API for this. If you want to temporarily disable it, empty the list with `ResetInput()` or manage the flag on the app side.
| Q. Can I statically link a DLL? | A. This time, only DLL format is available. You can also create a static library by rebuilding the function with the same name with `__declspec(dllexport)`.









