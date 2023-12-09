

#ifndef GLCORE_COMMAND_EXECUTION_HPP
#define GLCORE_COMMAND_EXECUTION_HPP

#include "glcore/glcore_export.h"
#include <vector>

namespace nitros::glcore
{
    namespace command
    {
        enum class error_type{
            no_error, invalid_enum, invalid_value, invalid_operation, invalid_framebuffer_operation, out_of_memory
        };
        
        GLCORE_EXPORT std::vector<error_type> error();
    } // namespace command
} // namespace nitros::glcore


#endif