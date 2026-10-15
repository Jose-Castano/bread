#include "Audio/Audio.hpp"
#include "display/display.h"
#include "Debug/Fail.hpp"
#include "gpu/gpu.h"
#include "Mem/GenericAllocator.hpp"
#include "os/os.h"
#include "Runtime/Runtime.hpp"


void* operator new(size_t)
{
    FailOn(true, "avoid 'new' statements!");
    return reinterpret_cast<void*>(0xFFFFFFFF'FFFFFFFF);
}

void* operator new[](size_t)
{
    FailOn(true, "avoid 'new' statements!");
    return reinterpret_cast<void*>(0xFFFFFFFF'FFFFFFFF);
}

void operator delete(void*)
{
    FailOn(true, "avoid 'delete' statements!");
}

void operator delete[](void*)
{
    FailOn(true, "avoid 'delete' statements!");
}

void BreadPlatformInitialize();
void BreadPlatformShutdown();

extern void BreadMain(const Runtime::ProcessInit& process_info);

void GlobalBreadEntryPoint()
{
    BreadPlatformInitialize();
	
    Mem::GenericAllocator global_allocator;

    BreadMain(
        {
            .allocator = global_allocator,
        }
    );

    BreadPlatformShutdown();

    OS::exit(0);
}
