

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
#include "../objs/creator.hpp"
#include "../transforms/transform.hpp"
#include "../common.hpp"

#include "utilities/files/text_reader.hpp"
#include "utilities/files/files.hpp"
#include "image/fileio.hpp"

#include <iostream>

auto apply_material(const nitros::Material  &material, nitros::glcore::Shader  &shader)
{
    using namespace nitros;

    shader.set_uniform("material.ambient", material.ambient);
    shader.set_uniform("material.diffuse", material.diffuse);

    shader.set_uniform("material.specular", material.specular);
    shader.set_uniform("material.shineness", material.shineness);
}

auto apply_light(const nitros::PointLight  &light, nitros::glcore::Shader  &shader, const std::uint32_t &index)
{
    const auto light_str = fmt::format("light[{}]", index);
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
    if(argc < 2){
        std::cout<<"Usage Exe <Diffuse>"<<std::endl;
        return 0;
    }

    auto diffuse_image = nitros::utils::image::read_image(argv[1]);
    //auto normal_image  = nitros::utils::image::read_image(argv[2]);

    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();
        auto&& camera = window.camera;
        camera.Position = {0, 0, 10};
        auto obj = creator.indexed_plane({1, 1}, {1, 1});

        auto vao = glcore::VertexArray{};
        vao.buffers[0] = std::move(obj.vertices);
        vao.buffers[1] = std::move(obj.normals);
        vao.buffers[2] = std::move(obj.tex_coords);
        vao.index = std::move(obj.indices);
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);
        vao.bind();

        {
            auto vertices = std::vector<utils::vec3f>{};
            vao.buffers[0]->read_data(vertices);
            for(auto &v : vertices)
            {
                std::cout<<v[0]<<','<<v[1]<<','<<v[2]<<std::endl;
            }
        }

        print_error();
        
        auto diffuse_texture = glcore::ColorTexture{};
        //auto normal_texture = glcore::ColorTexture{};

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
                            ,T::wrap_s{w_p::mirrored_repeat}
                            ,T::wrap_t{w_p::mirrored_repeat}
                            ,T::wrap_r{w_p::mirrored_repeat}
                            ,T::swizzle{s_v});

            auto parameters2 = std::make_unique<T>(*parameters);
            diffuse_texture.desired_texture_parameters(std::move(parameters));
            //normal_texture.desired_texture_parameters(std::move(parameters2));
            print_error();
        }

        diffuse_texture.texture(diffuse_image, true);

        print_error();

        auto srt1 = SRT{ glm::vec3{2, 2, 2}, glm::vec3{glm::radians(0.0f), 0, 0}, glm::vec3{0, 0, -3} };
        auto srt2 = SRT{ glm::vec3{10, 50, 10}, glm::vec3{glm::radians(-90.0f), 0, 0}, glm::vec3{0, -3, 0} };

        auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };

        auto shader = glcore::Shader{read_file("shaders/hdr/blin-phong.vert"), read_file("shaders/hdr/blin-phong.frag")};
        auto hdr_shader = glcore::Shader{read_file("shaders/hdr/hdr.vert"), read_file("shaders/hdr/hdr.frag")};

        auto plane_material = Material{};
        plane_material.ambient = {0.05, 0.05, 0.05, 1.0};
        plane_material.diffuse = {0, 1.0, 0, 1.0};
        plane_material.specular = {0.8, 0.8, 0.8, 1.0};
        plane_material.shineness = 32;

        auto light = PointLight{};
        light.diffuse = {0.7f, 0.7f, 0.7f, 1.0f};
        light.specular = {1, 1, 1, 1};
        light.ambient = {0.1, 0.1, 0.1, 0.1};

        light.constant = 0.04;
        light.linear = 0.75;
        light.quadratic = 0.002;
        light.position  = {0.0f, -1.0f, 2.5f};

        auto light2{light};
        light2.diffuse = {8.f, 8.f, 8.f};
        light2.specular = {10.f, 10.f, 10.f};
        light2.position = {0, 0, -1.5f};
        light2.quadratic = 0.2;


        auto [width, height] = window.window_dim();

        auto color_fb = glcore::ColorTexture{ utils::ImageMetaData{ {width, height}, utils::pixel::RGBA32f::value }, glcore::texture::target::texture_2D, false };
        auto depth_fb = glcore::DepthTexture{ utils::ImageMetaData{ {width, height}, utils::pixel::GREY8::value }, glcore::texture::target::texture_2D, false };

        auto fbo = glcore::FrameBuffer{ [&col = color_fb, &depth = depth_fb](){
            auto attachment = std::make_unique<glcore::framebuffer::Attachment>();
            attachment->color_views.push_back( col.image_view() );
            attachment->depth_view = std::make_unique< glcore::framebuffer::Attachment::DepthView >( depth.image_view() );
            return attachment;
        }() };

        print_error();
        

        auto&& frame_buffer = glcore::FrameBuffer::get_default();
        auto&& rasterizer = glcore::Rasterizer::get_instance();

        rasterizer.enable(glcore::Rasterizer::capability::depth_test);

        print_error();

        window.run([&]()
        {
            fbo.bind();        
            fbo.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            fbo.clear_color({0.4f, 0.4, 0.4, 0.0f});

            //frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            //frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            auto delta_time = Time2::delta_time();
            
            auto near_plane = 0.1f;
            auto far_plane  = 200.0f;
            auto [width, height] = window.window_dim();
            auto projection = perspective_projection_matrix( camera.Zoom , static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);
            auto view = camera.GetViewMatrix();

            auto model = model_matrix(srt1);

            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection * view * model);
            shader.set_uniform_matrix4fv("model", model);
            apply_material(plane_material, shader);
            
            shader.set_uniform("light_count", 2);
            apply_light(light, shader, 0);
            apply_light(light2, shader, 1);
            shader.set_uniform("material.diffuseTexture", 0);
            shader.set_uniform("viewPos", nitros::to_vec3f(camera.Position));
            shader.set_uniform("texture_on", 1);

            diffuse_texture.active_bind(0);
            //normal_texture.active_bind(1);

            vao.draw();

            auto model2 = model_matrix(srt2);

            shader.set_uniform_matrix4fv("mvp", projection * view * model2);
            shader.set_uniform_matrix4fv("model", model2);
            apply_material(plane_material, shader);
            shader.set_uniform("texture_on", 0);

            vao.draw();

            frame_buffer.bind();
            frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            //frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            hdr_shader.use();
            hdr_shader.set_uniform("hdrBuffer", 0);
            color_fb.active_bind(0);
            vao.draw();
        });
    }
    return 0;
}
