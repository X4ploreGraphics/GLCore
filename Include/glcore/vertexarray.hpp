

#ifndef GLCORE_VERTEX_ARRAY_HPP
#define GLCORE_VERTEX_ARRAY_HPP

#include "globj.hpp"
#include <map>
#include <memory>
#include "buffer.hpp"

namespace nitros::glcore
{
    class GLCORE_EXPORT VertexArray : public GLobj
    {
        public:

        enum class draw_mode{
            points,
            line,
            line_strip,
            line_loop,
            triangles,
            patches
        };

        explicit VertexArray();
        VertexArray(const VertexArray &) = delete;
        VertexArray(VertexArray &&) = default;
        ~VertexArray();

        VertexArray& operator=(const VertexArray &) = delete;
        VertexArray& operator=(VertexArray &&) = default;

        void bind() const;
        void draw() const;
        void draw_index_count(std::uint32_t  index_offset, std::uint32_t  index_count) const;
        void set_draw_mode(draw_mode    mode, const std::uint32_t  &patch_vertices = {});

        std::map<std::uint32_t, std::shared_ptr<Buffer>>  buffers;
        std::shared_ptr<Buffer>     index;

        private:
        draw_mode     _draw_mode;
        std::uint32_t   _vertices_per_primtive;
    };
}

#endif