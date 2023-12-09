

#ifndef NITROS_GLCORE_RASTERIZER_HPP
#define NITROS_GLCORE_RASTERIZER_HPP

#include "glcore/glcore_export.h"
#include "utilities/data/vecs.hpp"
#include <cstdint>

namespace nitros::glcore
{
    class GLCORE_EXPORT Rasterizer
    {
        public:

        enum class polygonMode{
            point, line, fill
        };
        enum class direction {
            clockwise, counter_clockwise
        };
        enum class cull {
            front, back, front_and_back
        };

        enum class capability{
            blend,
            cull_face,
            depth_test,
            scissor_test
        };

        Rasterizer(const Rasterizer&) = delete;
        Rasterizer(Rasterizer&&) = delete;

        Rasterizer& operator=(const Rasterizer &) = delete;
        Rasterizer& operator=(Rasterizer &&) = delete;

        static Rasterizer&  get_instance();

        //Not supported in OpenGL ES, use the draw lines or points in the mesh
        void set_polygonMode(polygonMode mode);
        void set_front_face(direction   mode);
        void set_cull_face(cull mnde);

        void set_line_width(float width);

        //Not supported in OpenGL ES, use the shader point size
        void set_point_size(float size);

        void enable(capability type);
        void disable(capability type);
        bool is_enabled(capability type);

        void set_blend_color(const utils::vec4f &color);

        [[deprecated]] void view_port(const std::uint32_t  &x, const std::uint32_t  &y, const std::size_t  &width, const std::size_t  &height);

        private:
        explicit Rasterizer() = default;

    };
    
} // namespace nitros::glcore


#endif