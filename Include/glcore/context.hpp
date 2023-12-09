

#ifndef GLCORE_CONTEXT_HPP
#define GLCORE_CONTEXT_HPP

#include "glcore/glcore_export.h"
#include <any>
#include <string>

namespace nitros::glcore
{
    GLCORE_EXPORT void load_context(std::any ptr);
    GLCORE_EXPORT auto get_context() -> std::any;

    /**
     * @returns Version -> in the format 40500 Major (2 Digits) Minor (3 Digits) , String core es
     * */
    GLCORE_EXPORT auto get_version() -> std::pair<std::uint32_t, std::string>;
}

#endif