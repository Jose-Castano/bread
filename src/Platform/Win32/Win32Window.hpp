#pragma once
#include "Collections/FreeList.hpp"
#include "Display/Window.hpp"
#include "Platform/platform_header.h"


struct Win32Window
{
    static constexpr const char* WindowClassName = "Bread:window";

    Core::Opaque* user_data;
    GPU::SurfaceID surface;
    HWND handle;
    RECT window_rect;
    bool mouse_captured;
    
    Display::HandleEventFn handle_event_fn;
};

