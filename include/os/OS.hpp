#pragma once
#include "Collections/StringView.hpp"
#include "Collections/Result.hpp"
#include "Mem/Allocator.hpp"


namespace OS
{
    enum MapAccess
    {
        Unknown = 0,
        ReadWrite,
        ReadWriteExecute,
    };

    using VoidFunction = void(*)();

    struct QueryMemory
    {
        MemoryAddress base_address;
        usize region_size;
    };

    f64 get_time();

    void exit(u64 code);
    usize get_page_size();
    Collections::StringView get_command_line();

    MemoryAddress load_library(Mem::Allocator& allocator, Collections::StringView lib_path);
    void unload_library(MemoryAddress library);
    VoidFunction get_proc_address(Mem::Allocator& allocator, MemoryAddress library, Collections::StringView symbol_name);

    Slice<u8> map_memory(usize memory_size, MapAccess access);
    void unmap_memory(const Slice<u8>& memory);
    QueryMemory query_memory(const Slice<u8>& memory);

    bool set_current_directory(Mem::Allocator& allocator, Collections::StringView dir_path);

    enum class CreateDirectoryError
    {
        PathNotFound,
        AlreadyExists,
    };

    Collections::ResultOnly<CreateDirectoryError> create_directory(Mem::Allocator& allocator, Collections::StringView dir_path);

    Collections::ResultOnly<CreateDirectoryError> create_directories(Mem::Allocator& allocator, Collections::StringView dir_path);
}
