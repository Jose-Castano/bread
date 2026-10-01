#pragma once
#include "Core/Templates.hpp"


namespace Core::Mem
{

struct [[nodiscard]] Layout
{
    usize size;
    usize alignment;

    constexpr Layout(usize size, usize alignment)
    : size(size), alignment(alignment) {}

    constexpr Layout array_of(usize extent)
    {
        return Layout(size * extent, alignment);
    }
};

template<typename T>
inline constexpr Layout layout()
{
    return Layout(sizeof(T), alignof(T));
}

}
