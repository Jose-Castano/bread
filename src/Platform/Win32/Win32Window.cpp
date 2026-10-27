#include "Platform/Win32/Win32Window.hpp"

#include "input/input.h"


static inline Win32Window& _get_window_impl(Core::Opaque* impl);

static inline LRESULT WINAPI _default_window_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if(GetWindowLongPtrA(handle, GWLP_USERDATA) == 0)
	{
		return DefWindowProcA(handle, msg, wparam, lparam);
	}

	Window& window = *reinterpret_cast<Window*>(GetWindowLongPtrA(handle, GWLP_USERDATA));
	Win32Window& window_data = _get_window_impl(window.impl);

	switch (msg)
	{
	case WM_DESTROY:
		break;
	case WM_CLOSE:
	{
		InputEventWindowClose event =
		{
			Event{EventType::WindowClose},
			window
		};
		window_data.handle_event_fn(window_data.user_data, event);
	}
		break;
	case WM_CAPTURECHANGED:
	{
		if (window_data.mouse_captured)
		{
			window_data.mouse_captured = false;
		}
		break;
	}
	case WM_KILLFOCUS:
	case WM_CANCELMODE:
	case WM_MOUSELEAVE:
	{
		Input::update_mouse_button(MouseButton::Left, false);
		Input::update_mouse_button(MouseButton::Middle, false);
		Input::update_mouse_button(MouseButton::Right, false);

		MouseButton buttons[] = {MouseButton::Left, MouseButton::Middle, MouseButton::Right};
		for(MouseButton btn : buttons)
		{
			Input::data.mouse_buttons[i32(btn)] = false;

			InputEventMouseButton event = {};
			event.type = EventType::MouseButton;
			event.position = Input::get_mouse_position();
			event.pressed = false;
			event.button = btn;
			window_data.handle_event_fn(window_data.user_data, event);
		}
	}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	{
		MouseButton button = MouseButton::Middle;
		if(msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP)
		{
			button = MouseButton::Left;
		}
		else if(msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP)
		{
			button = MouseButton::Right;
		}

		Input::update_mouse_button(button,
			msg == WM_LBUTTONDOWN
			|| msg == WM_RBUTTONDOWN
			|| msg == WM_MBUTTONDOWN);

		if(Input::get_mouse_state(button))
		{
			SetCapture(handle);
    		window_data.mouse_captured = true;
		}
		else
		{
			if(window_data.mouse_captured)
			{
				ReleaseCapture();
				window_data.mouse_captured = false;
			}
		}

		// Getting bread screen space coordinates
		RECT client_rect;
		GetClientRect(handle, &client_rect);

		Vector2 screen_space_position = Vector2(
			f32(GET_X_LPARAM(lparam)),
			(client_rect.bottom - client_rect.top) - f32(GET_Y_LPARAM(lparam))
		);

		InputEventMouseButton event = {};
		event.type = EventType::MouseButton;
		event.position = screen_space_position;
		event.pressed = Input::get_mouse_state(button);
		event.button = button;
		window_data.handle_event_fn(window_data.user_data, event);
		return 0;
	}
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		//u16 repeat_count = lparam & 0xFFFF;
		u8 scan_code = (lparam >> 16) & 0xFF;

		if (Input::get_key_state(Key(wparam)) == KeyState::RequestNewState
			&& (msg == WM_SYSKEYDOWN || msg == WM_KEYDOWN))
		{
			return 0;
		}

		// Extended key
		// Shift, Ctr, Alt
		KeyState new_key_state = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) ?
			KeyState::Pressed : KeyState::Released;

		UINT real_vk = wparam & 0xFFFU;
		if (wparam == VK_SHIFT || wparam == VK_CONTROL || wparam == VK_MENU)
		{
			real_vk = MapVirtualKeyExA(scan_code, MAPVK_VSC_TO_VK_EX, GetKeyboardLayout(0));
		}

		Input::update_key_state(Key(real_vk), new_key_state);
		Input::update_key_state(Key(wparam), new_key_state);

		InputEventKey event = {};
		event.type = EventType::KeyPress;
		event.pressed = Input::get_key_state(Key(wparam)) == KeyState::Pressed;
		event.key = static_cast<Key>(wparam);

		window_data.handle_event_fn(window_data.user_data, event);
		return 0;
	}
	break;
	case WM_SIZE:
	{
		GetWindowRect(handle, &window_data.window_rect);

		InputEventWindowResize event =
		{
			Event{EventType::WindowResize},
			window,
			Vector2I(
				window_data.window_rect.right - window_data.window_rect.left,
				window_data.window_rect.bottom - window_data.window_rect.top
			),
		};
		
		window_data.handle_event_fn(window_data.user_data, event);
		return 0;
	}
	break;
	case WM_MOUSEMOVE:
	{
		RECT client_rect;
		GetClientRect(handle, &client_rect);

		Vector2 screen_space_position = Vector2(
			f32(GET_X_LPARAM(lparam)),
			(client_rect.bottom - client_rect.top) - f32(GET_Y_LPARAM(lparam))
		);
		Input::data.mouse_position = screen_space_position;

		InputEventMouseMove event = {};
		event.type = EventType::MouseMove;
		event.position = screen_space_position;
		window_data.handle_event_fn(window_data.user_data, event);
		return 0;
	}
	break;
	}

	return DefWindowProcA(handle, msg, wparam, lparam);
}

static inline Win32Window& _get_window_impl(Core::Opaque* impl)
{
	return *impl->cast<Win32Window*>();
}

static void _try_initialize_class()
{
	static bool initialized = false;

	if(!initialized)
	{
		WNDCLASSEXA wc = {};
		wc.cbSize = sizeof(wc);
		wc.lpfnWndProc = &_default_window_proc;
		wc.lpszClassName = Win32Window::WindowClassName;
		wc.hIcon = LoadIconA(0, IDI_APPLICATION);
		wc.hCursor = LoadCursorA(0, IDC_ARROW);
		wc.hIconSm = LoadIconA(0, IDI_APPLICATION);
		RegisterClassExA(&wc);
	}
}

Window::Window(Mem::Allocator& allocator, Core::Opaque* user_data, Display::HandleEventFn handle_event)
: allocator(allocator)
{
	_try_initialize_class();

	Win32Window& window_data = *allocator.object<Win32Window>();
	impl = Core::Opaque::from(window_data);

	window_data.user_data = user_data;
	window_data.handle_event_fn = handle_event;

	RECT window_rect = {};
	window_rect.right = Display::DefaultWidth;
	window_rect.bottom = Display::DefaultHeight;

	AdjustWindowRectExForDpi(&window_rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW, GetDpiForSystem());

	window_data.window_rect = window_rect;
	window_data.handle = CreateWindowExA(
		WS_EX_OVERLAPPEDWINDOW, Win32Window::WindowClassName, Display::DefaultTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top,
		0, 0, GetModuleHandleA(nullptr), 0
	);

	SetWindowLongPtrA(window_data.handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(window_data.handle, SW_SHOW);

	window_data.surface = GPU::surface_create({.window_native_handle = get_native_handle()});
}

Window::~Window()
{
	Win32Window& window_data = _get_window_impl(impl);

	GPU::surface_destroy(window_data.surface);
	
	DestroyWindow(window_data.handle);

	allocator.free(Mem::to_bytes(Slice<u8>(impl->cast<u8*>(), 1)));
}

void Window::poll()
{
	Win32Window& window_data = _get_window_impl(impl);
	
	MSG msg;
	while(PeekMessageA(&msg, window_data.handle, 0, 0, PM_REMOVE) != 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}
}

Vector2I Window::get_size() const
{
	Win32Window& window_data = _get_window_impl(impl);
	RECT rect;
	GetClientRect(window_data.handle, &rect);

	return Vector2I(
		rect.right - rect.left,
		rect.bottom - rect.top
	);
}

void Window::set_size(const Vector2I& new_size)
{
	Win32Window& window_data = _get_window_impl(impl);

	RECT rect = { 0, 0, new_size.x, new_size.y };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

	i32 width = rect.right - rect.left;
	i32 height = rect.bottom - rect.top;

	MoveWindow(
		window_data.handle, window_data.window_rect.left, window_data.window_rect.top,
		width, height, TRUE
	);
}

MemoryAddress Window::get_native_handle() const
{
	Win32Window& window_data = _get_window_impl(impl);
	return MemoryAddress(window_data.handle);
}

GPU::SurfaceID Window::get_surface() const
{
	Win32Window& window_data = _get_window_impl(impl);
	return window_data.surface;
}

bool Window::get_resizable() const
{
	Win32Window& window_data = _get_window_impl(impl);
	LONG style = ::GetWindowLongA(window_data.handle, GWL_STYLE);
	return style & (WS_SIZEBOX);
}

void Window::set_resizable(bool resizable)
{
	Win32Window& window_data = _get_window_impl(impl);
	LONG style = ::GetWindowLongA(window_data.handle, GWL_STYLE);

	if(resizable)
	{
		::SetWindowLongA(window_data.handle, GWL_STYLE, style | (WS_SIZEBOX | WS_MAXIMIZEBOX));
	}
	else
	{
		::SetWindowLongA(window_data.handle, GWL_STYLE, style & ~(WS_SIZEBOX | WS_MAXIMIZEBOX));
	}
}
