

#include "window_wrapper.hpp"

#include "objs/creator.hpp"
#include "transforms/transform.hpp"

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

#include "common.hpp"

int main(int argc, char const *argv[])
{
    auto window = Window{};

    using namespace nitros;
    using namespace std::string_literals;


    #if !defined(__EMSCRIPTEN__)
glcore::load_context(glfwGetProcAddress);
#endif
    {

        auto f_p_dim = utils::vec2Ui{800, 800};
        auto col_texture = glcore::ColorTexture{utils::ImageMetaData{{f_p_dim[0], f_p_dim[1]}, utils::pixel::RGBA8::value}, glcore::texture::target::texture_2D , false};
        auto depth_texture = glcore::DepthTexture{utils::ImageMetaData{{f_p_dim[0], f_p_dim[1]}, utils::pixel::GREY32f::value}, glcore::texture::target::texture_2D , false};

        print_error();

        //Depth CubeMap Texture
        auto cubemap_depth = glcore::DepthTexture{utils::ImageMetaData{ {f_p_dim[0], f_p_dim[1]}, utils::pixel::GREY32f::value }, glcore::texture::target::cube_map, false };
        auto cubemap_color = glcore::ColorTexture{utils::ImageMetaData{ {f_p_dim[0], f_p_dim[1]}, utils::pixel::RGBA8::value }, glcore::texture::target::cube_map, false };
        //auto cubemap_color = glcore::ColorTexture{glcore::texture::target::cube_map, false};

        print_error();

        /*{
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

            cubemap_color.desired_texture_parameters(std::move(parameters));
            print_error();
        }
        
        auto img_create = [](){
            return utils::image::create_cpu({800, 600}, utils::pixel::RGBA8::value);
        };
        
        auto images = std::array<utils::ImageCpu, 6>{
            img_create(),
            img_create(),
            img_create(),
            img_create(),
            img_create(),
            img_create()
        };

        auto sp_image = gsl::span<const utils::ImageCpu , 6>{ &(*images.begin()), 6 };*/
        //cubemap_color.texture_cube_map(sp_image, false);

        print_error();
        
        //params.add(glcore::texture::Parameters::)
        //copy_texture.texture(color_image);

        auto attachment = std::make_unique<glcore::framebuffer::Attachment>();
        attachment->color_views.push_back(cubemap_color.image_view());
        //attachment->depth_view = std::make_unique< glcore::framebuffer::Attachment::DepthView >(std::make_shared<glcore::DepthRenderBuffer>(utils::ImageMetaData{{300, 300}, utils::pixel::GREY32f::value}));
        attachment->depth_view = std::make_unique< glcore::framebuffer::Attachment::DepthView >(cubemap_depth.image_view());
        auto fbo2 = glcore::FrameBuffer{std::move(attachment)};
        
        print_error();

        //window.run([&]()
        //{    
        //
        //});
    }
    return 0;
}
