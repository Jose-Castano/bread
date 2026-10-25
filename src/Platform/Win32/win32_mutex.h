#pragma once
#include "OS/mutex.h"
#include "Platform/platform_header.h"


static_assert(sizeof(Mutex) >= sizeof(SRWLOCK));