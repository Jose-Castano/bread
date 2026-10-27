#pragma once
#include "Display/Display.hpp"


struct Window
{
    DisableCopy(Window);
    DisableMove(Window);

    Mem::Allocator& allocator;
    Core::Opaque* impl;

    Window(Mem::Allocator& allocator, Core::Opaque* user_data, Display::HandleEventFn handle_event);
    ~Window();

    void poll();
    
    Vector2I get_size() const;
    void set_size(const Vector2I& new_size);
    MemoryAddress get_native_handle() const;
    GPU::SurfaceID get_surface() const;

    bool get_resizable() const;
    void set_resizable(bool resizable);
};
