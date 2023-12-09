

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
#include "glcore/staging_buffer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <generator/PlaneMesh.hpp>
#include <generator/AnyGenerator.hpp>

#include "utilities/files/text_reader.hpp"
#include "utilities/files/files.hpp"
#include "image/fileio.hpp"

#include "benchmark_/defaults.hpp"

B_PROFILE_UNIT(READ_TEXTURE)
B_PROFILE_UNIT(CONTENT_RENDER)

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
        

        auto cube_vao = glcore::VertexArray{};
        {
            auto cube = creator.indexed_cube2({1.0, 1.0, 1.0}, {2, 2, 2});
            cube_vao.buffers[0] = std::move(cube.vertices);
            cube_vao.buffers[1] = std::move(cube.tex_coords);
            cube_vao.buffers[2] = std::move(cube.normals);
            cube_vao.index = std::move(cube.indices);
        }
        cube_vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);
        cube_vao.bind();

        
        
        auto plane_vao = glcore::VertexArray{};
        plane_vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);
        {
            auto plane2 = creator.indexed_plane( {0.5, 0.5}, {1, 1} );
            plane_vao.buffers[0] = std::move(plane2.vertices);
            plane_vao.buffers[1] = std::move(plane2.tex_coords);
            plane_vao.buffers[2] = std::move(plane2.normals);
            plane_vao.index = std::move(plane2.indices);
            plane_vao.bind();
        }
    
        auto [width, height] = window.window_dim();

        print_error();

        auto _fovy = 45.0f;
        auto near_plane = 0.1f;
        auto far_plane  = 100.0f;
        auto projection = perspective_projection_matrix(_fovy, static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);
        auto small_projection = perspective_projection_matrix(_fovy, static_cast<float>(300), static_cast<float>(300), near_plane, far_plane);

        auto cam_view_mat = glm::lookAt(glm::vec3{0, 5, 5}, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});

        auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };

        auto shader  = glcore::Shader{ vert_header + read_file("shaders/simple_mvp.vert"), frag_header + read_file("shaders/simple_color.frag") };
        auto shader2 = glcore::Shader{ vert_header + read_file("shaders/simple_tex.vert"), frag_header + read_file("shaders/simple_tex.frag") };
        auto phong_shader = glcore::Shader{ vert_header + read_file("shaders/shadows/phong_spot.vert"), frag_header + read_file("shaders/shadows/phong_spot.frag") };

        print_error();

        glcore::Rasterizer::get_instance().enable(glcore::Rasterizer::capability::depth_test);

        auto f_p_dim = utils::vec2Ui{800, 600};
        auto copy_texture = glcore::Texture{utils::ImageMetaData{{f_p_dim[0], f_p_dim[1]}, utils::pixel::RGBA8::value}, glcore::texture::target::texture_2D , false};
        auto copy_depth_texture = glcore::DepthTexture{utils::ImageMetaData{ {f_p_dim[0], f_p_dim[1]}, utils::pixel::GREY32f::value }, glcore::texture::target::texture_2D, false};
        auto params = std::make_unique<glcore::texture::Parameters>();
        {
            using f_min = glcore::texture::Parameters::filter_min_params;
            using f_max = glcore::texture::Parameters::filter_max_params;

            using p_min = glcore::texture::Parameters::min_filter;
            using p_mag = glcore::texture::Parameters::mag_filter;

            params->add(
                p_min{f_min::nearest},
                p_mag{f_max::linear}
            );
        }
        auto params2 = std::make_unique<glcore::texture::Parameters>(*params);
        copy_texture.desired_texture_parameters( std::move(params));
        {
            using p = glcore::texture::Parameters;

            params2->add( p::border_color{utils::vec4f{1, 1, 1, 1}} );
            params2->add( p::wrap_s{ p::wrap_params::clamp_to_border } );
        }
        
        copy_depth_texture.desired_texture_parameters( std::move(params2) );
        
        //params.add(glcore::texture::Parameters::)
        //copy_texture.texture(color_image);

        auto attachment = std::make_unique<glcore::framebuffer::Attachment>();
        attachment->color_views.push_back(copy_texture.image_view());
        //attachment->depth_view = std::make_unique< glcore::framebuffer::Attachment::DepthView >(std::make_shared<glcore::DepthRenderBuffer>(utils::ImageMetaData{{300, 300}, utils::pixel::GREY32f::value}));
        attachment->depth_view = std::make_unique< glcore::framebuffer::Attachment::DepthView >(copy_depth_texture.image_view());;
        auto fbo2 = glcore::FrameBuffer{std::move(attachment)};

        print_error();

        auto first_pass_dim = glcore::ViewDim{};
        first_pass_dim.left_bottom = {0, 0};
        first_pass_dim.dimension = {f_p_dim[0], f_p_dim[1]};

        auto default_pass_dim = glcore::ViewDim{};
        default_pass_dim.left_bottom = {0, 0};
        default_pass_dim.dimension = {width, height};

        auto&& frame_buffer = glcore::FrameBuffer::get_default();

        auto srt  = SRT{ glm::vec3{0.5,0.5,0.5}, glm::vec3{ 0, 0, 0}, glm::vec3{0, 0, 0} };
        auto srt2 = SRT{ glm::vec3{20,20,20}, glm::vec3{glm::radians(90.f), 0, 0}, glm::vec3{0, -1, 0} };

        auto dir_light_pos = glm::vec3{3, 3, 0};

        auto spot_light = SpotLight{};
        spot_light.position  = to_vec3f(dir_light_pos);
        spot_light.direction = to_vec3f( glm::normalize( glm::vec3{0, 0, 0} - dir_light_pos ) );
        spot_light.ambient = {0.2, 0.2, 0.2, 1.0};
        spot_light.diffuse = {0.7, 0.7, 0.7, 1.0};
        spot_light.specular = {1, 1, 1, 1};
        spot_light.cutOff = 0.5;
        spot_light.outerCutOff = 1.9;


        auto cube_material = Material{};
        cube_material.ambient = {0.2, 0, 0, 1.0};
        cube_material.diffuse = {1.0, 0, 0, 1.0};
        cube_material.specular = {1.0, 1.0, 1.0, 1.0};
        cube_material.shineness = 32;

        auto plane_material = Material{};
        plane_material.ambient = {0.0, 0.2, 0, 1.0};
        plane_material.diffuse = {0.0, 1.0, 0, 1.0};
        plane_material.specular = {1.0, 1.0, 1.0, 1.0};
        plane_material.shineness = 32;

        auto spot_light_proj = perspective_projection_matrix(45, width, height, 0.1f, 100.0f);
        auto spot_light_view_mat = glm::lookAt(dir_light_pos, glm::vec3{0, 0, 0}, glm::vec3{0, 1, 0});


        auto reading_texture = glcore::ColorTexture{ utils::ImageMetaData{ {f_p_dim[0], f_p_dim[1]}, utils::pixel::RGBA8::value }, glcore::texture::target::texture_2D, false };

        window.run([&]()
        {   
            auto delta_time = Time2::delta_time();
            srt.rotate.y += (glm::radians(45.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);
            auto model2 = model_matrix(srt2);
            auto light_space = spot_light_proj * spot_light_view_mat;

            auto shadow_render = [&](){

                shader.use();

                shader.set_uniform_matrix4fv("mvp", light_space *  model);
                shader.set_uniform("d_color", utils::vec4f{1.0, 0.0, 0.0, 1.0});
                
                //height_texture.active_bind(0);
                //color_texture.active_bind(1);
                cube_vao.draw();

                shader.set_uniform_matrix4fv("mvp", light_space *  model2);
                shader.set_uniform("d_color", utils::vec4f{0.0, 1.0, 0.0, 1.0});

                plane_vao.draw();

            };

            auto content_render = [&](){

                phong_shader.use();

                phong_shader.set_uniform("spot_count", 1.f);
                phong_shader.set_uniform("spotLights[0].direction", spot_light.direction);
                phong_shader.set_uniform("spotLights[0].ambient"  , spot_light.ambient);
                phong_shader.set_uniform("spotLights[0].diffuse"  , spot_light.diffuse);
                phong_shader.set_uniform("spotLights[0].specular" , spot_light.specular);
                phong_shader.set_uniform("spotLights[0].position" , spot_light.position);
                phong_shader.set_uniform("spotLights[0].cutOff" , spot_light.cutOff);
                phong_shader.set_uniform("spotLights[0].outerCutOff" , spot_light.outerCutOff);

                phong_shader.set_uniform("shadow_map", utils::vec1i{0});
                copy_depth_texture.active_bind();
                phong_shader.set_uniform_matrix4fv("lightSpaceMatrix", light_space);

                phong_shader.set_uniform("material.ambient"  , cube_material.ambient);
                phong_shader.set_uniform("material.diffuse"  , cube_material.diffuse);
                phong_shader.set_uniform("material.specular" , cube_material.specular);
                phong_shader.set_uniform("material.shineness", utils::vec1f{cube_material.shineness});

                phong_shader.set_uniform_matrix4fv("mvp", projection * cam_view_mat *  model);
                phong_shader.set_uniform_matrix4fv("mv", cam_view_mat *  model);
                phong_shader.set_uniform_matrix4fv("model", model);
                phong_shader.set_uniform_matrix4fv("inv_model", glm::inverse(model));
                phong_shader.set_uniform("d_color", utils::vec4f{1.0, 0.0, 0.0, 1.0});
                
                //height_texture.active_bind(0);
                //color_texture.active_bind(1);
                cube_vao.draw();

                phong_shader.set_uniform("material.ambient"  , plane_material.ambient);
                phong_shader.set_uniform("material.diffuse"  , plane_material.diffuse);
                phong_shader.set_uniform("material.specular" , plane_material.specular);
                phong_shader.set_uniform("material.shineness", utils::vec1f{plane_material.shineness});

                phong_shader.set_uniform_matrix4fv("mvp", projection * cam_view_mat *  model2);
                phong_shader.set_uniform_matrix4fv("mv", cam_view_mat *  model2);
                phong_shader.set_uniform_matrix4fv("model", model2);
                phong_shader.set_uniform_matrix4fv("inv_model", glm::inverse(model2));
                phong_shader.set_uniform("d_color", utils::vec4f{0.4, 0.0, 0.4, 1.0});

                plane_vao.draw();
            };

            //First
            glcore::ViewPort::dimension(first_pass_dim);

            fbo2.bind();
            fbo2.clear_color({0.2f, 0.2, 0.2, 0.0f});
            fbo2.clear_depth();

            print_error();
            
            shadow_render();
        
            //Second Pass

            glcore::ViewPort::dimension(default_pass_dim);

            frame_buffer.bind();
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});
            frame_buffer.clear_depth();

            print_error();

            {
                B_MEASURE_SCOPE_DURATION(CONTENT_RENDER)
                content_render();
            }

            {
                B_MEASURE_SCOPE_DURATION(READ_TEXTURE)
                auto&& dst_image_view = reading_texture.image_view(0);
                copy_texture.image_view()->copy_to( *dst_image_view );
                //auto c_img = copy_texture.image_view()->read_image();
            }
            

            shader2.use();
            shader2.set_uniform("diffuse_map", 0);
            shader2.set_uniform_matrix4fv("mvp", projection * model);
            //copy_texture.active_bind(0);
            copy_depth_texture.active_bind(0);
            plane_vao.draw();

            std::this_thread::sleep_for(std::chrono::milliseconds(30));

        });

        auto read_buffer = glcore::StageBufferRead{};
        auto fence = read_buffer.stage_data(*copy_texture.image_view());

        for ( auto i = 0; !fence->commands_complete() && i < 10 ; i++ )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (!fence->commands_complete()) {
            std::cout << "Fence Not Complete" << std::endl;
            return EXIT_SUCCESS;
        }

        auto img_cpu = utils::image::create_cpu( copy_texture.image_view()->get_metaData().size, 
                                  copy_texture.image_view()->get_metaData().format );

        auto data = read_buffer.map();
        std::memcpy( img_cpu.buffer().data(), data.data(), data.size_bytes() );

        utils::image::write_image_png("sample_read.png", img_cpu);
        //write_cubemap(*copy_texture.image_view(), "sample_copy");

        write_cubemap(*copy_depth_texture.image_view(), "pt/Depth_s");
    }
    return 0;
}
