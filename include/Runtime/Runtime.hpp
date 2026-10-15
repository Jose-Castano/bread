#pragma once
#include "Mem/Allocator.hpp"


namespace Runtime
{

struct ProcessInit
{
    Mem::Allocator& allocator;
};

}