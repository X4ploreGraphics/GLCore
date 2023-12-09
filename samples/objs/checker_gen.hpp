

#ifndef CHECKER_GEN_HPP
#define CHECKER_GEN_HPP

#include "image/image.hpp"
#include "utilities/data/vecs.hpp"
#include <limits>


namespace nitros
{

auto gen_checker_RGBA8(const nitros::utils::ImgSize  &size, const nitros::utils::vec2Ui  &checker_size){
    using namespace nitros;
    using T = std::uint32_t;

    auto image = utils::image::create_cpu(size, utils::pixel::RGBA8::value);
    auto data = gsl::span<T>(reinterpret_cast<T*>(image.buffer().data()), image.buffer().size() );
    
    auto count = 0u;
    

    auto dim_x = size.width/checker_size[0];
    auto dim_y = size.height/checker_size[1];

    for(auto x = 0; x < size.width; x++)
    {
        for(auto y = 0 ; y < size.height; y++)
        {
            if( (x / checker_size[0]) % 2 == 0 && (y / checker_size[1]) % 2 == 0){
            
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::max();
            }
            else{
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::min();
            }
        }
    }

    return image;
}

auto gen_checker_RGBA16(const nitros::utils::ImgSize  &size, const nitros::utils::vec2Ui  &checker_size){
    using namespace nitros;
    using T = std::uint64_t;

    auto image = utils::image::create_cpu(size, utils::pixel::RGBA16::value);
    auto data = gsl::span<T>(reinterpret_cast<T*>(image.buffer().data()), image.buffer().size() );
    
    for(auto x = 0; x < size.width; x++)
    {
        for(auto y = 0 ; y < size.height; y++)
        {
            if( (x / checker_size[0]) % 2 == 0 && (y / checker_size[1]) % 2 == 0)
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::max();
            else
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::min();
        }
    }

    return image;
}

auto gen_checker_GREY16(const nitros::utils::ImgSize  &size, const nitros::utils::vec2Ui  &checker_size){
    using namespace nitros;
    using T = std::uint16_t;

    auto image = utils::image::create_cpu(size, utils::pixel::GREY16::value);
    auto data = gsl::span<T>(reinterpret_cast<T*>(image.buffer().data()), image.buffer().size() );
    
    for(auto x = 0; x < size.width; x++)
    {
        for(auto y = 0 ; y < size.height; y++)
        {
            if( (x / checker_size[0]) % 2 == 0 && (y / checker_size[1]) % 2 == 0)
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::max();
            else
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::min();
        }
    }

    return image;
}

auto gen_checker_GREY8(const nitros::utils::ImgSize  &size, const nitros::utils::vec2Ui  &checker_size){
    using namespace nitros;
    using T = std::uint8_t;

    auto image = utils::image::create_cpu(size, utils::pixel::GREY8::value);
    auto data = gsl::span<T>(reinterpret_cast<T*>(image.buffer().data()), image.buffer().size() );
    
    for(auto x = 0; x < size.width; x++)
    {
        for(auto y = 0 ; y < size.height; y++)
        {
            if( (x / checker_size[0]) % 2 == 0 && (y / checker_size[1]) % 2 == 0)
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::max();
            else
                data[y * image.meta_data().size.width + x] = std::numeric_limits<T>::min();
        }
    }

    return image;
}

auto gen_checker_R8(const nitros::utils::ImgSize  &size, const nitros::utils::vec2Ui  &checker_size){
    auto image = gen_checker_GREY8(size, checker_size);
    return utils::ImageCpu{utils::ImageMetaData{image.meta_data().size, utils::pixel::R8::value }, std::move(image.buffer())};
}

}
#endif