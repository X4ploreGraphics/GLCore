

#include "window_wrapper.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"
#include "glcore/commands.hpp"
#include "glcore/rasterizer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "objs/creator.hpp"
#include "transforms/transform.hpp"

#include "image/fileio.hpp"

#include "common.hpp"
#include <iostream>


int main(int argc, char const *argv[])
{
    if(argc < 7){
        std::cout<<"Usage Exe <right> <left> <top> <bottom> <front> <back>"<<std::endl;
        return 0;
    }

    //auto image = nitros::utils::image::read_image(argv[1]);

    auto paths = std::array<std::string, 6>{ argv[1], argv[2], argv[3], argv[4], argv[5], argv[6] };
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
        vao.bind();
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

        print_error();
        
        auto texture = glcore::ColorTexture{ utils::ImageMetaData{ {2048, 2048}, utils::pixel::RGBA8::value } ,glcore::texture::target::cube_map};
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

        auto srt = SRT{ glm::vec3{1,1,1}, glm::vec3{0, 0, 0}, glm::vec3{0, 0, -3} };

        const auto vertex_shader3 =  R"( #version 450 core
                                    layout (location = 0)  in vec3 position;

                                    out vec3 VTexCoords;

                                    uniform mat4 projection;
                                    uniform mat4 view;

                                    void main(void)
                                    {
                                       vec4 pos = projection * view * vec4(position, 1.0);            
                                       gl_Position = pos.xyww ;
                                       VTexCoords = position;
                                    } )";

        const auto frag_shader3 =   R"( #version 450 core 
                                    in vec3 VTexCoords;
                                    out vec4 color;     
                                    uniform samplerCube cubemap;
                                    void main(void) 
                                    {  
                                        color = texture(cubemap, VTexCoords);
                                    } )";

        //auto shader = glcore::Shader{vertex_shader1, frag_shader2};
        auto shader = glcore::Shader{vertex_shader3, frag_shader3};

        print_error();

        //glcore::Rasterizer::get_instance().disable(glcore::Rasterizer::capability::depth_test);

        auto rotate_vec = glm::vec3{0, 0, 0};
        auto cam_view_mat = glm::lookAt(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0.01}, glm::vec3{0, 1, 0});
    
        window.run([&]()
        {    
            auto&& frame_buffer = glcore::FrameBuffer::get_default();
            frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            frame_buffer.depth_mask(false);

            auto delta_time = Time2::delta_time();

            srt.rotate.y += (glm::radians(90.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);

            shader.use();
            //shader.set_uniform("texture_diffuse0", utils::vec1i{0});
            auto delta = 4 * glm::radians(5.0f) * Time2::delta_time().count() / 1000;
            rotate_vec.y += delta;
            //rotate_vec.x += delta;

            auto rotate_mat = glm::mat4_cast( glm::quat{rotate_vec} );

            shader.set_uniform_matrix4fv("projection", projection * cam_view_mat * rotate_mat);
            shader.set_uniform_matrix4fv("view", glm::mat4{1.0});
            shader.set_uniform("cubemap", 0);
            texture.active_bind();
            vao.draw();
        });

        auto image_vec = texture.image_view()->read_image();
        std::cout<<image_vec.size()<<std::endl;

        for(auto i = 0 ; i < 6 ; i++){
            auto&& image_t = image_vec.at(i);
            utils::image::write_image_png(fmt::format("CImage{}.png",i), *image_t);
        }
        
    }
    return 0;
}
