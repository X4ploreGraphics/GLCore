

#include <glcore/globj.hpp>

namespace nitros::glcore
{
    std::uint32_t   GLobj::get_id() const noexcept
    {
        return _id; 
    }
}