

#ifndef GLCORE_TEXTURES_H
#define GLCORE_TEXTURES_H

#include <vector>
#include <string>

#include <glcore/glcore_export.h>
#include <glcore/globj.hpp>

#include "image/image.hpp"
#include "utilities/memory/memory.hpp"
#include "utilities/data/vecs.hpp"

#include <map>
#include <any>
#include <variant>
#include <utility>
#include <optional>

namespace nitros::glcore {

enum class [[deprecated]] texture_type
{
    diffuseType, specularType
};

namespace texture
{
    enum class type
    {
        color, depth, stencil, depth_stencil
    };
}

template <texture::type t>
class Texture;

namespace texture
{
    enum class target { texture_2D, cube_map };
    
class GLCORE_EXPORT Parameters
{
    public:
    enum class option
    {
        base_level,
        border_color,
        min_filter,
        mag_filter,
        min_lod,
        max_lod,
        max_level,
        swizzle,
        wrap_s,
        wrap_t, 
        wrap_r
    };

    enum class filter_min_params
    {
        nearest,
        linear,
        nearest_mipmap_nearest,
        linear_mipmap_nearest,
        nearest_mipmap_linear,
        linear_mipmap_linear
    };

    enum class filter_max_params
    {
        nearest,
        linear,
    };

    enum class wrap_params
    {
        clamp_to_edge,
        clamp_to_border,
        mirrored_repeat,
        repeat,
        mirror_clamp_to_edge
    };

    struct swizzle_value
    {
        enum class component { red, green, blue, alpa};

        swizzle_value()
            :red_channel{component::red}
            ,green_channel{component::green}
            ,blue_channel{component::blue}
            ,alpha_channel{component::alpa}
        {}

        void set_rgba(component red_channel, component green_channel, component blue_channel, component alpha_channel){
            
            swizzle_value::red_channel   = red_channel;
            swizzle_value::green_channel = green_channel;
            swizzle_value::blue_channel  = blue_channel;
            swizzle_value::alpha_channel = alpha_channel;
        }

        component red_channel;
        component green_channel;
        component blue_channel;
        component alpha_channel;
    };

    template <option N, typename value_>
    struct Param
    {
        using value_type = value_;
        Param() = default;
        Param(const value_type &v)
            :value{v}
        {}

        value_type value;
        static constexpr option key = N;
    };

    using base_level    = Param<option::base_level, std::uint32_t>;
    using border_color  = Param<option::border_color, utils::vec4f>;
    using min_filter    = Param<option::min_filter, filter_min_params>;
    using mag_filter    = Param<option::mag_filter, filter_max_params>;

    using min_lod       = Param<option::min_lod, float>;
    using max_lod       = Param<option::max_lod, float>;
    using max_level     = Param<option::max_level, std::uint32_t>;
    
    using swizzle       = Param<option::swizzle, swizzle_value>;
    using wrap_s        = Param<option::wrap_s, wrap_params>;
    using wrap_t        = Param<option::wrap_t, wrap_params>;
    using wrap_r        = Param<option::wrap_r, wrap_params>;

    using var_t = std::variant<
                             base_level  
                            ,border_color
                            ,min_filter  
                            ,mag_filter  
                            ,min_lod     
                            ,max_lod     
                            ,max_level   
                            ,swizzle
                            ,wrap_s 
                            ,wrap_t 
                            ,wrap_r>;

    template <option N, typename value>
    void add(const Param<N, value>  &parameter){
        _options[N] = parameter;
    }

    template <option N, typename value, typename ... Args>
    void add(const Param<N, value>  &parameter, Args&& ... args){
        add(parameter);
        add(std::forward<Args>(args)...);
    }

    template <typename p_type>
    [[nodiscard]] auto get() -> std::optional<p_type>{
        constexpr auto N = p_type::key;
        for(auto& [k , p] : _options)
        {
            if(k == N){
                return std::get<p_type>(p);
            }
        }
        return std::nullopt;
    }
    
    private:
    std::map<option, var_t >    _options;
    template <type T> friend class ::nitros::glcore::Texture;
};

    template <type T_>
    class GLCORE_EXPORT ImageView
    {
        public:
        using texture_value = ::nitros::glcore::Texture<T_>;
        constexpr static auto texture_type = T_;

        ImageView(const texture_value &texture, const std::uint32_t  &level, const utils::ImageMetaData  &meta_data);
        ImageView(const ImageView  &);
        ImageView(ImageView &&) = default;
        ~ImageView();

        auto operator=(const ImageView &) -> ImageView&;
        auto operator=(ImageView &&) -> ImageView& = default;

        [[nodiscard]] auto get_id() const noexcept -> std::uint32_t;
        [[nodiscard]] auto get_level() const noexcept -> std::uint32_t;
        [[nodiscard]] auto get_target() const noexcept -> texture::target;
        [[nodiscard]] auto get_metaData() const noexcept -> const utils::ImageMetaData&;

        //Wont work with OpenGL ES. Returns an empty image
        [[nodiscard]] auto read_image() const -> std::vector<utils::Uptr<utils::ImageCpu>>;
        [[nodiscard]] auto read_sub_image(std::uint32_t x_offset, std::uint32_t y_offset, std::uint32_t z_offset, std::uint32_t width, std::uint32_t height, std::uint32_t depth) const -> utils::Uptr<utils::ImageCpu>;

        auto copy_to(ImageView  &dst_image_view) -> bool;
        
        private:
        std::reference_wrapper<const texture_value>   _texture;
        std::uint32_t     _level;
        utils::Uptr<utils::ImageMetaData>    _meta_data;
    };
}

template <texture::type T_ = texture::type::color>
class GLCORE_EXPORT Texture : public GLobj
{
    public:
    using Parameters = texture::Parameters;
    using ImageView = texture::ImageView<T_>;

    constexpr static auto type = T_;

    [[deprecated]] explicit Texture(texture::target  target_ = texture::target::texture_2D, bool mipmap = true);
    explicit Texture(const utils::ImageMetaData  &meta_data, texture::target  target_ = texture::target::cube_map, bool mipmap = true);
    Texture(const Texture&) = delete;
    Texture(Texture &&) = default;
    ~Texture();
    
    Texture&    operator=(const Texture&) = delete;
    Texture&   operator=(Texture &&) = default;
  
    template<typename buffer_type_>
    void texture(const utils::Image<buffer_type_>  &image, bool mipmap = true);

    template<typename buffer_type_>
    void texture_realloc_size(const utils::Image<buffer_type_>  &image, bool mipmap = true);

    //Order of images is Right, Left, Top, Bottom, Front, Back
    template<typename buffer_type_>
    void texture_cube_map(const gsl::span<const utils::Image<buffer_type_>, 6>  &images, bool mipmap = true);

    //Order of images is Right, Left, Top, Bottom, Front, Back
    template<typename buffer_type_>
    void texture_realloc_cube_map(const gsl::span<const utils::Image<buffer_type_>, 6>  &images, bool mipmap = true);
    
    void desired_texture_parameters(utils::Uptr<Parameters>  params);
    [[nodiscard]] auto current_texture_parameters() const -> Parameters;
    
    void bind() const;
    void active_bind(int num = 0) const;

    [[deprecated]] auto get_type() const noexcept -> texture::target;
    auto get_target() const noexcept -> texture::target;

    auto current_mip_levels() const noexcept -> std::uint32_t;

    //Returns empty if the right level is not found
    [[nodiscard]] auto image_view(const std::uint32_t  &level = 0) -> utils::Uptr<ImageView>;

    private:
    void texture_parameters(const Parameters  &params);
    void alloc_storage(const texture::target  &target, const utils::ImageMetaData &meta_data, bool mip_map);
    void copy_data(const std::uint32_t  &level, const utils::vec2Ui  &offset, const utils::vec2Ui  &dim, const utils::pixel::Format  &format, const gsl::span<const std::uint8_t>  &data);
    void copy_data(const std::uint32_t  &level, const utils::vec2Ui  &offset, const utils::vec2Ui  &dim, const utils::pixel::Format  &format, const std::array<gsl::span<const std::uint8_t>, 6>  &data);
    
    texture::target  _target;
    bool            _mip_map;
    utils::Uptr<Parameters>     _params;
    utils::Uptr<utils::ImageMetaData>        _meta_data;
};

using ColorTexture = Texture<texture::type::color>;
using DepthTexture = Texture<texture::type::depth>;
using StencilTexture = Texture<texture::type::stencil>;
using DepthStencilTexture = Texture<texture::type::depth_stencil>;


extern template class GLCORE_EXPORT Texture<texture::type::color>;
extern template class GLCORE_EXPORT Texture<texture::type::depth>;
extern template class GLCORE_EXPORT Texture<texture::type::stencil>;
extern template class GLCORE_EXPORT Texture<texture::type::depth_stencil>;

extern template class GLCORE_EXPORT texture::ImageView<texture::type::color>;
extern template class GLCORE_EXPORT texture::ImageView<texture::type::depth>;
extern template class GLCORE_EXPORT texture::ImageView<texture::type::stencil>;
extern template class GLCORE_EXPORT texture::ImageView<texture::type::depth_stencil>;

extern template GLCORE_EXPORT void ColorTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
extern template GLCORE_EXPORT void DepthTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
extern template GLCORE_EXPORT void StencilTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
extern template GLCORE_EXPORT void DepthStencilTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);

extern template GLCORE_EXPORT void ColorTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
extern template GLCORE_EXPORT void DepthTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
extern template GLCORE_EXPORT void StencilTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
extern template GLCORE_EXPORT void DepthStencilTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);

extern template GLCORE_EXPORT void ColorTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
extern template GLCORE_EXPORT void DepthTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
extern template GLCORE_EXPORT void StencilTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
extern template GLCORE_EXPORT void DepthStencilTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);

extern template GLCORE_EXPORT void ColorTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
extern template GLCORE_EXPORT void DepthTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
extern template GLCORE_EXPORT void StencilTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
extern template GLCORE_EXPORT void DepthStencilTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);

}

#endif // TEXTURES_H
