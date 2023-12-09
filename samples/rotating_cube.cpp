

#include "window_wrapper.hpp"
#include "shader_header.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "objs/creator.hpp"
#include "transforms/transform.hpp"

#include <iostream>
#include <thread>

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
        vao.buffers[1] = std::move(cube.colors);
        vao.index = std::move(cube.indices);
        vao.bind();
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

        auto [width, height] = window.window_dim();

        auto _fovy = glm::radians(60.0f);
        auto near_plane = 0.1f;
        auto far_plane  = 200.0f;
        auto projection = perspective_projection_matrix(_fovy, static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);

        auto srt = SRT{ glm::vec3{1,1,1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, -3} };

        auto model_mat = model_matrix( srt );
        auto model_vec = glm::vec4{ 1, 1, 1, 1 } * (projection * model_mat);

        std::cout <<"Model Vec  "<< model_vec[0] << ','
                  << model_vec[1] << ','
                  << model_vec[2] << ','
                  << model_vec[3] << ','
                  << std::endl;

        const auto vertex_shader1 =  R"( layout (location = 0)  in vec3 position;
                                    layout (location = 1)  in vec3 vColor;
                                    uniform mat4 mvp;
                                    out vec3 VColor;
                                    void main(void)
                                    {                
                                       gl_Position = mvp * vec4(position, 1.0);  

                                        if(gl_Position.z > 1) {
                                            VColor = vec3(1, 0, 0);
                                        }
                                       else {
                                            VColor = vec3(0, 1, 0);
                                        }
                                       // VColor = vColor;
                                    } )";

        const auto frag_shader1 =   R"( in vec3 VColor;
                                    out vec4 color;     
                                    void main(void) 
                                    {  
                                        //color = vec4(1.0, 0.5, 0.0, 1.0);  
                                        color = vec4(VColor, 1.0);
                                    } )";

        auto shader = glcore::Shader{vert_header + vertex_shader1, frag_header + frag_shader1};

        auto t1 = std::chrono::steady_clock::now();
        window.run([&]()
        {    
            auto&& frame_buffer = glcore::FrameBuffer::get_default();
            frame_buffer.bind();
            frame_buffer.clear_color({0.5f, 0.1, 0.1, 0.0f});
            frame_buffer.clear_depth();

            auto delta_time = Time2::delta_time();

            srt.rotate.y += (glm::radians(90.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);

            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection *  model);
            vao.draw();

            using namespace std::chrono_literals;
            std::this_thread::sleep_until(t1 + 10ms);
            t1 = std::chrono::steady_clock::now();
            //std::cout<<"Frame Count "<<frame_count++<<std::endl;
        });

    }
    return 0;
}
