

#include "../window_wrapper.hpp"
#include "../shader_header.hpp"
#include "../objs/creator.hpp"
#include "../transforms/transform.hpp"
#include "../common.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"
#include "glcore/rasterizer.hpp"
#include "glcore/common_processing.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <generator/PlaneMesh.hpp>
#include <generator/AnyGenerator.hpp>

#include "utilities/files/text_reader.hpp"
#include "utilities/files/files.hpp"
#include "image/fileio.hpp"

//#include "engine/Input/input.hpp"

#include <chrono>

auto init_cube() -> nitros::utils::Uptr<nitros::glcore::VertexArray>{
    using namespace nitros;
    auto cube = nitros::Creator::get_instance().indexed_cube2({1, 1, 1}, {2, 2, 2});
    auto vao = std::make_unique<glcore::VertexArray>();
    vao->buffers[0] = std::move(cube.vertices);
    vao->buffers[1] = std::move(cube.normals);
    vao->buffers[2] = std::move(cube.tex_coords);
    vao->index = std::move(cube.indices);
    vao->bind();
    return vao;
}

auto init_plane() -> nitros::utils::Uptr<nitros::glcore::VertexArray> {
    using namespace nitros;
    auto plane = nitros::Creator::get_instance().indexed_plane({1, 1}, {1, 1});
    auto vao = std::make_unique<glcore::VertexArray>();
    vao->buffers[0] = std::move(plane.vertices);
    vao->buffers[1] = std::move(plane.normals);
    vao->buffers[2] = std::move(plane.tex_coords);
    vao->index = std::move(plane.indices);
    vao->bind();
    return vao;
}

auto apply_material(const nitros::Material  &material, nitros::glcore::Shader  &shader)
{
    using namespace nitros;

    shader.set_uniform("material.ambient", material.ambient);
    shader.set_uniform("material.diffuse", material.diffuse);

    shader.set_uniform("material.specular", material.specular);
    shader.set_uniform("material.shineness", material.shineness);
}

auto apply_light(const nitros::PointLight  &light, nitros::glcore::Shader  &shader)
{
    const auto light_str = fmt::format("pointLights[{}]", 0);
    shader.set_uniform(light_str + ".constant", light.constant);
    shader.set_uniform(light_str + ".linear", light.linear);
    shader.set_uniform(light_str + ".quadratic", light.quadratic);

    shader.set_uniform(light_str + ".diffuse", light.diffuse);
    shader.set_uniform(light_str + ".specular", light.specular);
    shader.set_uniform(light_str + ".ambient", light.ambient);
    
    shader.set_uniform(light_str + ".position", light.position);
}

int main(int argc, char const *argv[])
{
    const unsigned int SCR_WIDTH = 1280;
    const unsigned int SCR_HEIGHT = 720;
    auto window = Window{SCR_WIDTH, SCR_HEIGHT};

    using namespace nitros;
    using namespace std::string_literals;

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();
        auto&& camera = window.camera;
        
        auto cube_vao_uptr  = init_cube();
        auto plane_vao_uptr = init_plane();

        auto&& cube_vao  = *cube_vao_uptr;
        auto&& plane_vao = *plane_vao_uptr;
    
        print_error();

        camera.Position = glm::vec3{0, 4, 5};

        auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };

    #if !defined(__EMSCRIPTEN__)
        auto shader = glcore::Shader{vert_header + read_file("shaders/shadows/phong_pt.vert"), frag_header + read_file("shaders/shadows/phong_pt.frag")};
    #else
        auto shader = glcore::Shader{vert_header + read_file("shaders/shadows/phong_pt_es.vert"), frag_header + read_file("shaders/shadows/phong_pt_es.frag")};
    #endif

        auto stages = glcore::shader::Stages{};
    #if !defined(__EMSCRIPTEN__)
        stages.vertex   = vert_header + read_file("shaders/shadows/test/point_shadows_depth.vs");
        stages.geometry = geom_header + read_file("shaders/shadows/test/point_shadows_depth.gs");
    #else
        stages.vertex   = vert_header + read_file("shaders/shadows/test/point_shadows_depth2.vs");
    #endif
        stages.fragment = frag_header + read_file("shaders/shadows/test/point_shadows_depth.fs");
        auto shadow_shader = glcore::Shader{stages};

        print_error();

        glcore::Rasterizer::get_instance().enable(glcore::Rasterizer::capability::depth_test);
        glcore::Rasterizer::get_instance().disable(glcore::Rasterizer::capability::cull_face);

        const std::uint32_t shadow_width = 1024, shadow_height = 1024;

        //Depth CubeMap Texture
        auto cubemap_depth = glcore::DepthStencilTexture{utils::ImageMetaData{ { shadow_width, shadow_height }, utils::pixel::GREY_STENCIL_24_8::value }, glcore::texture::target::cube_map, false };
        auto cubemap_color = glcore::ColorTexture{utils::ImageMetaData{ { shadow_width, shadow_height }, utils::pixel::RGBA8::value }, glcore::texture::target::cube_map, false };
        {
            auto params = std::make_unique<glcore::texture::Parameters>();
            {
                using f_min = glcore::texture::Parameters::filter_min_params;
                using f_max = glcore::texture::Parameters::filter_max_params;

                using p = glcore::texture::Parameters;
                using p_min = glcore::texture::Parameters::min_filter;
                using p_mag = glcore::texture::Parameters::mag_filter;

                params->add(
                    p_min{f_min::linear},
                    p_mag{f_max::linear},
                    p::wrap_s{p::wrap_params::clamp_to_edge},
                    p::wrap_r{p::wrap_params::clamp_to_edge},
                    p::wrap_t{p::wrap_params::clamp_to_edge}
                );
            }
            cubemap_depth.desired_texture_parameters( std::move(params) );
        }
                
        

        auto&& frameBuffer_default = glcore::FrameBuffer::get_default();
        auto frameBuffer_depth = glcore::FrameBuffer{[&col = cubemap_color, &depth = cubemap_depth](){
            auto f_attachment = std::make_unique<glcore::framebuffer::Attachment>();
            f_attachment->depth_view = std::make_unique<glcore::framebuffer::Attachment::DepthView>( depth.image_view(0) ) ;
            f_attachment->color_views.push_back(col.image_view(0));
            return f_attachment;
        }()};

        print_error();

        auto [width, height] = window.window_dim();

        auto first_pass_dim = glcore::ViewDim{};
        first_pass_dim.dimension = { shadow_width, shadow_height };
        first_pass_dim.left_bottom = {0, 0};
        
        auto default_pass_dim = glcore::ViewDim{};
        default_pass_dim.dimension = {width, height};
        default_pass_dim.left_bottom = {0, 0};

        auto cube_pos  = glm::vec3{0, 1, 1};
        auto plane_pos = glm::vec3{0, 0, 0};

        auto cube_material = Material{};
        cube_material.ambient = {0.2, 0, 0, 1.0};
        cube_material.diffuse = {1.0, 0, 0, 1.0};
        cube_material.specular = {1.0, 1.0, 1.0, 1.0};
        cube_material.shineness = 32;

        auto plane_material = Material{cube_material};
        plane_material.diffuse = {0.0, 1.0, 0, 1.0};

        auto srt  = SRT{ glm::vec3{0.25,0.25,0.25}, glm::vec3{ 0, 0, 0}, cube_pos };
        auto srt2 = SRT{ glm::vec3{20, 20, 20}, glm::vec3{glm::radians(-90.f), 0, 0}, plane_pos };

        auto light = PointLight{};
        light.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
        light.specular = {1, 1, 1, 1};
        light.ambient = {0.2, 0.2, 0.2, 0.2};

        light.constant = 0.04;
        light.linear = 0.25;
        light.quadratic = 0.002;
        light.position  = {0.0f, 3.0f, 0.0f};

        using clock_type = std::chrono::steady_clock;
        auto last_time = clock_type::now();

        window.run([&]()
        {   
            auto delta_time = Time2::delta_time();
            srt.rotate.y += (glm::radians(45.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);
            auto model2 = model_matrix(srt2);
            //auto light_space = spot_light_proj * spot_light_view_mat;

            auto current_time = clock_type::now();
            last_time = current_time;

            {
                auto off = std::sin( (Time2::get_time().time_since_epoch().count() * 0.5)/1000 ) * 8.0;
                light.position[2] = off;
            }

            auto shadow_far_plane = 25.0f;

            auto shadow_render = [&](){

                auto light_pos = glm::vec3{light.position[0], light.position[1], light.position[2]};

                auto near_plane = 1.0f;
                auto far_plane  = shadow_far_plane;
                auto shadow_projection = perspective_projection_matrix( glm::radians(90.0f), shadow_width, shadow_height, near_plane, far_plane );

                auto shadow_transforms = std::vector<glm::mat4>{
                    shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                    shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                    shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                    shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                };

                shadow_shader.use();
                shadow_shader.set_uniform("lightPos", light.position );
                shadow_shader.set_uniform("far_plane", utils::vec1f{far_plane});
            
            #if !defined(__EMSCRIPTEN__)
                for (unsigned int i = 0; i < 6; ++i){
                    shadow_shader.set_uniform_matrix4fv("shadowMatrices[" + std::to_string(i) + "]", shadow_transforms[i]);
                }

                shadow_shader.set_uniform_matrix4fv("model", model);
                cube_vao.draw();

                shadow_shader.set_uniform_matrix4fv("model", model2);
                plane_vao.draw();
            #else
                for (unsigned int i = 0; i < 6; ++i){
                    shadow_shader.set_uniform_matrix4fv("shadowMatrices", shadow_transforms[i]);
                    shadow_shader.set_uniform_matrix4fv("model", model);
                    cube_vao.draw();

                    shadow_shader.set_uniform_matrix4fv("model", model2);
                    plane_vao.draw();
                }
            #endif
            };

            auto content_render = [&](){

                shader.use();

                auto [width, height] = window.window_dim();

                auto projection = perspective_projection_matrix(camera.Zoom, static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

                shader.set_uniform_matrix4fv("projection", projection);
                shader.set_uniform_matrix4fv("view", camera.GetViewMatrix());

                shader.set_uniform("lightPos", light.position);
                shader.set_uniform("viewPos", to_vec3f(camera.Position));
                shader.set_uniform("shadows", static_cast<std::int32_t>(true)); // enable/disable shadows by pressing 'SPACE'
                shader.set_uniform("far_plane", shadow_far_plane);
                shader.set_uniform("depthMap", utils::vec1i{1});

                shader.set_uniform("point_count", 1.f);
                apply_light(light, shader);
                apply_material(cube_material, shader);
            
                //cubemap_color.active_bind(1);
                cubemap_depth.active_bind(1);

                shader.set_uniform_matrix4fv("model", model);
                shader.set_uniform_matrix4fv("mvp", projection * camera.GetViewMatrix() * model);
                cube_vao.draw();
            
                //apply_light(light, shader);
                apply_material(plane_material, shader);
                
                shader.set_uniform_matrix4fv("model", model2);
                shader.set_uniform_matrix4fv("mvp", projection * camera.GetViewMatrix() * model2);
                plane_vao.draw();
            };

            glcore::Rasterizer::get_instance().enable(glcore::Rasterizer::capability::depth_test);

            //First
            glcore::ViewPort::dimension(first_pass_dim);

            frameBuffer_depth.bind();
            frameBuffer_depth.clear_color({0.f, 0.f, 0.f, 0.f});
            frameBuffer_depth.clear_depth();
            //frameBuffer_depth.clear_color({0.2f, 0.2, 0.2, 0.0f});

            print_error();
            
            shadow_render();
            
        
            //Second Pass

            //glcore::Rasterizer::get_instance().disable(glcore::Rasterizer::capability::depth_test);

            glcore::ViewPort::dimension(default_pass_dim);

            frameBuffer_default.bind();
            frameBuffer_default.clear_depth();
            frameBuffer_default.clear_color({0.4f, 0.4, 0.4, 0.0f});

            print_error();

            content_render();

            //phong_shader.use();
            //phong_shader.set_uniform("diffuse_map", 0);
            //phong_shader.set_uniform_matrix4fv("mvp", projection * model);
            ////copy_texture.active_bind(0);
            //cubemap_depth.active_bind(0);
            //plane_vao.draw();

        });

        //write_cubemap(*copy_col_texture.image_view(), "pt/CL");
        //write_cubemap(*copy_depth_texture.image_view(), "pt/DP");

        //write_cubemap(*cubemap_color.image_view(), "pt/Im");
        //write_cubemap(*cubemap_depth.image_view(), "pt/DIm");
    }
    return 0;
}
