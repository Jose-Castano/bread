#include "OS/OS.hpp"


namespace OS
{

Collections::ResultOnly<CreateDirectoryError> create_directories(Mem::Allocator& allocator, Collections::StringView dir_path)
{
    for(usize i = 0; i < dir_path.len; i++)
    {
        char c = dir_path[i];
        if(c == '/' || i == dir_path.len - 1)
        {
            Collections::StringView sub_path = dir_path.slice(i+1);

            // try creating the upper dir
        
            Collections::ResultOnly<OS::CreateDirectoryError> result = OS::create_directory(allocator, sub_path);
            // returns at first fail, ignore the already created for upper folders
            if(result.fail() && result.error() != OS::CreateDirectoryError::AlreadyExists)
            {
                return result;
            }
        }
    }

    return {};
}

}