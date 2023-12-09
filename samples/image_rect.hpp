

#ifndef NITROS_GLCORE_DEV_AREA_IMAGE_READER_HPP
#define NITROS_GLCORE_DEV_AREA_IMAGE_READER_HPP

#include "image/image.hpp"
#include <gsl/gsl>

namespace nitros::glcore
{
    class ImageRect final
    {
        public:
        using dim_t = std::pair<std::uint32_t, std::uint32_t>;
        ImageRect(utils::ImageCpu  &image_cpu, const dim_t  &offset, const dim_t  &rect_size)
            :_image{image_cpu}
            ,_offset{offset}
            ,_rect_size{rect_size}
        {}

        std::pair<utils::pixel::Format, gsl::span<std::uint8_t>>  at(const dim_t  &pixel) {
            return {_image.meta_data().format, pixel_data(pixel)};
        }

        private:
        auto  pixel_data(const dim_t &pixel) -> gsl::span<std::uint8_t>
        {
            auto ptr = _image.buffer().data();
            const auto y_offset = _offset.second;
            const auto x_offset = _offset.first;

            return gsl::span<std::uint8_t>{ptr  + y_offset * _image.meta_data().step + x_offset * _image.meta_data().format.pixel_layout.bytes 
                                                + pixel.second * _image.meta_data().step + pixel.first * _image.meta_data().format.pixel_layout.bytes , gsl::narrow_cast<std::ptrdiff_t>(_image.meta_data().format.pixel_layout.bytes)};
        }

        utils::ImageCpu&    _image;
        dim_t  _offset;
        dim_t  _rect_size;
    };
} // namespace nitros::imgui

#endif