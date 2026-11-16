#include "Platform/Win32/Win32OS.hpp"

#include "Collections/Array.hpp"
#include "Debug/Fail.hpp"


namespace OS
{

f64 get_time()
{
    i64 frequency;
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));

    LARGE_INTEGER platform_time;
    QueryPerformanceCounter(&platform_time);

    return (f64(platform_time.QuadPart) / f64(frequency));
}

void exit(u64 code)
{
    ExitProcess(static_cast<UINT>(code));
}

usize get_page_size()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return info.dwPageSize;
}

Collections::StringView get_command_line()
{
    LPSTR str = ::GetCommandLineA();
    return Collections::StringView(str, Core::NullTerminatedLen(str));
}

MemoryAddress load_library(Mem::Allocator& allocator, Collections::StringView lib_path)
{
    Slice path = allocator.array<char>(lib_path.len + 1);
    Mem::copy(path, lib_path);

    MemoryAddress library = reinterpret_cast<MemoryAddress>(LoadLibraryA(path.ptr()));
    allocator.free(Mem::to_bytes(path));

    return library;
}

void unload_library(MemoryAddress library)
{
    FreeLibrary(reinterpret_cast<HMODULE>(library));
}

VoidFunction get_proc_address(Mem::Allocator& allocator, MemoryAddress library, Collections::StringView symbol_name)
{
    Slice symbol = allocator.array<char>(symbol_name.len + 1);
    Mem::copy(symbol, symbol_name);
    
    VoidFunction func = reinterpret_cast<VoidFunction>(
        GetProcAddress(reinterpret_cast<HMODULE>(library), symbol.ptr())
    );
    
    allocator.free(Mem::to_bytes(symbol));
    return func;
}

Slice<u8> map_memory(usize memory_size, MapAccess access)
{
    const usize aligned_size = Mem::align_up(memory_size, get_page_size());
    Slice<u8> ptr{};

    switch (access)
    {
    case Unknown:
        break;
    case ReadWrite:
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
    case ReadWriteExecute:
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

void unmap_memory(const Slice<u8>& memory)
{
    VirtualFreeEx(GetCurrentProcess(), memory.items, 0, MEM_RELEASE);
}

QueryMemory query_memory(const Slice<u8>& memory)
{
    MEMORY_BASIC_INFORMATION mem_info;
    VirtualQueryEx(GetCurrentProcess(), memory.items, &mem_info, sizeof(mem_info));
    
    return QueryMemory
    {
        .base_address = MemoryAddress(mem_info.BaseAddress),
        .region_size = usize(mem_info.RegionSize),
    };    
}

bool set_current_directory(Mem::Allocator& allocator, Collections::StringView dir_path)
{
    Slice path = allocator.array<char>(dir_path.len + 1);
    Mem::copy(path, dir_path);
    bool result = SetCurrentDirectoryA(path.ptr()) == TRUE;
    allocator.free(Mem::to_bytes(path));

    return result;
}

Collections::String get_current_directory(Mem::Allocator& allocator)
{
    DWORD size = GetCurrentDirectoryA(0, nullptr);

    Collections::Array<char> path{allocator, size, {}};
    path.resize(size);
    GetCurrentDirectoryA(size, path.slice().ptr());

    return Collections::String(allocator, 0, Collections::StringView(path.slice().ptr(), path.count - 1));
}

Collections::ResultOnly<CreateDirectoryError> create_directory(Mem::Allocator& allocator, Collections::StringView dir_path)
{
    Slice path = allocator.array<char>(dir_path.len + 1);
    Mem::copy(path, dir_path);
    HRESULT result = CreateDirectoryA(path.ptr(), NULL);
    allocator.free(Mem::to_bytes(path));

    if(result == 0)
    {
        DWORD error_code = GetLastError();
        
        if(error_code == ERROR_PATH_NOT_FOUND)
        {
            return CreateDirectoryError::PathNotFound;
        }
        else if(error_code == ERROR_ALREADY_EXISTS)
        {
            return CreateDirectoryError::AlreadyExists;
        }
    }

    return {};
}

Collections::ResultOnly<CreateProcessError> create_process(Mem::Allocator& allocator, Collections::StringView app_path,
    Collections::StringView command_line, ProcessFlags flags)
{
    Collections::String combined{allocator, 0, app_path};
    combined.add(" ");
    combined.add(command_line);

    Slice cmd = allocator.array<char>(combined.count + 1);
    Mem::copy(cmd, combined.view());

    DWORD creation_flags = 0;
    if(Core::HasValue(flags & ProcessFlags::NoWindow))
    {
        creation_flags |= CREATE_NO_WINDOW;
    }

    if(Core::HasValue(flags & ProcessFlags::NewConsole))
    {
        creation_flags |= CREATE_NEW_CONSOLE;
    }

    STARTUPINFOA si{};
    si.cb = sizeof(STARTUPINFOA);
    
    PROCESS_INFORMATION pi{};

    BOOL result = CreateProcessA(
        nullptr, cmd.ptr(),
        nullptr, nullptr, TRUE, creation_flags,
        nullptr, nullptr, &si, &pi
    );

    allocator.free(Mem::to_bytes(cmd));

    if(result == 0)
    {
        DWORD last_error = GetLastError();
        if(last_error == ERROR_FILE_NOT_FOUND)
        {
            return CreateProcessError::FileNotFound;
        }

        return CreateProcessError::Fail;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return {};
}

Collections::ResultOnly<EnumFilesError> enum_files_in_directory(Mem::Allocator& allocator, Collections::StringView dir_path,
    Collections::Array<Collections::String>& files)
{
    Collections::String search_path{allocator, 0, dir_path};
    search_path.add("/*");

    
    Slice search_path_c = allocator.array<char>(search_path.count + 1);
    Mem::copy(search_path_c, search_path.view());

    WIN32_FIND_DATAA find_data;
    HANDLE file_handle = FindFirstFileA(search_path_c.ptr(), &find_data);

    if (file_handle == INVALID_HANDLE_VALUE) {
        return EnumFilesError::Fail;
    }

    do {
        // Skip directories (this also skips "." and "..")
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            (void)files.emplace(
                allocator, 0,
                Collections::StringView(find_data.cFileName, Core::NullTerminatedLen(find_data.cFileName))
            );
        }
    } while (FindNextFileA(file_handle, &find_data) != 0);

    FindClose(file_handle);
    allocator.free(Mem::to_bytes(search_path_c));

    return {};
}

}
