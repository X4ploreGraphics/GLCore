

#include "../window_wrapper.hpp"
#include "../shader_header.hpp"

#include "../objs/creator.hpp"
#include "../transforms/transform.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"
#include "glcore/rasterizer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <generator/PlaneMesh.hpp>
#include <generator/AnyGenerator.hpp>

#include "utilities/files/text_reader.hpp"
#include "utilities/files/files.hpp"
#include "image/fileio.hpp"


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
        auto plane = creator.indexed_plane({1.0, 1.0}, {10, 10});
        //auto plane = creator.indexed_plane();

        auto vao = glcore::VertexArray{};
        vao.buffers[0] = std::move(plane.vertices);
        //vao.buffers[1] = std::move(plane.colors);
        vao.buffers[1] = std::move(plane.normals);
        vao.index = std::move(plane.indices);
        vao.bind();
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

        auto [width, height] = window.window_dim();

        auto _fovy = 45.0f;
        auto near_plane = 0.1f;
        auto far_plane  = 200.0f;
        auto projection = perspective_projection_matrix(_fovy, static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);

        auto srt = SRT{ glm::vec3{1,1,1}, glm::vec3{glm::radians(-45.f), 0, 0}, glm::vec3{0, 0, -3} };

        auto normals_stages = glcore::shader::Stages{};
        {
            auto shaders = std::array<std::vector<std::uint8_t>, 4>{
                utils::reader::read_binary("shaders/geometry/normals.vert"),
                utils::reader::read_binary("shaders/geometry/normals.frag"),
                utils::reader::read_binary("shaders/geometry/normals.geom"),
            };

            normals_stages.vertex   = vert_header + std::string{}.append(shaders[0].begin(), shaders[0].end());
            normals_stages.fragment = frag_header + std::string{}.append(shaders[1].begin(), shaders[1].end());
            normals_stages.geometry = geom_header + std::string{}.append(shaders[2].begin(), shaders[2].end());
        }

        auto normal_shader = glcore::Shader{normals_stages};
        auto stages = normals_stages;
        stages.geometry.clear();

        auto shader = glcore::Shader{stages};

        glcore::Rasterizer::get_instance().enable(glcore::Rasterizer::capability::depth_test);
        glcore::Rasterizer::get_instance().set_polygonMode(glcore::Rasterizer::polygonMode::fill);

        window.run([&]()
        {    
            auto&& frame_buffer = glcore::FrameBuffer::get_default();
            frame_buffer.clear_depth();
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            auto delta_time = Time2::delta_time();

            srt.rotate.y += (glm::radians(45.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);

            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection *  model);
            shader.set_uniform("cc", utils::vec4f{1, 0, 0, 1});

            vao.draw();

            normal_shader.use();
            normal_shader.set_uniform("cc", utils::vec4f{0, 1, 0, 1});
            normal_shader.set_uniform_matrix4fv("mvp", projection * model);

            vao.draw();
        });

    }
    return 0;
}
