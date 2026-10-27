#pragma once
#include "Core/Header.hpp"
#include "gpu/gpu.h"
#include "Mem/Allocator.hpp"
#include "Math/vec2.h"


struct SurfaceID;
struct Event;
struct Window;

namespace Display
{
    static constexpr i32 DefaultWidth = 600;
    static constexpr i32 DefaultHeight = 800;

    static constexpr const char* DefaultTitle = "Bread!!!";

    using HandleEventFn = void(*)(Core::Opaque*, const Event&);
};
