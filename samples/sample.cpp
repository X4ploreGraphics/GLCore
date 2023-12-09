

#include "window_wrapper.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"

#include "objs/creator.hpp"
#include "shader_header.hpp"

int main(int argc, char const *argv[])
{
    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();
        auto cube = creator.indexed_cube();

        auto vao = glcore::VertexArray{};
        vao.buffers[0] = std::move(cube.vertices);
        vao.index = std::move(cube.indices);
        vao.bind();
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

    
        const auto vertex_shader1 =  R"(layout (location = 0)  in vec3 position;
                                    void main(void)
                                    {                
                                       gl_Position = vec4(position, 1.0);  
                                    } )";

        const auto frag_shader1 =   R"(out vec4 color;     
                                    void main(void) 
                                    {  
                                        color = vec4(1.0, 0.5, 0.0, 1.0);  
                                    } )";

        auto shader = glcore::Shader{vert_header + vertex_shader1, frag_header + frag_shader1};

        window.run([&]()
        {    
            auto&& frame_buffer = glcore::FrameBuffer::get_default();
            frame_buffer.clear_color({0.1f, 0.1, 0.1, 0.0f});
            frame_buffer.clear_depth();

            shader.use();
            vao.draw();
    
        });

    }
    
    return 0;
}
