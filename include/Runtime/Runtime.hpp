#pragma once
#include "Collections/StringView.hpp"
#include "Mem/Allocator.hpp"


namespace Runtime
{

struct ProcessInit
{
    Mem::Allocator& allocator;
    Slice<Collections::StringView> arguments;
};

}