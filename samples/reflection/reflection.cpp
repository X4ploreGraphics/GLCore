

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"
#include "glcore/commands.hpp"
#include "glcore/rasterizer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../window_wrapper.hpp"
#include "../shader_header.hpp"
#include "../objs/creator.hpp"
#include "../transforms/transform.hpp"
#include "../common.hpp"

#include "utilities/files/text_reader.hpp"
#include "utilities/files/files.hpp"
#include "image/fileio.hpp"

#include <iostream>

int main(int argc, char const *argv[])
{
#if !defined(__EMSCRIPTEN__)
    if(argc < 7){
        std::cout<<"Usage Exe <right> <left> <top> <bottom> <front> <back>"<<std::endl;
        return 0;
    }

    auto paths = std::array<std::string, 6>{ argv[1], argv[2], argv[3], argv[4], argv[5], argv[6] };
#else
    auto paths = std::array<std::string, 6>{ "textures/bricks2.jpg", "textures/bricks2.jpg", "textures/bricks2.jpg", "textures/bricks2.jpg", "textures/bricks2.jpg", "textures/bricks2.jpg" };
#endif
    auto images = std::vector<nitros::utils::ImageCpu>{};
    for(const auto& path : paths)
    {
        images.push_back(nitros::utils::image::read_image(path));
    }

    //auto image = nitros::gen_checker_GREY16({300, 300}, {50u, 50u});

    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;

    

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();
        auto obj = creator.indexed_cube2({1, 1, 1}, {2, 2, 2});

        auto tex_points = std::vector<utils::vec2f>{
            {-1.0f, -1.0f},
            { 1.0f, -1.0f},
            { 1.0f,  1.0f},
            {-1.0f,  1.0f}
        };

        auto tex =  std::make_shared<glcore::Buffer>();
        tex->write_data(tex_points);

        print_error();

        auto vao = glcore::VertexArray{};
        vao.buffers[0] = std::move(obj.vertices);
        vao.buffers[1] = tex;
        vao.index = std::move(obj.indices);
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);
        vao.bind();

        auto cube_vao = glcore::VertexArray{};
        {
            auto cube = creator.indexed_cube2({.5, .5, .5}, {2, 2, 2});
            cube_vao.buffers[0] = std::move(cube.vertices);
            cube_vao.buffers[1] = std::move(cube.tex_coords);
            cube_vao.buffers[2] = std::move(cube.normals);
            cube_vao.index = std::move(cube.indices);
        }
        cube_vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);
        cube_vao.bind();

        print_error();
        
        auto texture = glcore::ColorTexture{glcore::texture::target::cube_map};
        //auto texture = glcore::ColorTexture{};

        print_error();
        {
            using T = glcore::texture::Parameters;
            using f_p_min = T::filter_min_params;
            using f_p_max = T::filter_max_params;
            using w_p = T::wrap_params;
            using s_c = T::swizzle_value::component;

            auto s_v = T::swizzle_value{};
            //s_v.set_rgba( s_c::red, s_c::green, s_c::blue, s_c::alpa );

            auto parameters = std::make_unique<T>();
            parameters->add(T::min_filter{f_p_min::linear_mipmap_linear}
                            ,T::mag_filter{f_p_max::linear}
                            ,T::wrap_s{w_p::clamp_to_edge}
                            ,T::wrap_t{w_p::clamp_to_edge}
                            ,T::wrap_r{w_p::clamp_to_edge}
                            ,T::swizzle{s_v});

            texture.desired_texture_parameters(std::move(parameters));
            print_error();
        }

        auto arr_int = std::array<int, 6>{};
        auto sp = gsl::span<int, 6>{&(*arr_int.begin()), 6 };

        auto span_images = gsl::span<const utils::ImageCpu , 6>{ &(*images.begin()), 6 };
        texture.texture_cube_map(span_images, true);
        //texture.texture(span_images[0], true);

        print_error();
        print_error();

        std::cout<<"Mip Map "<< texture.current_mip_levels()<<std::endl;

        auto view = texture.image_view(8);
        print_error();
        if(view){
            auto [w, h] = view->get_metaData().size;
            std::cout<<w<<','<<h<<std::endl;
        }

        auto current_parameters = texture.current_texture_parameters();

        print_error();

        auto [width, height] = window.window_dim();

        auto _fovy = 45.0f;
        auto near_plane = 0.1f;
        auto far_plane  = 200.0f;
        auto projection = perspective_projection_matrix(_fovy, static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);

        auto srt = SRT{ glm::vec3{1,1,1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0} };

        const auto vertex_shader3 =  R"(layout (location = 0)  in vec3 position;

                                    out vec3 VTexCoords;

                                    uniform mat4 projection;
                                    uniform mat4 view;

                                    void main(void)
                                    {
                                       vec4 pos = projection * view * vec4(position, 1.0);            
                                       gl_Position = pos.xyww ;
                                       VTexCoords = position;
                                    } )";

        const auto frag_shader3 =   R"(in vec3 VTexCoords;
                                    out vec4 color;     
                                    uniform samplerCube cubemap;
                                    void main(void) 
                                    {  
                                        color = texture(cubemap, VTexCoords);
                                    } )";

        //auto shader = glcore::Shader{vertex_shader1, frag_shader2};
        auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };

        auto shader = glcore::Shader{vert_header + vertex_shader3, frag_header + frag_shader3};
        auto shader3  = glcore::Shader{ vert_header + read_file("shaders/reflection/env.vert"), frag_header + read_file("shaders/reflection/env_refl.frag") };

        print_error();

        auto rotate_vec = glm::vec3{0, 0, 0};
        auto&& frame_buffer = glcore::FrameBuffer::get_default();

        print_error();

        auto &&rasterizer = glcore::Rasterizer::get_instance();

        rasterizer.enable(glcore::Rasterizer::capability::depth_test);
        

        std::cout<<"Enabled "<< rasterizer.is_enabled(glcore::Rasterizer::capability::depth_test);

        window.run([&]()
        {        
            frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            auto delta_time = Time2::delta_time();
            auto model = model_matrix(srt);

            auto delta = 4 * glm::radians(5.0f) * Time2::delta_time().count() / 1000;
            rotate_vec.y += delta;

            auto rotate_quat = glm::quat{rotate_vec};
            auto eye = rotate_quat * glm::vec3{0, 0, 3};
            
            auto cam_view_mat = glm::lookAt(eye , glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});

            //Sky Box Pass
            frame_buffer.depth_mask(false);
            frame_buffer.depthFunc(glcore::FrameBuffer::comparison::less_or_equal);
            shader.use();

            auto cam_mat = glm::mat4{ glm::mat3{cam_view_mat} };
            shader.set_uniform_matrix4fv("projection", projection * cam_mat);
            shader.set_uniform_matrix4fv("view", glm::mat4{1.0});
            shader.set_uniform("cubemap", 0);
            texture.active_bind();
            vao.draw();

            //Box contents pass
            frame_buffer.depth_mask(true);
            frame_buffer.depthFunc(glcore::FrameBuffer::comparison::less);
            shader3.use();
            shader3.set_uniform_matrix4fv("model", model);
            shader3.set_uniform_matrix4fv("view", cam_view_mat);
            shader3.set_uniform_matrix4fv("projection", projection);
            shader3.set_uniform("cameraPos", utils::vec3f{eye.x, eye.y, eye.z});
            shader3.set_uniform("skybox", 0);
            texture.active_bind();
            //shader3.set_uniform_matrix4fv("mvp", projection * cam_view_mat * model);
            cube_vao.draw();

        });

    }
    return 0;
}
