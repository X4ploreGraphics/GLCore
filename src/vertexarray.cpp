

#include <glcore/vertexarray.hpp>
#include "platform/gl.hpp"
#include <exception>

namespace nitros::glcore
{

    namespace 
    {
        inline auto get_gl(VertexArray::draw_mode  mode){
            switch (mode)
            {
            case VertexArray::draw_mode::line   : return GL_LINES;
            case VertexArray::draw_mode::line_loop   : return GL_LINE_LOOP;
            case VertexArray::draw_mode::line_strip  : return GL_LINE_STRIP;
            case VertexArray::draw_mode::points   : return GL_POINTS;
            case VertexArray::draw_mode::triangles   : return GL_TRIANGLES;
            
            #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
            case VertexArray::draw_mode::patches     : return GL_PATCHES;
            #endif
            
            default:
                return GL_TRIANGLES;
            }
        }

        inline auto get_v(Buffer::value_type    v_type) {
            switch (v_type)
            {
            case Buffer::value_type::FLOAT : return GL_FLOAT;
                    break;
            case Buffer::value_type::UINT  : return GL_UNSIGNED_INT;
                    break;
            default:
                return GL_FLOAT;
            }
        }
    } // namespace name
    

    VertexArray::VertexArray()
        :_draw_mode{draw_mode::triangles}
        ,_vertices_per_primtive{}
    {
    #if OPENGL_CORE >= 40500
        glCreateVertexArrays(1, &_id);
    #else
        glGenVertexArrays(1, &_id);
    #endif
    }

    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &_id);
    }

    void VertexArray::bind() const
    {
        glBindVertexArray(_id);
        for(auto &[index, buffer] : buffers)
        {
            buffer->bind(Buffer::buffer_type::ARRAY_BUFFER);
            glVertexAttribPointer(index, buffer->vec_length(), GL_FLOAT , GL_FALSE, 0, nullptr);
            
        #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
            if(_draw_mode == draw_mode::patches) 
            {
                glPatchParameteri(GL_PATCH_VERTICES, _vertices_per_primtive);
            }
        #endif

            glEnableVertexAttribArray(index);
        }
        if(index)
            index->bind(Buffer::buffer_type::ELEMENT_BUFFER);
        glBindVertexArray(0);
    }

    void VertexArray::draw() const
    {
        glBindVertexArray(_id);
        if(index){
            glDrawElements(get_gl(_draw_mode) , index->get_elements_count(), GL_UNSIGNED_INT, 0);
        }
        else
            glDrawArrays( get_gl(_draw_mode), 0, buffers.at(0)->get_elements_count()/buffers.at(0)->vec_length());

    #if defined(OPENGL_CORE)
        glBindVertexArray(0);
    #endif
    }

    void VertexArray::draw_index_count(std::uint32_t  index_offset, std::uint32_t  index_count) const
    {
        glBindVertexArray(_id);
        if(index)
        {
            auto row_stride   = index->row_stride();
            auto num_of_comps = index->vec_length();

            auto element_stride = row_stride / num_of_comps;
            auto ind_count = std::min( index->get_elements_count(), static_cast<std::size_t>(index_count) );
            
            glDrawElements(get_gl(_draw_mode) , index_count, GL_UNSIGNED_INT, reinterpret_cast<void*>( element_stride * index_offset ) );
        }

    #if defined(OPENGL_CORE)
        glBindVertexArray(0);
    #endif
    }

    void VertexArray::set_draw_mode(draw_mode mode, const std::uint32_t  &patch_vertices) {
        if(mode == draw_mode::patches){
            if(patch_vertices > 0)
            {
                _vertices_per_primtive = patch_vertices;
            }else
            {
                throw std::invalid_argument{"Patch vertices must be Greater Than Zero"};
            }
        }
        else if(mode == draw_mode::line || mode == draw_mode::line_loop){
            _vertices_per_primtive = 2;
        }
        else if(mode == draw_mode::triangles){
            _vertices_per_primtive = 3;
        }
        else if(mode == draw_mode::points){
            _vertices_per_primtive = 1;
        }
        _draw_mode = mode;
    }

}