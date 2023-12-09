

#include "glcore/rasterizer.hpp"
#include "platform/gl.hpp"
#include "utils/gl_conversions.hpp"

namespace nitros::glcore
{
    Rasterizer&  Rasterizer::get_instance(){
        static Rasterizer instance;
        return instance;
    }

    void Rasterizer::set_polygonMode(polygonMode mode){
#if defined(OPENGL_CORE)
        auto gl_mode = [&](){
        switch (mode)
            {
                case polygonMode::point: return GL_POINT;
                case polygonMode::line: return GL_LINE;
                default: return GL_FILL;
            }
        };
        glPolygonMode(GL_FRONT_AND_BACK, gl_mode());
#endif
    }

    void Rasterizer::set_front_face(direction   mode){
        auto gl_mode = [&mode](){
            if(mode == direction::counter_clockwise){
                return GL_CCW;
            }
            return GL_CW;
        };
        glFrontFace(gl_mode());
    }

    void Rasterizer::set_cull_face(cull mode){
        auto gl_mode = [&mode](){
            switch (mode) {
                case cull::front : return GL_FRONT;
                case cull::back : return GL_BACK;
                case cull::front_and_back : return GL_FRONT_AND_BACK;

                default : return GL_FRONT;
            }
        };
        glCullFace(gl_mode());
    }

    void Rasterizer::set_line_width(float width){
        glLineWidth(width);
    }

    void Rasterizer::set_point_size(float size){
    #if defined(OPENGL_CORE)
        glPointSize(size);
    #endif
    }

    void Rasterizer::enable(Rasterizer::capability type){
        glEnable(to_glType(type));
    }

    void Rasterizer::disable(Rasterizer::capability type){
        glDisable(to_glType(type));
    }

    bool Rasterizer::is_enabled(Rasterizer::capability type){
        return glIsEnabled(to_glType(type));
    }

    void Rasterizer::set_blend_color(const utils::vec4f &color){
        glBlendColor(color[0], color[1], color[2], color[3]);
    }

    void Rasterizer::view_port(const std::uint32_t  &x, const std::uint32_t  &y, const std::size_t  &width, const std::size_t  &height){
        glViewport(gsl::narrow_cast<GLint>(x), gsl::narrow_cast<GLint>(y), gsl::narrow_cast<GLsizei>(width), gsl::narrow_cast<GLsizei>(height));
    }


} // namespace nitros::glcore
