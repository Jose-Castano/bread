#include "Platform/win32/win32_os.h"

#include "Debug/Fail.hpp"


f64 OS::get_time()
{
    i64 frequency;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));

    LARGE_INTEGER platform_time;
    QueryPerformanceCounter(&platform_time);

    return (f64(platform_time.QuadPart) / f64(frequency));
}

void OS::exit(u64 code)
{
    ExitProcess(static_cast<UINT>(code));
}

usize OS::get_page_size()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

Collections::StringView OS::get_command_line()
{
    LPSTR str = ::GetCommandLineA();
    return Collections::StringView(str, Core::NullTerminatedLen(str));
}

OS::Handle OS::load_library(Mem::Allocator& allocator, Collections::StringView lib_path)
{
    Slice path = allocator.array<char>(lib_path.len + 1);
    Mem::copy(path, lib_path);

    OS::Handle library = reinterpret_cast<OS::Handle>(LoadLibraryA(path.ptr()));
    allocator.free(Mem::to_bytes(path));

    return library;
}

void OS::unload_library(OS::Handle library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

OS::VoidFunction OS::get_proc_address(Mem::Allocator& allocator, OS::Handle library, Collections::StringView symbol_name)
{
    Slice symbol = allocator.array<char>(symbol_name.len + 1);
    Mem::copy(symbol, symbol_name);
    
    OS::VoidFunction func = reinterpret_cast<OS::VoidFunction>(
        GetProcAddress(reinterpret_cast<HMODULE>(library), symbol.ptr())
    );
    
    allocator.free(Mem::to_bytes(symbol));
    return func;
}

Slice<u8> OS::map_memory(usize memory_size, OS::MapAccess access)
{
    const usize aligned_size = Mem::align_up(memory_size, get_page_size());
    Slice<u8> ptr{};

    switch (access)
    {
    case OS::Unknown:
        break;
    case OS::ReadWrite:
    {
        ptr.items = reinterpret_cast<u8*>(
            VirtualAllocEx(GetCurrentProcess(),
                nullptr, aligned_size,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE
            )
        );
        ptr.len = aligned_size;
    }
        break;
    case OS::ReadWriteExecute:
    {
        ptr.items = reinterpret_cast<u8*>(
            VirtualAllocEx(GetCurrentProcess(),
                nullptr, aligned_size,
                MEM_RESERVE | MEM_COMMIT,
                PAGE_EXECUTE_READWRITE
            )
        );
        ptr.len = aligned_size;
    }
        break;
    }

    return ptr;
}

void OS::unmap_memory(const Slice<u8>& memory)
{
    VirtualFreeEx(GetCurrentProcess(), memory.items, 0, MEM_RELEASE);
}

OS::QueryMemory OS::query_memory(const Slice<u8>& memory)
{
    MEMORY_BASIC_INFORMATION mem_info;
    VirtualQueryEx(GetCurrentProcess(), memory.items, &mem_info, sizeof(mem_info));
    
    return OS::QueryMemory
    {
        .base_address = MemoryAddress(mem_info.BaseAddress),
        .region_size = usize(mem_info.RegionSize),
    };    
}

bool OS::set_current_directory(Mem::Allocator& allocator, Collections::StringView dir)
{
    Slice path = allocator.array<char>(dir.len + 1);
    Mem::copy(path, dir);
    bool result = SetCurrentDirectoryA(path.ptr()) == TRUE;
    allocator.free(Mem::to_bytes(path));

    return result;
}
