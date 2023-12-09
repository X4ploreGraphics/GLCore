

#ifndef NITROS_GLCORE_COMMON_PROCESSING_HPP
#define NITROS_GLCORE_COMMON_PROCESSING_HPP

#include "glcore_export.h"
#include "image/image.hpp"
#include "utilities/data/vecs.hpp"

namespace nitros::glcore
{
    struct ViewDim
    {
        public:
        utils::vec2Ui   left_bottom;
        utils::vec2Ui   dimension;        
    };

    class GLCORE_EXPORT ViewPort
    {
        public:
        static void dimension(const ViewDim  &view_dim);
        [[nodiscard]] static auto dimension() -> ViewDim;
    };

    class GLCORE_EXPORT Scissor
    {
        public:
        static void sissor(const ViewDim  &view_dim);
        [[nodiscard]] static auto dimension() -> ViewDim;
    };
} // namespace nitros::glcore


#endif