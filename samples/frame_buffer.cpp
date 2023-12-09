

#include "window_wrapper.hpp"
#include "shader_header.hpp"

#include "objs/creator.hpp"
#include "transforms/transform.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"
#include "glcore/rasterizer.hpp"
#include "glcore/common_processing.hpp"
#include "glcore/rasterizer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <generator/PlaneMesh.hpp>
#include <generator/AnyGenerator.hpp>

#include "utilities/files/text_reader.hpp"
#include "utilities/files/files.hpp"
#include "image/fileio.hpp"

#include "common.hpp"

int main(int argc, char const *argv[])
{
    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;

    if(argc < 2){
        std::cout<<"Exe <HeightMap> <Diffuse Map> else default"<<std::endl;
    }

    //auto check_image = gen_checker_GREY8({300, 300}, {50, 50});

    auto height_image = (argc > 1)? utils::image::read_image(argv[1]) : gen_checker_GREY16({300, 300}, {50, 50});
    auto color_image = (argc > 2)? utils::image::read_image(argv[2]) : gen_checker_RGBA8({300, 300}, {50, 50});

#if !defined(__EMSCRIPTEN__)
    glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();
        auto plane = creator.indexed_plane({1.0, 1.0}, {100, 100});

        auto vao = glcore::VertexArray{};
        vao.buffers[0] = std::move(plane.vertices);
        vao.buffers[1] = std::move(plane.tex_coords);
        vao.index = std::move(plane.indices);
        vao.bind();
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

        auto plane2 = creator.indexed_plane( {0.7, 0.7}, {1, 1} );

    #if !defined(__EMSCRIPTEN__)
        auto tex = std::vector<std::array<float, 2>>{};
        plane2.tex_coords->read_data(tex);
        for(auto & pt : tex)
        {
            std::cout<<pt[0]<<','<<pt[1]<<std::endl;
        }
    #endif
        
        auto plane_vao = glcore::VertexArray{};
        plane_vao.buffers[0] = std::move(plane2.vertices);
        plane_vao.buffers[1] = std::move(plane2.tex_coords);
        plane_vao.index = std::move(plane2.indices);
        plane_vao.bind();
        plane_vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

    
        auto height_texture = glcore::DepthTexture{height_image.meta_data(), glcore::texture::target::texture_2D};
        height_texture.texture(height_image, false);
        print_error();

        auto color_texture = glcore::ColorTexture{color_image.meta_data(), glcore::texture::target::texture_2D};
        color_texture.texture(color_image);
        print_error();

        auto [width, height] = window.window_dim();
        

        auto _fovy = 45.0f;
        auto near_plane = 0.1f;
        auto far_plane  = 200.0f;
        auto projection = perspective_projection_matrix(_fovy, static_cast<float>(width), static_cast<float>(height), near_plane, far_plane);
        auto small_projection = perspective_projection_matrix(_fovy, static_cast<float>(300), static_cast<float>(300), near_plane, far_plane);

        auto srt = SRT{ glm::vec3{1,1,1}, glm::vec3{glm::radians(-45.f), 0, 0}, glm::vec3{0, 0, -4} };

        auto read_file = [](const std::string &path){
            auto str1 = std::string{};
            auto vert = utils::reader::read_binary(path);
            str1.append(vert.begin(), vert.end());
            return str1;
        };

        auto shader  = glcore::Shader{ vert_header + read_file("shaders/height_map.vert"), frag_header + read_file("shaders/height_map.frag") };
        auto shader2 = glcore::Shader{ vert_header + read_file("shaders/simple_tex.vert"), frag_header + read_file("shaders/simple_tex.frag") };

        print_error();

        glcore::Rasterizer::get_instance().enable(glcore::Rasterizer::capability::depth_test);

        auto copy_texture = glcore::Texture{utils::ImageMetaData{{300, 300}, utils::pixel::RGBA8::value}, glcore::texture::target::texture_2D, false};
        auto copy_depth_texture = glcore::DepthStencilTexture{utils::ImageMetaData{ {300, 300}, utils::pixel::GREY_STENCIL_24_8::value }, glcore::texture::target::texture_2D, false};
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
        first_pass_dim.dimension = {300, 300};

        auto default_pass_dim = glcore::ViewDim{};
        default_pass_dim.left_bottom = {0, 0};
        default_pass_dim.dimension = {width, height};

        auto&& frame_buffer = glcore::FrameBuffer::get_default();

        window.run([&]()
        {    
            //First
            glcore::ViewPort::dimension(first_pass_dim);

            fbo2.bind();
            fbo2.clear_color({0.2f, 0.2, 0.2, 0.0f});
            fbo2.clear_depth();

            print_error();

            auto delta_time = Time2::delta_time();

            srt.rotate.y += (glm::radians(45.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);

            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection *  model);
            shader.set_uniform("height_map", utils::vec1i{0});
            shader.set_uniform("diffuse_map", utils::vec1i{1});
            height_texture.active_bind(0);
            color_texture.active_bind(1);
            vao.draw();

            //Second Pass

            glcore::ViewPort::dimension(default_pass_dim);

            frame_buffer.bind();
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});
            frame_buffer.clear_depth();

            print_error();

            shader.use();
            shader.set_uniform_matrix4fv("mvp", projection *  model);
            shader.set_uniform("height_map", utils::vec1i{0});
            shader.set_uniform("diffuse_map", utils::vec1i{1});
            height_texture.active_bind(0);
            color_texture.active_bind(1);
            vao.draw();

            shader2.use();
            shader2.set_uniform("diffuse_map", utils::vec1i{0});
            shader2.set_uniform_matrix4fv("mvp", projection * model);
            //copy_texture.active_bind(0);
            copy_depth_texture.active_bind(0);
            plane_vao.draw();

        });

        auto image_vec = copy_texture.image_view()->read_image();
        std::cout<<image_vec.size()<<std::endl;

        auto&& image_t = image_vec.at(0);
        utils::image::write_image_png("AImage.png", *image_t);
    }
    return 0;
}
