

#ifndef GLCORE_DEV_AREA_COMMON_HPP
#define GLCORE_DEV_AREA_COMMON_HPP

#include <string_view>
#if defined(__GNUC__) && !defined(__clang__)
#include <experimental/source_location>
#endif
#include <iostream>

#include "glcore/commands.hpp"
#include "image/fileio.hpp"
#include <spdlog/fmt/fmt.h>

#include <algorithm>

namespace nitros
{
    
    #if defined(__GNUC__) && !defined(__clang__)

    inline void print_error(const std::experimental::source_location& location = std::experimental::source_location::current())
    {
        auto errors = nitros::glcore::command::error();
        errors.erase(std::remove(errors.begin(), errors.end(), nitros::glcore::command::error_type::no_error), errors.end());
        if(errors.size() > 0){
            std::cout<< fmt::format("errors {} {}", errors.size(), location.line()).data() <<std::endl;    
        }
    }
    #else
    inline void print_error()
    {
        auto errors = nitros::glcore::command::error();
        errors.erase(std::remove(errors.begin(), errors.end(), nitros::glcore::command::error_type::no_error), errors.end());
        if(errors.size() > 0){
            std::cout<< fmt::format("errors {} ", errors.size()).data() <<std::endl;    
        }
    }
    #endif

    struct DirLight
    {
        using vec3 = nitros::utils::vec3f;
        using vec4 = nitros::utils::vec4f;
        
        vec3    direction;
        vec4    ambient;
        vec4    diffuse;
        vec4    specular;
    };

    struct SpotLight {
        using vec3 = nitros::utils::vec3f;
        using vec4 = nitros::utils::vec4f;

        vec3 position;
        vec3 direction;
        vec4 ambient;
        vec4 diffuse;
        vec4 specular;
        float cutOff;
        float outerCutOff;
    };

    struct PointLight {
        using vec3 = nitros::utils::vec3f;
        using vec4 = nitros::utils::vec4f;

        vec3 position;
        vec4 ambient;
        vec4 diffuse;
        vec4 specular;

        float constant;
        float linear;
        float quadratic;
    };

    struct Material
    {
        public:
        nitros::utils::vec4f    ambient;
        nitros::utils::vec4f    diffuse;
        nitros::utils::vec4f    specular;      
        float   shineness;
    };

    auto to_vec3f(const glm::vec3 &vec){
        return nitros::utils::vec3f{ vec.x, vec.y, vec.z };
    }

    auto to_vec4f(const glm::vec4 &vec){
        return nitros::utils::vec4f{ vec.x, vec.y, vec.z, vec.w };
    }

    void write_cubemap(const nitros::glcore::ColorTexture::ImageView &cubemap, const std::string &prefix){
        auto images = cubemap.read_image();
        std::cout<<"Color View"<<std::endl;
        for(auto i = 0 ; i < images.size(); i++)
        {
            nitros::utils::image::write_image_png(fmt::format("{}_{}.png", prefix, i), *(images[i]) );
        }
    }

    void write_cubemap(const nitros::glcore::DepthTexture::ImageView &cubemap, const std::string &prefix){
        using namespace nitros;
        auto images = cubemap.read_image();
        std::cout<<"Depth View"<<std::endl;
    
        if(images.size() > 0)
        {
            if(images.at(0)->meta_data().format.pixel_layout.bytes == 4 &&  images.at(0)->meta_data().format.pixel_layout.normalized)
            {
                std::cout<<"Grey 32"<<std::endl;
                auto i = 0;
            
                for(auto&& img : images)
                {
                    auto grey_image = utils::image::create_cpu(cubemap.get_metaData().size, utils::pixel::GREY8::value );

                    auto sp8  = gsl::span<std::uint8_t>{grey_image.buffer().data(), gsl::narrow_cast<std::ptrdiff_t>( grey_image.buffer().size() ) };
                    auto sp32 = gsl::span<float>{ reinterpret_cast<float*>( img->buffer().data() ), gsl::narrow_cast<std::ptrdiff_t>( img->buffer().size()/4 ) };

                    auto it_8  = sp8.begin();
                    auto it_32 = sp32.begin();

                    for( ; it_8 < sp8.end() && it_32 < sp32.end()  ; it_8++, it_32++) {
                        *it_8 = *it_32 * 255.f;
                    }

                    utils::image::write_image_png(fmt::format("{}_{}.png", prefix, i), grey_image );
                    i++;
                }
            }
            else
            {
                for(auto i = 0 ; i < images.size(); i++) {
                    nitros::utils::image::write_image_png(fmt::format("{}_{}.png", prefix, i), *(images[i]) );
                }
            }
        }
    }
} // namespace nitros


#endif