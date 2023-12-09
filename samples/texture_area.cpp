

#include "window_wrapper.hpp"

#include "glcore/framebuffer.hpp"
#include "glcore/context.hpp"
#include "glcore/shader.h"
#include "glcore/vertexarray.hpp"
#include "glcore/commands.hpp"

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
    if(argc < 2){
        std::cout<<"Usage Exe <image>"<<std::endl;
        return 0;
    }

    //auto image = nitros::utils::image::read_image(argv[1]);
    auto image = nitros::gen_checker_GREY16({300, 300}, {50u, 50u});

    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;

    

    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {
        auto creator = Creator::get_instance();
        auto plane = creator.indexed_plane();

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
        vao.buffers[0] = std::move(plane.vertices);
        vao.buffers[1] = tex;
        vao.index = std::move(plane.indices);
        vao.bind();
        vao.set_draw_mode(glcore::VertexArray::draw_mode::triangles);

        print_error();
        
        auto texture = glcore::DepthTexture{};
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
                            ,T::wrap_s{w_p::repeat}
                            ,T::wrap_t{w_p::repeat}
                            ,T::swizzle{s_v});

            texture.desired_texture_parameters(std::move(parameters));
            print_error();
        }

        
        
        texture.texture(image, true);

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

        const auto vertex_shader1 =  R"( #version 450 core
                                    layout (location = 0)  in vec3 position;
                                    layout (location = 1)  in vec2 vtex_coords;
                                    out vec2 VTexCoords;
                                    void main(void)
                                    {                
                                       gl_Position = vec4(position, 1.0);  
                                       VTexCoords = vtex_coords;
                                    } )";

        const auto frag_shader1 =   R"( #version 450 core 
                                    in vec2 VTexCoords;
                                    out vec4 color;     
                                    uniform sampler2D texture_diffuse0;
                                    void main(void) 
                                    {  
                                        color = vec4(texture(texture_diffuse0, VTexCoords));
                                    } )";

        const auto frag_shader2 =   R"( #version 450 core 
                                    in vec2 VTexCoords;
                                    out vec4 color;     
                                    uniform sampler2D texture_diffuse0;
                                    void main(void) 
                                    {  
                                        color = vec4(texture(texture_diffuse0, VTexCoords).r);
                                    } )";

        auto shader = glcore::Shader{vertex_shader1, frag_shader2};

        print_error();
    
        window.run([&]()
        {    
            auto&& frame_buffer = glcore::FrameBuffer::get_default();
            frame_buffer.clear({ glcore::FrameBuffer::bitFields::color, glcore::FrameBuffer::bitFields::depth });
            frame_buffer.clear_color({0.4f, 0.4, 0.4, 0.0f});

            auto delta_time = Time2::delta_time();

            srt.rotate.y += (glm::radians(90.f)*delta_time.count()/1000.0);
            auto model = model_matrix(srt);

            shader.use();
            shader.set_uniform("texture_diffuse0", utils::vec1i{0});
            texture.active_bind();
            vao.draw();
        });

    }
    return 0;
}
