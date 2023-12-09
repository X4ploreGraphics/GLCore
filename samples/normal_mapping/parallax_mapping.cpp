

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
    const auto light_str = fmt::format("light", 0);
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

#if !defined(__EMSCRIPTEN__)
    if(argc < 4){
        std::cout<<"Usage Exe <Diffuse> <Normal> <DepthMap>"<<std::endl;
        return 0;
    }

    auto diffuse_image = nitros::utils::image::read_image(argv[1]);
    auto normal_image  = nitros::utils::image::read_image(argv[2]);
    auto depth_image   = nitros::utils::image::read_image(argv[3]);
#else
    auto diffuse_image = nitros::utils::image::read_image("textures/bricks2.jpg");
    auto normal_image  = nitros::utils::image::read_image("textures/bricks2_normal.jpg");
    auto depth_image   = nitros::utils::image::read_image("textures/bricks2_disp.jpg");
#endif

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

    #if !defined(__EMSCRIPTEN__)
        auto indices = std::vector<utils::vec3Ui>{};
        vao.index->read_data(indices);
    #else
        auto indices = std::vector<utils::vec3Ui>{obj.indices_data};
    #endif

        for(auto &ind : indices )
        {
            std::cout<<ind[0]<<','<<ind[1]<<','<<ind[2]<<std::endl;
        }

        #if !defined(__EMSCRIPTEN__)
            auto vertices = std::vector<utils::vec3f>{};
            vao.buffers[0]->read_data(vertices);
        #else
            auto vertices = std::vector<utils::vec3f>{obj.vertices_data};
        #endif


        #if !defined(__EMSCRIPTEN__)
            auto tex_coords = std::vector<utils::vec2f>{};
            vao.buffers[2]->read_data(tex_coords);
        #else
            auto tex_coords = std::vector<utils::vec2f>{obj.tex_coords_data};
        #endif

        auto pos1 = glm::vec3{vertices[0].at(0), vertices[0].at(1), vertices[0].at(2)};
        auto pos2 = glm::vec3{vertices[1].at(0), vertices[1].at(1), vertices[1].at(2)};
        auto pos3 = glm::vec3{vertices[2].at(0), vertices[2].at(1), vertices[2].at(2)};
        auto pos4 = glm::vec3{vertices[3].at(0), vertices[3].at(1), vertices[3].at(2)};

        auto uv1 = glm::vec2{ tex_coords[0].at(0), tex_coords[0].at(1) };
        auto uv2 = glm::vec2{ tex_coords[1].at(0), tex_coords[1].at(1) };
        auto uv3 = glm::vec2{ tex_coords[2].at(0), tex_coords[2].at(1) };
        auto uv4 = glm::vec2{ tex_coords[3].at(0), tex_coords[3].at(1) };

        auto compute_tangent = [](const glm::vec3  &pos1, const glm::vec3  &pos2, const glm::vec3  &pos3, const glm::vec2  &uv1, const glm::vec2  &uv2, const glm::vec2 &uv3) -> std::pair<glm::vec3, glm::vec3>
        {

            auto edge1 = pos2 - pos1;
            auto edge2 = pos3 - pos1;
            auto deltaUV1 = uv2 - uv1;
            auto deltaUV2 = uv3 - uv1;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            auto tangent1 = glm::vec3{};
            tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
            tangent1 = glm::normalize(tangent1);

            auto bitangent1 = glm::vec3{};
            bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
            bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
            bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
            bitangent1 = glm::normalize(bitangent1);  

            return {tangent1, bitangent1};
        };

        auto [tangent1, bitangent1] = compute_tangent(pos1, pos2, pos3, uv1, uv2, uv3);
        auto [tangent2, bitangent2] = compute_tangent(pos2, pos4, pos3, uv2, uv4, uv3);

        auto tangent_buffer = std::make_unique<glcore::Buffer>();
        tangent_buffer->write_data( std::vector<utils::vec3f>{
            {tangent1.x, tangent1.y, tangent1.z},
            {tangent1.x, tangent1.y, tangent1.z},
            {tangent1.x, tangent1.y, tangent1.z},
            {tangent1.x, tangent1.y, tangent1.z},
        });

        auto bitangent_buffer = std::make_unique<glcore::Buffer>();
        bitangent_buffer->write_data( std::vector<utils::vec3f>{
            {bitangent1.x, bitangent1.y, bitangent1.z},
            {bitangent1.x, bitangent1.y, bitangent1.z},
            {bitangent1.x, bitangent1.y, bitangent1.z},
            {bitangent1.x, bitangent1.y, bitangent1.z},
        } );

        vao.buffers[3] = std::move(tangent_buffer);
        //vao.buffers[4] = std::move(bitangent_buffer);
        vao.bind();

        print_error();
        
        auto diffuse_texture = glcore::ColorTexture{};
        auto normal_texture = glcore::ColorTexture{};
        auto depth_texture  = glcore::ColorTexture{};

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
            auto parameters3 = std::make_unique<T>(*parameters);
            diffuse_texture.desired_texture_parameters(std::move(parameters));
            normal_texture.desired_texture_parameters(std::move(parameters2));
            depth_texture.desired_texture_parameters(std::move(parameters3));
            print_error();
        }

        diffuse_texture.texture(diffuse_image, true);
        normal_texture.texture(normal_image, true);
        depth_texture.texture(depth_image, true);

        print_error();

        auto srt = SRT{ glm::vec3{2, 2, 2}, glm::vec3{glm::radians(0.0f), 0, 0}, glm::vec3{0, 0, 0} };

        auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };

        auto shader = glcore::Shader{vert_header + read_file("shaders/normal_mapping/parallax_es.vert"), frag_header + read_file("shaders/normal_mapping/parallax_es.frag")};

        auto plane_material = Material{};
        plane_material.ambient = {0.05, 0.05, 0.05, 1.0};
        plane_material.diffuse = {0.0, 0, 0, 1.0};
        plane_material.specular = {0.8, 0.8, 0.8, 1.0};
        plane_material.shineness = 32;

        auto light = PointLight{};
        light.diffuse = {0.7f, 0.7f, 0.7f, 1.0f};
        light.specular = {1, 1, 1, 1};
        light.ambient = {0.1, 0.1, 0.1, 0.1};

        light.constant = 0.04;
        light.linear = 0.5;
        light.quadratic = 0.002;
        light.position  = {0.0f, 0.0f, 3.0f};

        auto&& frame_buffer = glcore::FrameBuffer::get_default();

        print_error();

        window.run([&]()
        {        
            frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            auto delta_time = Time2::delta_time();
            auto model = model_matrix(srt);
            
            auto near_plane = 0.1f;
            auto far_plane  = 200.0f;
            auto [width, height] = window.window_dim();
            auto projection = perspective_projection_matrix( camera.Zoom , static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);
            auto view = camera.GetViewMatrix();


            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection * view * model);
            shader.set_uniform_matrix4fv("model", model);
            apply_material(plane_material, shader);
            apply_light(light, shader);
            shader.set_uniform("material.diffuseTexture", 0);
            shader.set_uniform("material.normalTexture", 1);
            shader.set_uniform("material.depthTexture", 2);
            shader.set_uniform("viewPos", nitros::to_vec3f(camera.Position));
            shader.set_uniform("height_scale", 0.1f);

            diffuse_texture.active_bind(0);
            normal_texture.active_bind(1);
            depth_texture.active_bind(2);

            vao.draw();
        });

    }
    return 0;
}
