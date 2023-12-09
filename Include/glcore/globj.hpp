

#ifndef GLCORE_GLOBJ_HPP
#define GLCORE_GLOBJ_HPP

#include <glcore/glcore_export.h>
#include <cstdint>

namespace nitros::glcore
{
    class GLCORE_EXPORT GLobj
    {
        public:
       GLobj() = default;
       virtual ~GLobj() = default;

        std::uint32_t get_id() const noexcept;

        protected:
        std::uint32_t   _id;
    };    

}

#endif