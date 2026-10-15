#include "Collections/Array.hpp"
#include "Debug/Fail.hpp"
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

    // TODO: This only works for windows

    Collections::StringView command_line = OS::get_command_line();

    // parse until space
    Collections::Array<Collections::StringView> arguments{global_allocator, 4, {}};
    usize begin = Core::MaxValue<usize>;
    for(usize i = 0; i < command_line.len; i++)
    {
        char c = command_line[i];
        if(c == ' ' && begin == Core::MaxValue<usize>)
        {
            continue;
        }
        else if(c == ' ' && begin != Core::MaxValue<usize>)
        {
            (void)arguments.emplace(command_line.add(begin).slice(i - begin));
            begin = Core::MaxValue<usize>;
        }
        else if(c != ' ' && begin == Core::MaxValue<usize>)
        {
            begin = i;
        }
    }

    BreadMain(
        {
            .allocator = global_allocator,
            .arguments = arguments.slice(),
        }
    );

    BreadPlatformShutdown();

    OS::exit(0);
}
