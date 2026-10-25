#pragma once
#include "OS/semaphore.h"
#include "Platform/platform_header.h"


static_assert(sizeof(Semaphore) >= sizeof(HANDLE));


