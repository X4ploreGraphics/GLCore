

#include "glcore/textures.h"
#include "platform/gl.hpp"
#include "utils/utils.hpp"
#include "logger.hpp"
#include <cstring>
#include <cmath>
#include "glcore/commands.hpp"
#include <iostream>

namespace nitros::glcore
{
    constexpr auto to_glType(texture::Parameters::filter_min_params  param)
    {
        using f_params = texture::Parameters::filter_min_params;
        switch (param)
        {
            case f_params::linear : return GL_LINEAR;
            case f_params::linear_mipmap_linear  : return GL_LINEAR_MIPMAP_LINEAR;
            case f_params::linear_mipmap_nearest : return GL_LINEAR_MIPMAP_NEAREST;
            
            case f_params::nearest : return GL_NEAREST;
            case f_params::nearest_mipmap_linear : return GL_NEAREST_MIPMAP_LINEAR;
            case f_params::nearest_mipmap_nearest : return GL_NEAREST_MIPMAP_NEAREST;
            
        default:
            return GL_LINEAR;
        }
    };

    constexpr auto to_glType(texture::Parameters::filter_max_params  param)
    {
        using f_params = texture::Parameters::filter_max_params;
        switch (param)
        {
            case f_params::linear : return GL_LINEAR;
            case f_params::nearest : return GL_NEAREST;
            
        default:
            return GL_LINEAR;
        }
    };

    constexpr auto to_glType(texture::Parameters::wrap_params  param)
    {
        using w_params = texture::Parameters::wrap_params;
        switch (param)
        {
        #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
            case w_params::clamp_to_border : return GL_CLAMP_TO_BORDER;
        #else
            case w_params::clamp_to_border : 
                    log::Logger()->warn("CLAMP TO BORDER not supported under OpenGL ES 3.2\nResolving to Clamp to Edge");
                    return GL_CLAMP_TO_EDGE;
        #endif
            case w_params::clamp_to_edge   : return GL_CLAMP_TO_EDGE;
            case w_params::repeat : return GL_REPEAT;

        #if OPENGL_CORE >= 40500
            case w_params::mirror_clamp_to_edge : return GL_MIRROR_CLAMP_TO_EDGE;
        #else
            case w_params::mirror_clamp_to_edge : 
                    log::Logger()->warn("MIRRORED CLAMP TO EDGE not supported in OpenGL ES\nResolving to Clamp to Edge");
                    return GL_CLAMP_TO_EDGE;
        #endif
            case w_params::mirrored_repeat      : return GL_MIRRORED_REPEAT;
            
        default:
            return GL_REPEAT;
        }
    };

    constexpr auto to_glType(texture::Parameters::swizzle_value::component  comp)
    {
        using w_comp = texture::Parameters::swizzle_value::component;
        switch (comp)
        {
            case w_comp::red   : return GL_RED;
            case w_comp::green : return GL_GREEN;
            case w_comp::blue  : return GL_BLUE;
            case w_comp::alpa  : return GL_ALPHA;
            
        default:
            return GL_RED;
        }
    };

    constexpr auto to_filter_min_params(const std::int32_t &symbolic_consant) -> texture::Parameters::filter_min_params
    {
        using f_params = texture::Parameters::filter_min_params;
        switch (symbolic_consant)
        {
            case GL_LINEAR : return f_params::linear;
            case GL_LINEAR_MIPMAP_LINEAR : return f_params::linear_mipmap_linear;
            case GL_LINEAR_MIPMAP_NEAREST: return f_params::linear_mipmap_nearest;
            
            case GL_NEAREST :  return f_params::nearest;
            case GL_NEAREST_MIPMAP_LINEAR : return f_params::nearest_mipmap_linear;
            case GL_NEAREST_MIPMAP_NEAREST: return f_params::nearest_mipmap_nearest;
            
        default:
            return f_params::linear;
        }
    }

    constexpr auto to_filter_max_params(const std::int32_t &symbolic_consant) -> texture::Parameters::filter_max_params
    {
        using f_params = texture::Parameters::filter_max_params;
        switch (symbolic_consant)
        {
            case GL_LINEAR : return f_params::linear;
            case GL_NEAREST :  return f_params::nearest;
            
        default:
            return f_params::linear;
        }
    }

    constexpr auto to_wrap_params(const std::int32_t  &symbolic_constant) -> texture::Parameters::wrap_params
    {
        using w_params = texture::Parameters::wrap_params;
        switch (symbolic_constant)
        {
        #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
            case GL_CLAMP_TO_BORDER : return w_params::clamp_to_border;
        #else
            // Case not Supported yet
        #endif
            case GL_CLAMP_TO_EDGE   : return w_params::clamp_to_edge;
            case GL_REPEAT : return w_params::repeat;
        
        #if OPENGL_CORE >= 40500
            case GL_MIRROR_CLAMP_TO_EDGE: return w_params::mirror_clamp_to_edge ;
        #else
            //Case Not Supported yet
        #endif
            case GL_MIRRORED_REPEAT : return w_params::mirrored_repeat;
            
        default:
            return w_params::repeat;
        }
    };

    constexpr auto to_swizzle(const std::int32_t  &comp) -> texture::Parameters::swizzle_value::component
    {
        using w_comp = texture::Parameters::swizzle_value::component;
        switch (comp)
        {
            case GL_RED   : return w_comp::red  ; 
            case GL_GREEN : return w_comp::green; 
            case GL_BLUE  : return w_comp::blue ; 
            case GL_ALPHA : return w_comp::alpa ; 
            
        default:
            return w_comp::red;
        }
    };

    template <class T>
    struct always_false : std::false_type {};

    template <texture::type T_>
    Texture<T_>::Texture(texture::target  target_, bool mipmap)
        :_target{target_}
        ,_mip_map{mipmap}
        ,_params{std::make_unique<Parameters>()}
        ,_meta_data{ []() -> utils::Uptr<utils::ImageMetaData> {
                if constexpr( T_ == texture::type::color ) {
                    return std::make_unique<utils::ImageMetaData>( utils::ImgSize{0, 0}, utils::pixel::RGBA8::value );
                }
                else if constexpr( T_ == texture::type::depth ) {
                    return std::make_unique<utils::ImageMetaData>( utils::ImgSize{0, 0}, utils::pixel::GREY16::value );
                }
                else if constexpr( T_ == texture::type::depth_stencil ) {
                    return std::make_unique<utils::ImageMetaData>( utils::ImgSize{0, 0}, utils::pixel::GREY_STENCIL_24_8::value );
                }
                //Only Stencil Case
                else {
                    return std::make_unique<utils::ImageMetaData>( utils::ImgSize{0, 0}, utils::pixel::STENCIL8::value );
                }
            }()}
    {
    #if OPENGL_CORE >= 40500
        glCreateTextures(to_glType(_target), 1, &_id);
    #else
        glGenTextures(1, &_id);
    #endif

        using f_min = Parameters::filter_min_params;
        using f_max = Parameters::filter_max_params;
        using w_params = Parameters::wrap_params;
        
        _params->add(
            Parameters::min_filter{f_min::nearest_mipmap_linear},
            Parameters::mag_filter{f_max::linear}, 
            Parameters::wrap_s{w_params::repeat} ,
            Parameters::wrap_t{w_params::repeat});

        if(!_mip_map)
        {
            if(auto filter = _params->get<Parameters::min_filter>()) {
                if(filter->value != f_min::linear && filter->value != f_min::nearest)
                    _params->add( Parameters::min_filter{f_min::linear} );
            }
        }
        texture_parameters(*_params);
    }

    template <texture::type T_>
    Texture<T_>::Texture(const utils::ImageMetaData  &meta_data, texture::target  target_, bool mipmap)
        :_target{target_}
        ,_mip_map{mipmap}
        ,_params{std::make_unique<Parameters>()}
        ,_meta_data{std::make_unique<utils::ImageMetaData>(meta_data)}
    {
    #if OPENGL_CORE >= 40500
        glCreateTextures(to_glType(_target), 1, &_id);
    #else
        glGenTextures(1, &_id);
    #endif

        using f_min = Parameters::filter_min_params;
        using f_max = Parameters::filter_max_params;
        using w_params = Parameters::wrap_params;
        
        _params->add(
            Parameters::min_filter{f_min::nearest_mipmap_linear},
            Parameters::mag_filter{f_max::linear}, 
            Parameters::wrap_s{w_params::repeat} ,
            Parameters::wrap_t{w_params::repeat});

        if(!_mip_map)
        {
            if(auto filter = _params->get<Parameters::min_filter>()) {
                if(filter->value != f_min::linear && filter->value != f_min::nearest)
                    _params->add( Parameters::min_filter{f_min::linear} );
            }
        }
        texture_parameters(*_params);
        alloc_storage(_target, *_meta_data, _mip_map);
    }

    template <texture::type T_>
    Texture<T_>::~Texture()
    {
        glDeleteTextures(1, &_id);
    }

    template <texture::type T_>
    template<typename buffer_type_>
    void Texture<T_>::texture(const utils::Image<buffer_type_>  &image, bool mipmap)
    {
        if(_target == texture::target::cube_map){
            log::Logger()->error("Texture is Cube Map, wrong function call");
            return;
        }

        auto [width, height] = image.meta_data().size;
        auto levels = std::log2(std::max(width, height));
        //Mip Map status change
        if(_mip_map != mipmap)
        {
            using f_params = Parameters::filter_min_params;
            auto params = Parameters{};

            if(!mipmap)
            {
                if(auto filter = _params->get<Parameters::min_filter>()) {
                    if(filter->value != f_params::linear && filter->value != f_params::nearest)
                        params.add( Parameters::min_filter{f_params::linear} );
                }
            }
            else
            {
                if(auto filter = _params->get<Parameters::min_filter>()) {
                    params.add( Parameters::min_filter{filter->value} );
                }
            }
            
            texture_parameters(params);
            _mip_map = mipmap;
        }

        if(_meta_data->size != image.meta_data().size){
            _meta_data = std::make_unique<utils::ImageMetaData>(image.meta_data().size, _meta_data->format);
            alloc_storage(texture::target::texture_2D, *_meta_data, _mip_map);
        }

        auto dim = utils::vec2Ui{
            gsl::narrow_cast<std::uint32_t>(width),
            gsl::narrow_cast<std::uint32_t>(height)
        };

        copy_data( 0, utils::vec2Ui{0, 0}, dim, image.meta_data().format, image.buffer() );
    
        if(mipmap){
    #if OPENGL_CORE >= 40500
            glGenerateTextureMipmap(_id);
    #else
            glGenerateMipmap(to_glType(_target));
    #endif
        }

    #if defined(OPENGL_ES)
        // OpenGL ES doesn't support bgr formats
        if(image.meta_data().format.pixel_type == utils::pixel::type::bgr || image.meta_data().format.pixel_type == utils::pixel::type::bgra)
        {
            auto params = Parameters{};
            auto s_v = Parameters::swizzle_value{};
            s_v.red_channel  = Parameters::swizzle_value::component::blue;
            s_v.blue_channel = Parameters::swizzle_value::component::red;
            params.add(Parameters::swizzle{s_v});
            texture_parameters(params);
        }
    #endif
    }

    template <texture::type T_>
    template<typename buffer_type_>
    void Texture<T_>::texture_realloc_size(const utils::Image<buffer_type_>  &image, bool mipmap)
    {
        if (_meta_data->size == image.meta_data().size) {
            texture(image, mipmap);
            return ;
        }

        _meta_data = std::make_unique<utils::ImageMetaData>(image.meta_data().size, _meta_data->format);
        alloc_storage(texture::target::texture_2D, *_meta_data, _mip_map);

        auto [width, height] = _meta_data->size;

        auto dim = utils::vec2Ui{
            gsl::narrow_cast<std::uint32_t>(width),
            gsl::narrow_cast<std::uint32_t>(height)
        };

        copy_data( 0, utils::vec2Ui{0, 0}, dim, image.meta_data().format, image.buffer() );
    
        if(mipmap){
    #if OPENGL_CORE >= 40500
            glGenerateTextureMipmap(_id);
    #else
            glGenerateMipmap(to_glType(_target));
    #endif
        }

    #if defined(OPENGL_ES)
        // OpenGL ES doesn't support bgr formats
        if(image.meta_data().format.pixel_type == utils::pixel::type::bgr || image.meta_data().format.pixel_type == utils::pixel::type::bgra)
        {
            auto params = Parameters{};
            auto s_v = Parameters::swizzle_value{};
            s_v.red_channel  = Parameters::swizzle_value::component::blue;
            s_v.blue_channel = Parameters::swizzle_value::component::red;
            params.add(Parameters::swizzle{s_v});
            texture_parameters(params);
        }
    #endif
    }

    template <texture::type T_>
    template<typename buffer_type_>
    void Texture<T_>::texture_cube_map(const gsl::span<const utils::Image<buffer_type_>, 6>  &images, bool mipmap )
    {
        if(_target != texture::target::cube_map){
            log::Logger()->error("Texture is not Cube Map, wrong function call");
            return;
        }

        const auto meta_data = images[0].meta_data();
        auto iter = std::find_if_not(images.begin(), images.end(), [meta_data](const utils::Image<buffer_type_>  &img){
            return ( img.meta_data().size == meta_data.size ) && img.meta_data().format == meta_data.format;
        });

        if(iter != images.end()){
            log::Logger()->error("Cube Images meta Data did not Match !!!");
            return ;
        }

        auto [width, height] = meta_data.size;
        auto levels = std::log2(std::max(width, height));
        //Mip Map status change
        if(_mip_map != mipmap)
        {
            using f_params = Parameters::filter_min_params;
            auto params = Parameters{};

            if(!mipmap)
            {
                if(auto filter = _params->get<Parameters::min_filter>()) {
                    if(filter->value != f_params::linear && filter->value != f_params::nearest)
                        params.add( Parameters::min_filter{f_params::linear} );
                }
            }
            else
            {
                if(auto filter = _params->get<Parameters::min_filter>()) {
                    params.add( Parameters::min_filter{filter->value} );
                }
            }
            
            texture_parameters(params);
            _mip_map = mipmap;
        }

        if(_meta_data->size != meta_data.size){
            _meta_data = std::make_unique<utils::ImageMetaData>(meta_data.size, _meta_data->format);
            alloc_storage(texture::target::cube_map, *_meta_data, _mip_map);
        }

        auto dim = utils::vec2Ui{
            gsl::narrow_cast<std::uint32_t>(width),
            gsl::narrow_cast<std::uint32_t>(height)
        };

        auto images_arr = std::array< gsl::span<const std::uint8_t> ,6>{
            images[0].buffer(), images[1].buffer(), 
            images[2].buffer(), images[3].buffer(), 
            images[4].buffer(), images[5].buffer(), 
        };

        copy_data( 0, utils::vec2Ui{0, 0}, dim, meta_data.format, images_arr );

        if(mipmap){
    #if OPENGL_CORE >= 40500
            glGenerateTextureMipmap(_id);
    #else
            glGenerateMipmap(to_glType(_target));
    #endif
        }

    #if defined(OPENGL_ES)
        // OpenGL ES doesn't support bgr formats
        if(meta_data.format.pixel_type == utils::pixel::type::bgr || meta_data.format.pixel_type == utils::pixel::type::bgra)
        {
            auto params = Parameters{};
            auto s_v = Parameters::swizzle_value{};
            s_v.red_channel  = Parameters::swizzle_value::component::blue;
            s_v.blue_channel = Parameters::swizzle_value::component::red;
            params.add(Parameters::swizzle{s_v});
            texture_parameters(params);
        }
    #endif
    }

    template <texture::type T_>
    template<typename buffer_type_>
    void Texture<T_>::texture_realloc_cube_map(const gsl::span<const utils::Image<buffer_type_>, 6>  &images, bool mipmap)
    {
        if(_target != texture::target::cube_map) {
            LOG_E("Texture is not Cube Map, wrong function call");
            return ;
        }

        if(_meta_data->size == images[0].meta_data().size ) {
            texture_cube_map(images, mipmap);
            return ;
        }

        auto&& meta_data = images[0].meta_data();

        _meta_data = std::make_unique<utils::ImageMetaData>(meta_data.size, _meta_data->format);
        alloc_storage(texture::target::cube_map, *_meta_data, _mip_map);

        auto [width, height] = _meta_data->size;

        auto dim = utils::vec2Ui{
            gsl::narrow_cast<std::uint32_t>(width),
            gsl::narrow_cast<std::uint32_t>(height)
        };

        auto images_arr = std::array< gsl::span<const std::uint8_t> ,6>{
            images[0].buffer(), images[1].buffer(), 
            images[2].buffer(), images[3].buffer(), 
            images[4].buffer(), images[5].buffer(), 
        };

        copy_data( 0, utils::vec2Ui{0, 0}, dim, meta_data.format, images_arr );

        if(mipmap){
    #if OPENGL_CORE >= 40500
            glGenerateTextureMipmap(_id);
    #else
            glGenerateMipmap(to_glType(_target));
    #endif
        }

    #if defined(OPENGL_ES)
        // OpenGL ES doesn't support bgr formats
        if(meta_data.format.pixel_type == utils::pixel::type::bgr || meta_data.format.pixel_type == utils::pixel::type::bgra)
        {
            auto params = Parameters{};
            auto s_v = Parameters::swizzle_value{};
            s_v.red_channel  = Parameters::swizzle_value::component::blue;
            s_v.blue_channel = Parameters::swizzle_value::component::red;
            params.add(Parameters::swizzle{s_v});
            texture_parameters(params);
        }
    #endif
    }
    
    template <texture::type T_>
    void Texture<T_>::alloc_storage(const texture::target  &target, const utils::ImageMetaData &meta_data, bool mip_map)
    {
        auto [width, height] = meta_data.size;
        auto levels = static_cast<std::size_t>( mip_map ? std::log2(std::max(width, height)) : 1);

        if(target == texture::target::cube_map && meta_data.size.height != meta_data.size.width){
            log::Logger()->error("Cube Map Alloc Storage Height != Width {} {}", meta_data.size.height, meta_data.size.width);
            throw std::runtime_error("Cube Map Storage Allocation");
        }

    #if OPENGL_CORE >= 40500
        glTextureStorage2D(_id, levels, to_internal_glFormat<type>(meta_data.format), width, height);
        _meta_data = std::make_unique<utils::ImageMetaData>(meta_data);
    #else
        glBindTexture(to_glType(_target), _id);
        glTexStorage2D(to_glType(_target), levels, to_internal_glFormat<type>(meta_data.format), width, height);
        _meta_data = std::make_unique<utils::ImageMetaData>(meta_data);
    #endif
        log::Logger()->debug("Allocating Texture Storage {} X {}",width, height);
    }

    template <texture::type T_>
    void Texture<T_>::copy_data(const std::uint32_t  &level, const utils::vec2Ui  &offset, const utils::vec2Ui  &dim, const utils::pixel::Format  &format, const gsl::span<const std::uint8_t>  &data)
    {
    #if OPENGL_CORE >= 40500
        glTextureSubImage2D(_id, level, offset[0], offset[1], dim[0], dim[1], to_glFormat<type>(format), to_glType(format), data.data() );
    #else
        glBindTexture(to_glType(_target), _id);
        glTexSubImage2D(to_glType(_target), level, offset[0], offset[1], dim[0], dim[1], to_glFormat<type>(format), to_glType(format), data.data());
    #endif
    }

    //Cube Map Case
    template <texture::type T_>
    void Texture<T_>::copy_data(const std::uint32_t  &level, const utils::vec2Ui  &offset, const utils::vec2Ui  &dim, const utils::pixel::Format  &format, const std::array<gsl::span<const std::uint8_t>, 6>  &data)
    {
    #if OPENGL_CORE >= 40500
        for (auto face = 0; face < data.size(); face++) {
            glTextureSubImage3D(_id, level, offset[0], offset[1], face, dim[0], dim[1], 1, to_glFormat<type>(format), to_glType(format), data[face].data() );
        }
        
    #else
        auto image_pairs = std::array< std::pair< std::uint32_t , gsl::span<const std::uint8_t> >  , 6>{
            std::make_pair( gsl::narrow_cast<std::int32_t>( GL_TEXTURE_CUBE_MAP_POSITIVE_X ), data[0] ),
            std::make_pair( gsl::narrow_cast<std::int32_t>( GL_TEXTURE_CUBE_MAP_NEGATIVE_X ), data[1] ),

            std::make_pair( gsl::narrow_cast<std::int32_t>( GL_TEXTURE_CUBE_MAP_POSITIVE_Y ), data[2] ),
            std::make_pair( gsl::narrow_cast<std::int32_t>( GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ), data[3] ),

            std::make_pair( gsl::narrow_cast<std::int32_t>( GL_TEXTURE_CUBE_MAP_POSITIVE_Z ), data[4] ),
            std::make_pair( gsl::narrow_cast<std::int32_t>( GL_TEXTURE_CUBE_MAP_NEGATIVE_Z ), data[5] ),
        };

        glBindTexture(to_glType(_target), _id);

        for(const auto & [tgt, tex] : image_pairs)
        {
            glTexSubImage2D(tgt, level, offset[0], offset[1], dim[0], dim[1], to_glFormat<type>(format), to_glType(format), tex.data());
        }
    #endif
    }

    template <texture::type T_>
    void Texture<T_>::desired_texture_parameters(utils::Uptr<texture::Parameters>  params_)
    {
        _params = std::move(params_);
        using f_params = Parameters::filter_min_params;
        auto params = Parameters{*_params};

        if(!_mip_map)
        {
            if(auto filter = _params->get<Parameters::min_filter>()) {
                if(filter->value != f_params::linear && filter->value != f_params::nearest)
                    params.add( Parameters::min_filter{f_params::linear} );
            }
        }
        texture_parameters(params);   
    }

#if OPENGL_CORE >= 40500
    template <texture::type T_>
    auto Texture<T_>::current_texture_parameters() const -> texture::Parameters
    {
        auto params = Parameters{};
        auto param_vec = std::vector<texture::Parameters::var_t>{
            Parameters::base_level{},
            Parameters::border_color{},
            Parameters::min_filter{},
            Parameters::mag_filter{},
            Parameters::min_lod{},
            Parameters::max_lod{},
            Parameters::max_level{},
            Parameters::swizzle{},
            Parameters::wrap_s{},
            Parameters::wrap_t{},
            Parameters::wrap_r{}
        };

        for(const auto& param : param_vec)
        {
            std::visit([&id = _id, &params](auto&& args)
            {
                using T = std::decay_t<decltype(args)>;

                if constexpr(std::is_same_v<T, Parameters::base_level>) {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_BASE_LEVEL, &value);
                    params.add( Parameters::base_level{gsl::narrow_cast<std::uint32_t>(value)} );
                }
                else if constexpr(std::is_same_v<T, Parameters::border_color>) 
                {
                    auto color = utils::vec4f{};
                    glGetTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, color.data());
                    params.add( Parameters::border_color{color} );
                }
                else if constexpr(std::is_same_v<T, Parameters::min_filter>) 
                {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_MIN_FILTER, &value);
                    params.add( Parameters::min_filter{ to_filter_min_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::mag_filter>) 
                {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_MAG_FILTER, &value);
                    params.add( Parameters::mag_filter{ to_filter_max_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::max_level>) 
                {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_MAX_LEVEL, &value);
                    params.add( Parameters::max_level{gsl::narrow_cast<std::uint32_t>(value)} );
                }
                else if constexpr(std::is_same_v<T, Parameters::min_lod>) 
                {
                    float value{};
                    glGetTextureParameterfv(id, GL_TEXTURE_MIN_LOD, &value);
                    params.add( Parameters::min_lod{value} );
                }
                else if constexpr(std::is_same_v<T, Parameters::max_lod>) 
                {
                    float value{};
                    glGetTextureParameterfv(id, GL_TEXTURE_MAX_LOD, &value);
                    params.add( Parameters::max_lod{value} );
                }
                else if constexpr(std::is_same_v<T, Parameters::swizzle>) 
                {
                    auto value = utils::vec4i{};
                    glGetTextureParameteriv(id, GL_TEXTURE_SWIZZLE_RGBA, value.data());
                    auto s_v = Parameters::swizzle_value{};
                    s_v.set_rgba( to_swizzle(value[0]), 
                                  to_swizzle(value[1]), 
                                  to_swizzle(value[2]),
                                  to_swizzle(value[3]) ) ;
                    
                    params.add( Parameters::swizzle{s_v} );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_s>) 
                {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_WRAP_S, &value );
                    params.add( Parameters::wrap_s{ to_wrap_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_t>) 
                {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_WRAP_T, &value );
                    params.add( Parameters::wrap_t{ to_wrap_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_r>) 
                {
                    std::int32_t value{};
                    glGetTextureParameteriv(id, GL_TEXTURE_WRAP_R, &value );
                    params.add( Parameters::wrap_r{ to_wrap_params(value) } );
                }
                else
                {
                    static_assert(always_false<T>{}, "Non exhaustive Variant");
                }
            }, param);   
        }
        return params;
    }
#else
    template <texture::type T_>
    auto Texture<T_>::current_texture_parameters() const -> texture::Parameters
    {
        auto params = Parameters{};
        auto param_vec = std::vector<texture::Parameters::var_t>{
            Parameters::base_level{},
            Parameters::border_color{},
            Parameters::min_filter{},
            Parameters::mag_filter{},
            Parameters::min_lod{},
            Parameters::max_lod{},
            Parameters::max_level{},
            Parameters::swizzle{},
            Parameters::wrap_s{},
            Parameters::wrap_t{},
            Parameters::wrap_r{}
        };

        glBindTexture(to_glType(_target), _id);

        for(const auto& param : param_vec)
        {
            std::visit([id = to_glType(_target), &params](auto&& args)
            {
                using T = std::decay_t<decltype(args)>;

                if constexpr(std::is_same_v<T, Parameters::base_level>) {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_BASE_LEVEL, &value);
                    params.add( Parameters::base_level{gsl::narrow_cast<std::uint32_t>(value)} );                 
                }
                else if constexpr(std::is_same_v<T, Parameters::border_color>) 
                {
                    log::Logger()->warn("Border Color not present in OpenGL ES");
                    //auto color = utils::vec3f{};
                    //glGetTexParameterfv(id, GL_TEXTURE_BORDER_COLOR, color.data());
                    //params.add( Parameters::border_color{color} );
                    //Not Supported Yet
                }
                else if constexpr(std::is_same_v<T, Parameters::min_filter>) 
                {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_MIN_FILTER, &value);
                    params.add( Parameters::min_filter{ to_filter_min_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::mag_filter>) 
                {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_MAG_FILTER, &value);
                    params.add( Parameters::mag_filter{ to_filter_max_params(value) } );    
                }
                else if constexpr(std::is_same_v<T, Parameters::max_level>) 
                {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_MAX_LEVEL, &value);
                    params.add( Parameters::max_level{gsl::narrow_cast<std::uint32_t>(value)} );
                }
                else if constexpr(std::is_same_v<T, Parameters::min_lod>) 
                {
                    float value{};
                    glGetTexParameterfv(id, GL_TEXTURE_MIN_LOD, &value);
                    params.add( Parameters::min_lod{value} );
                }
                else if constexpr(std::is_same_v<T, Parameters::max_lod>) 
                {
                    float value{};
                    glGetTexParameterfv(id, GL_TEXTURE_MAX_LOD, &value);
                    params.add( Parameters::max_lod{value} );
                }
                else if constexpr(std::is_same_v<T, Parameters::swizzle>) 
                {
                    auto value = utils::vec4i{};
                    glGetTexParameteriv(id, GL_TEXTURE_SWIZZLE_R, &value[0] );
                    glGetTexParameteriv(id, GL_TEXTURE_SWIZZLE_G, &value[1] );
                    glGetTexParameteriv(id, GL_TEXTURE_SWIZZLE_B, &value[2] );
                    glGetTexParameteriv(id, GL_TEXTURE_SWIZZLE_A, &value[3] );
                    auto s_v = Parameters::swizzle_value{};
                    s_v.set_rgba( to_swizzle(value[0]), 
                                  to_swizzle(value[1]), 
                                  to_swizzle(value[2]),
                                  to_swizzle(value[3]) ) ;
                    
                    params.add( Parameters::swizzle{s_v} );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_s>) 
                {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_WRAP_S, &value );
                    params.add( Parameters::wrap_s{ to_wrap_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_t>) 
                {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_WRAP_T, &value );
                    params.add( Parameters::wrap_t{ to_wrap_params(value) } );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_r>) 
                {
                    std::int32_t value{};
                    glGetTexParameteriv(id, GL_TEXTURE_WRAP_R, &value );
                    params.add( Parameters::wrap_r{ to_wrap_params(value) } );
                }
                else
                {
                    static_assert(always_false<T>{}, "Non exhaustive Variant");
                }
            }, param);   
        }
        return params;
    }

#endif

    template <texture::type T_>
    auto Texture<T_>::image_view(const std::uint32_t  &level) -> utils::Uptr<ImageView>
    {
        if(_mip_map && level != 0)
        {
            auto levels = std::log2( std::max(_meta_data->size.width, _meta_data->size.height) );
            if(! (level < levels) )
            {
                return {};
            }

        #if OPENGL_CORE >= 40500
            
            auto width  = std::int32_t{};
            auto height = std::int32_t{};
        
            glGetTextureLevelParameteriv(_id, level, GL_TEXTURE_WIDTH , &width);
            glGetTextureLevelParameteriv(_id, level, GL_TEXTURE_HEIGHT, &height);

            if( !(width > 0) || !(height > 0) ) {
                return {};
            }
        #elif OPENGL_CORE >= 40300

            auto width = std::int32_t{};
            auto height = std::int32_t{};

            glBindTexture(to_glType(_target), _id);

            if (_target == texture::target::cube_map)
            {
                glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, GL_TEXTURE_WIDTH, &width);
                glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, GL_TEXTURE_WIDTH, &height);
            }
            else 
            {
                glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &width);
                glGetTexLevelParameteriv(GL_TEXTURE_2D, level, GL_TEXTURE_WIDTH, &height);
            }

            if (!(width > 0) || !(height > 0)) {
                return {};
            }
            
        #else
            auto den = std::pow(2, level);
            auto width  = gsl::narrow_cast<std::int32_t>( _meta_data->size.width/den );
            auto height = gsl::narrow_cast<std::int32_t>( _meta_data->size.height/den );
        #endif

            auto meta_data = utils::ImageMetaData{ { gsl::narrow_cast<std::uint32_t>(width), gsl::narrow_cast<std::uint32_t>(height) }, _meta_data->format };
            return std::make_unique<texture::ImageView<T_>>(*this, level, meta_data);
        }
        else if(!_mip_map && level != 0)
        {
            return {};
        }
        else
        {
            return std::make_unique<texture::ImageView<T_>>(*this, 0, *_meta_data);
        }
    }


#if OPENGL_CORE >= 40500
    template <texture::type T_>
    void Texture<T_>::texture_parameters(const texture::Parameters  &params)
    {
        for(const auto& [key , param] : params._options)
        {
            std::visit([&id = _id](auto&& args)
            {
                using T = std::decay_t<decltype(args)>;

                if constexpr(std::is_same_v<T, Parameters::base_level>) {
                    auto&& value = static_cast<Parameters::base_level>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_BASE_LEVEL, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::border_color>) 
                {
                    auto&& value = static_cast<Parameters::border_color>(args).value;
                    glTextureParameterfv(id, GL_TEXTURE_BORDER_COLOR, value.data());
                }
                else if constexpr(std::is_same_v<T, Parameters::min_filter>) 
                {
                    auto&& value = static_cast<Parameters::min_filter>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, to_glType(value));
                }
                else if constexpr(std::is_same_v<T, Parameters::mag_filter>) 
                {
                    auto&& value = static_cast<Parameters::mag_filter>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, to_glType(value));
                }
                else if constexpr(std::is_same_v<T, Parameters::max_level>) 
                {
                    auto&& value = static_cast<Parameters::max_level>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_MAX_LEVEL, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::min_lod>) 
                {
                    auto&& value = static_cast<Parameters::min_lod>(args).value;
                    glTextureParameterf(id, GL_TEXTURE_MIN_LOD, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::max_lod>) 
                {
                    auto&& value = static_cast<Parameters::max_lod>(args).value;
                    glTextureParameterf(id, GL_TEXTURE_MAX_LOD, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::swizzle>) 
                {
                    auto&& value = static_cast<Parameters::swizzle>(args).value;
                    auto values = utils::vec4i{
                        to_glType(value.red_channel),
                        to_glType(value.green_channel),
                        to_glType(value.blue_channel),
                        to_glType(value.alpha_channel)
                    };
                    glTextureParameteriv(id, GL_TEXTURE_SWIZZLE_RGBA, values.data());
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_s>) 
                {
                    auto&& value = static_cast<Parameters::wrap_s>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_WRAP_S, to_glType(value) );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_t>) 
                {
                    auto&& value = static_cast<Parameters::wrap_t>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_WRAP_T, to_glType(value) );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_r>) 
                {
                    auto&& value = static_cast<Parameters::wrap_r>(args).value;
                    glTextureParameteri(id, GL_TEXTURE_WRAP_R, to_glType(value) );                 
                }
                else
                {
                    static_assert(always_false<T>{}, "Non exhaustive Variant");
                }
            }, param);   
        }
    }
#else
    template <texture::type T_>
    void Texture<T_>::texture_parameters(const texture::Parameters  &params)
    {
        glBindTexture(to_glType(_target), _id);
        for(const auto& [key , param] : params._options)
        {
            std::visit([id = to_glType(_target)](auto&& args)
            {
                using T = std::decay_t<decltype(args)>;

                if constexpr(std::is_same_v<T, Parameters::base_level>) {
                    auto&& value = static_cast<Parameters::base_level>(args).value;
                    glTexParameteri(id, GL_TEXTURE_BASE_LEVEL, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::border_color>) 
                {
                    auto&& value = static_cast<Parameters::border_color>(args).value;
                    //glTexParameterfv(id, GL_TEXTURE_BORDER_COLOR, value.data());  Not Supported
                }
                else if constexpr(std::is_same_v<T, Parameters::min_filter>) 
                {
                    auto&& value = static_cast<Parameters::min_filter>(args).value;
                    glTexParameteri(id, GL_TEXTURE_MIN_FILTER, to_glType(value));
                }
                else if constexpr(std::is_same_v<T, Parameters::mag_filter>) 
                {
                    auto&& value = static_cast<Parameters::mag_filter>(args).value;
                    glTexParameteri(id, GL_TEXTURE_MAG_FILTER, to_glType(value));
                }
                else if constexpr(std::is_same_v<T, Parameters::max_level>) 
                {
                    auto&& value = static_cast<Parameters::max_level>(args).value;
                    glTexParameteri(id, GL_TEXTURE_MAX_LEVEL, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::min_lod>) 
                {
                    auto&& value = static_cast<Parameters::min_lod>(args).value;
                    glTexParameterf(id, GL_TEXTURE_MIN_LOD, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::max_lod>) 
                {
                    auto&& value = static_cast<Parameters::max_lod>(args).value;
                    glTexParameterf(id, GL_TEXTURE_MAX_LOD, value);
                }
                else if constexpr(std::is_same_v<T, Parameters::swizzle>) 
                {
                    auto&& value = static_cast<Parameters::swizzle>(args).value;
                    auto values = utils::vec4i{
                        to_glType(value.red_channel),
                        to_glType(value.green_channel),
                        to_glType(value.blue_channel),
                        to_glType(value.alpha_channel)
                    };
                    glTexParameteri(id, GL_TEXTURE_SWIZZLE_R, values[0]);
                    glTexParameteri(id, GL_TEXTURE_SWIZZLE_G, values[1]);
                    glTexParameteri(id, GL_TEXTURE_SWIZZLE_B, values[2]);
                    glTexParameteri(id, GL_TEXTURE_SWIZZLE_A, values[3]);
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_s>) 
                {
                    auto&& value = static_cast<Parameters::wrap_s>(args).value;
                    glTexParameteri(id, GL_TEXTURE_WRAP_S, to_glType(value) );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_t>) 
                {
                    auto&& value = static_cast<Parameters::wrap_t>(args).value;
                    glTexParameteri(id, GL_TEXTURE_WRAP_T, to_glType(value) );
                }
                else if constexpr(std::is_same_v<T, Parameters::wrap_r>) 
                {
                    auto&& value = static_cast<Parameters::wrap_r>(args).value;
                    glTexParameteri(id, GL_TEXTURE_WRAP_R, to_glType(value) );
                }
                else
                {
                    static_assert(always_false<T>{}, "Non exhaustive Variant");
                }
            }, param);   
        }
    }
#endif

    template <texture::type T_>
    void Texture<T_>::bind() const
    {
        glBindTexture(to_glType(_target), _id);
    }

    template <texture::type T_>
    void Texture<T_>::active_bind(int num) const
    {
    #if OPENGL_CORE >= 40500
        glBindTextureUnit(num, _id);
    #else
        glActiveTexture(GL_TEXTURE0 + num);
        glBindTexture(to_glType(_target), _id);
    #endif
    }

    template <texture::type T_>
    auto Texture<T_>::get_type() const noexcept -> texture::target{
        return _target;
    }

    template <texture::type T_>
    auto Texture<T_>::get_target() const noexcept -> texture::target{
        return _target;
    }

    template <texture::type T_>
    auto Texture<T_>::current_mip_levels() const noexcept -> std::uint32_t {
        if(!_mip_map)
            return 1;
        else
            return std::log2( std::max(_meta_data->size.height, _meta_data->size.width) );
    }

    namespace texture
    {
        template <type  T_>
        ImageView<T_>::ImageView(const texture_value &texture, const std::uint32_t  &level, const utils::ImageMetaData  &meta_data)
            :_texture{texture}
            ,_level{level}
            ,_meta_data{std::make_unique<utils::ImageMetaData>(meta_data)}
        {}

        template <type  T_>
        ImageView<T_>::ImageView(const ImageView<T_>  &view)
            :_texture{view._texture}
            ,_level{view._level}
            ,_meta_data{std::make_unique<utils::ImageMetaData>(*view._meta_data)}
        {}

        template <type  T_>
        ImageView<T_>::~ImageView() = default;

        template <type  T_>
        auto ImageView<T_>::operator=(const ImageView<T_> &other) -> ImageView<T_>&
        {
            _texture = other._texture;
            _level   = other._level;
            _meta_data = std::move( std::make_unique<utils::ImageMetaData>(*other._meta_data) );
            return *this;
        }
    
        template <type  T_>
        auto ImageView<T_>::get_id() const noexcept -> std::uint32_t
        {
            return _texture.get().get_id();
        }

        template <type  T_>
        auto ImageView<T_>::get_level() const noexcept -> std::uint32_t
        {
            return _level;
        }

        template <type  T_>
        auto ImageView<T_>::get_target() const noexcept -> texture::target
        {
            return _texture.get().get_target();
        }

        template <type  T_>
        auto ImageView<T_>::get_metaData() const noexcept -> const utils::ImageMetaData&
        {
            return *_meta_data;
        }

        template <type  T_>
        auto ImageView<T_>::read_image() const -> std::vector<utils::Uptr<utils::ImageCpu>>
        {
        #if OPENGL_CORE >= 40500

        if( get_target() == texture::target::cube_map )
        {
            auto img = [this]() -> utils::Uptr < utils::ImageCpu > {
                auto img_size = _meta_data->size;
                return std::make_unique<utils::ImageCpu>(utils::image::create_cpu({img_size.width , img_size.height }, _meta_data->format));
            };

            auto image_vecs = std::vector<utils::Uptr< utils::ImageCpu >>{};
            for(auto i = 0; i < 6; i++)
            {
                auto img_cpu = std::make_unique<utils::ImageCpu>(utils::image::create_cpu( _meta_data->size , _meta_data->format));
                glGetTextureSubImage(_texture.get().get_id(), _level, 0, 0, i, _meta_data->size.width, _meta_data->size.height, 1, to_glFormat<T_>(_meta_data->format), to_glType(_meta_data->format), img_cpu->buffer().size(), img_cpu->buffer().data());

                image_vecs.push_back( std::move(img_cpu) );
            }
        
            return image_vecs;
        }
        else
        {
            auto image_cpu = std::make_unique<utils::ImageCpu>( utils::image::create_cpu(_meta_data->size, _meta_data->format) );
            glGetTextureImage(_texture.get().get_id(), _level, to_glFormat<T_>(_meta_data->format), to_glType(_meta_data->format), image_cpu->buffer().size(), image_cpu->buffer().data() );
            
            auto image_vecs = std::vector<utils::Uptr<utils::ImageCpu>>{};
            image_vecs.push_back( std::move(image_cpu) );
            return image_vecs;
        }

        #elif OPENGL_CORE >= 40300

            glBindTexture(to_glType(get_target()), _texture.get().get_id());

            if (get_target() == texture::target::cube_map)
            {
                auto img = [this]() -> utils::Uptr < utils::ImageCpu > {
                    auto img_size = _meta_data->size;
                    return std::make_unique<utils::ImageCpu>(utils::image::create_cpu({ img_size.width , img_size.height }, _meta_data->format));
                };

                auto image_vecs = std::vector<utils::Uptr< utils::ImageCpu >>{};
                for (auto i = 0; i < 6; i++)
                {
                    auto img_cpu = std::make_unique<utils::ImageCpu>(utils::image::create_cpu(_meta_data->size, _meta_data->format));
                    glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _level, to_glFormat<T_>(_meta_data->format), to_glType(_meta_data->format), img_cpu->buffer().data());

                    image_vecs.push_back(std::move(img_cpu));
                }

                return image_vecs;
            }
            else
            {
                auto image_cpu = std::make_unique<utils::ImageCpu>(utils::image::create_cpu(_meta_data->size, _meta_data->format));
                glGetTexImage(GL_TEXTURE_2D, _level, to_glFormat<T_>(_meta_data->format), to_glType(_meta_data->format), image_cpu->buffer().data());

                auto image_vecs = std::vector<utils::Uptr<utils::ImageCpu>>{};
                image_vecs.push_back(std::move(image_cpu));
                return image_vecs;
            }

        #else
            auto image_vecs = std::vector<utils::Uptr<utils::ImageCpu>>{};
            image_vecs.push_back( std::make_unique<utils::ImageCpu>(utils::image::create_cpu( {0, 0}, _meta_data->format)) );
            return image_vecs;
        #endif
        }

        template<type T_>
        auto ImageView<T_>::read_sub_image(std::uint32_t x, std::uint32_t y, std::uint32_t z, std::uint32_t width, std::uint32_t height, std::uint32_t depth) const -> utils::Uptr<utils::ImageCpu>
        {
        #if OPENGL_CORE >= 40500
        
        if( get_target() == texture::target::texture_2D )
        {
            auto image_cpu = std::make_unique<utils::ImageCpu>( utils::image::create_cpu( utils::ImgSize{ width, height }, _meta_data->format ) );
            glGetTextureSubImage( _texture.get().get_id(), _level, x, y, z, width, height, depth, to_glFormat<T_>(_meta_data->format), to_glType(_meta_data->format), image_cpu->buffer().size(), image_cpu->buffer().data() );

            return std::move( image_cpu );
        }else {
            log::Logger()->debug("Texture is not a 2D texture to read sub_texture");

            return {};
        }
        #elif OPENGL_CORE >= 40300

        glBindTexture(to_glType(get_target()), _texture.get().get_id());

        if (get_target() == texture::target::texture_2D)
        {
            auto full_imgs  = read_image();
            assert(full_imgs.size() > 0);

            auto image_cpu = std::make_unique<utils::ImageCpu>( utils::image::create_cpu( utils::ImgSize{width, height}, _meta_data->format ) );

            if(x >= full_imgs.at(0)->meta_data().size.width || y >= full_imgs.at(0)->meta_data().size.height ) {
                log::Logger()->debug("x, y {} {} Sub Texture", x, y);
                return std::move(image_cpu);
            }

            auto& img_buf  = image_cpu->buffer();
            auto img_step  = image_cpu->meta_data().step;
            auto img_bytes = image_cpu->meta_data().format.pixel_layout.bytes;

            auto&& full_img = full_imgs.at(0);
            auto&& full_img_buf = full_img->buffer();
            auto full_step      = full_img->meta_data().step;

            for(auto i = 0; i < height; i++)
            {
                std::memcpy( &img_buf[ i*img_step ], &full_img_buf[ (y + i) * full_step + x * img_bytes ], img_step );
            }

            return std::move(image_cpu);
        }
        else {
            log::Logger()->debug("Texture is not a 2D texture to read sub_texture");

            return {};
        }

        #else
            return std::make_unique<utils::ImageCpu>(utils::image::create_cpu( {0, 0}, _meta_data->format));
        #endif
        }

        template <type  T_>
        auto ImageView<T_>::copy_to(ImageView  &dst_image_view) -> bool
        {
            if( !(dst_image_view.get_target() == texture::target::texture_2D && 
               get_target() == texture::target::texture_2D) ) {
                LOG_W("Texture Copy other than Texture 2D is not handled currently");
                return false;
            }

            auto [src_width, src_height] = get_metaData().size;
            auto [dst_width, dst_height] = dst_image_view.get_metaData().size;

            auto width_roi  = std::min( src_width, dst_width );
            auto height_roi = std::min( src_height, dst_height );

            glCopyImageSubData(get_id(), 
                           to_glType(get_target()),
                           get_level(),   //src_level 
                           0,   //src_x
                           0,   //src_y
                           0,   //src_z
                           dst_image_view.get_id(),
                           to_glType(dst_image_view.get_target()),
                           dst_image_view.get_level(),   //dst_level
                           0,   //dst_x
                           0,   //dst_y
                           0,   //dst_z
                           width_roi,
                           height_roi,
                           1  );

            return true;
        }

        
    } // namespace texture

    template class Texture<texture::type::color>;
    template class Texture<texture::type::depth>;
    template class Texture<texture::type::stencil>;
    template class Texture<texture::type::depth_stencil>;

    template void ColorTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
    template void DepthTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
    template void StencilTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
    template void DepthStencilTexture::texture(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);

    template void ColorTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
    template void DepthTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
    template void StencilTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);
    template void DepthStencilTexture::texture_realloc_size(const utils::Image<utils::ImgBufferCpu>  &image, bool mipmap);

    template void ColorTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
    template void DepthTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
    template void StencilTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
    template void DepthStencilTexture::texture_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);

    template void ColorTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
    template void DepthTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
    template void StencilTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);
    template void DepthStencilTexture::texture_realloc_cube_map(const gsl::span<const utils::Image<utils::ImgBufferCpu>, 6>  &images, bool mipmap);

    template class texture::ImageView<texture::type::color>;
    template class texture::ImageView<texture::type::depth>;
    template class texture::ImageView<texture::type::stencil>;
    template class texture::ImageView<texture::type::depth_stencil>;
}