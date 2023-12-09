

#ifndef _NITROS_GLCORE_GL_CONVERSION_HPP
#define _NITROS_GLCORE_GL_CONVERSION_HPP

#include "../platform/gl.hpp"
#include "image/image.hpp"
#include "glcore/textures.h"
#include "glcore/rasterizer.hpp"
#include "glcore/framebuffer.hpp"
#include "../logger.hpp"

namespace nitros::glcore
{


    // Use only for color Textures
    template <texture::type>
    auto to_internal_glFormat(const utils::pixel::Format  &format) -> std::uint32_t;
    
    template <>
    inline auto to_internal_glFormat<texture::type::color>(const utils::pixel::Format  &format) -> std::uint32_t {
        
        using namespace utils;
        
        auto pixel_type = format.pixel_type;
        auto normalized = format.pixel_layout.normalized;
        auto channel_length = format.pixel_layout.bytes/ format.pixel_layout.channels;

        auto unsupported_channel_length = [](){
            throw std::invalid_argument("Unsupported Channel Length in Color");
        };

        auto gl_format_ = std::uint32_t{GL_RGBA8};

        switch (pixel_type)
        {
            case pixel::type::r :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_R32F;   break;
                            default:  unsupported_channel_length();
                        }
                    }
                    else {
                        switch (channel_length)  {
                            case 1: gl_format_ = GL_R8;   break;
        #if defined(OPENGL_CORE)
                            case 2: gl_format_ = GL_R16;  break;   
        #endif
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;

            case pixel::type::rg :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_RG32F;   break;
                            default:  unsupported_channel_length();   break;
                        }
                    }
                    else 
                    {
                        switch (channel_length)  {
                            case 1: gl_format_ = GL_RG8;   break;
        #if defined(OPENGL_CORE)
                            case 2: gl_format_ = GL_RG16;  break;
        #endif
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;

            case pixel::type::rgb :
            case pixel::type::bgr :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_RGB32F;   break;
                            default:  unsupported_channel_length();
                        }
                    }
                    else 
                    {
                        switch (channel_length)  {
                            case 1: gl_format_ = GL_RGB8;  break;
        #if defined(OPENGL_CORE)
                            case 2: gl_format_ = GL_RGB16; break;
        #endif
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;

            case pixel::type::rgba :
            case pixel::type::bgra :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_RGBA32F;    break;
                            default:  unsupported_channel_length();
                        }
                    }
                    else 
                    {
                        switch (channel_length)  {
                            case 1: gl_format_ = GL_RGBA8;     break;
        #if defined(OPENGL_CORE)
                            case 2: gl_format_ = GL_RGBA16;    break;
        #endif
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;

            default:
                throw std::invalid_argument("Pixel Type Not Supported for Color Texture");
                break;
        }
        return gl_format_;
    }

    template <>
    inline auto to_internal_glFormat<texture::type::depth>(const utils::pixel::Format  &format) -> std::uint32_t{
        
        using namespace utils;
        
        auto pixel_type = format.pixel_type;
        auto normalized = format.pixel_layout.normalized;
        auto channel_length = format.pixel_layout.bytes/ format.pixel_layout.channels;

        auto unsupported_channel_length = [](){
            throw std::invalid_argument("Unsupported Channel Length in Depth");
        };

        auto gl_format_ = std::uint32_t{GL_DEPTH_COMPONENT16};

        switch (pixel_type)
        {
            case pixel::type::grey :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_DEPTH_COMPONENT32F; break;
                            default:  unsupported_channel_length();
                        }
                    }
                    else {
                        switch (channel_length)  {
                            case 1: log::Logger()->info("Using Depth 16 For Internal Pixel Representation");
                                    gl_format_ = GL_DEPTH_COMPONENT16;  break;  // Minimum Bit Depth for Depth is 16, so GREY8 is upscaled to 16
                            case 2: gl_format_ = GL_DEPTH_COMPONENT16;  break;
                            case 3: gl_format_ = GL_DEPTH_COMPONENT24;  break;
        #if defined(OPENGL_CORE)   
                            case 4: gl_format_ = GL_DEPTH_COMPONENT32;  break;   
        #endif
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;

            default:
                throw std::invalid_argument("Pixel Type Not Supported for Depth Texture");
                break;
        }
        return gl_format_;
    }

    template <>
    inline auto to_internal_glFormat<texture::type::stencil>(const utils::pixel::Format  &format) -> std::uint32_t{
        
        using namespace utils;
        
        auto pixel_type = format.pixel_type;
        auto normalized = format.pixel_layout.normalized;
        auto channel_length = format.pixel_layout.bytes/ format.pixel_layout.channels;

        if(pixel_type != pixel::type::stencil) {
            throw std::invalid_argument("Pixel Type not Supported for Stencil Texture");
        }

        if(format.pixel_layout.bytes > 1)
            log::Logger()->warn("Consider using image with 8bit data\n Using Stencil Index 8");
        return GL_STENCIL_INDEX8;
    }

    template <>
    inline auto to_internal_glFormat<texture::type::depth_stencil>(const utils::pixel::Format  &format) -> std::uint32_t{
        
        using namespace utils;
        
        auto pixel_type = format.pixel_type;
        auto normalized = format.pixel_layout.normalized;
        auto channel_length = format.pixel_layout.bytes/ format.pixel_layout.channels;

        auto unsupported_channel_length = [](){
            throw std::invalid_argument("Unsupported Channel Length in Depth");
        };

        auto gl_format_ = std::uint32_t{GL_DEPTH24_STENCIL8};

        switch (pixel_type)
        {
            case pixel::type::grey :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_DEPTH32F_STENCIL8;  break;
                            default:  unsupported_channel_length();
                        }
                    }
                    else {
                        switch (channel_length)  {
                            case 1: 
                            case 2: 
                            case 3: 
                            case 4: gl_format_ = GL_DEPTH24_STENCIL8;   break;
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;
            
            case pixel::type::grey_stencil :
                {
                    if(normalized) {
                        switch (channel_length)  {
                            case 4: gl_format_ = GL_DEPTH32F_STENCIL8;  break;
                            default:  unsupported_channel_length();
                        }
                    }
                    else {
                        switch (format.pixel_layout.bytes)  {
                            case 3: {
                                    log::Logger()->warn("Depth Stencil Using D24_S8, OpenGL D16_8 not available");
                                    gl_format_ = GL_DEPTH24_STENCIL8;
                                };
                                break;
                            case 4: gl_format_ = GL_DEPTH24_STENCIL8;  break;
                            default:  unsupported_channel_length();
                        }
                    }
                }
                break;

            default:
                throw std::invalid_argument("Pixel Type Not Supported for Depth Stencil Texture");
                break;
        }
        return gl_format_;
    }

    template <texture::type>
    auto to_glFormat(const utils::pixel::Format  &format) -> std::uint32_t;

    template <>
    inline auto to_glFormat<texture::type::color>(const utils::pixel::Format  &format) -> std::uint32_t{
        
        using namespace utils;
            switch(format.pixel_type)
            {
                case pixel::type::rgb : return GL_RGB;
                case pixel::type::rgba: return GL_RGBA;
        #if defined(OPENGL_CORE)
                case pixel::type::bgr : return GL_BGR;
                case pixel::type::bgra: return GL_BGRA;
        #else
                case pixel::type::bgr : return GL_RGB;
                case pixel::type::bgra: return GL_RGBA;
        #endif
                case pixel::type::r : return GL_RED;
                case pixel::type::rg: return GL_RG;
                case pixel::type::grey : 
                {
                    if( format.pixel_layout.bytes == 1 )
                        return GL_RED;
                    else
                        return GL_DEPTH_COMPONENT;
                };
                default : throw std::invalid_argument("Invalid Pixel Type For Color");
            }        
    }

    template <>
    inline auto to_glFormat<texture::type::depth>(const utils::pixel::Format  &format) -> std::uint32_t {

        using namespace utils;
            switch(format.pixel_type)
            {
                case pixel::type::rgb : return GL_RGB;
                case pixel::type::rgba: return GL_RGBA;
        #if defined(OPENGL_CORE)
                case pixel::type::bgr : return GL_BGR;
                case pixel::type::bgra: return GL_BGRA;
        #else
                case pixel::type::bgr : return GL_RGB;
                case pixel::type::bgra: return GL_RGBA;
        #endif
                case pixel::type::r : return GL_RED;
                case pixel::type::rg: return GL_RG;
                case pixel::type::grey : 
                {
                    if( format.pixel_layout.bytes == 1 )
                        return GL_RED;
                    else
                        return GL_DEPTH_COMPONENT;
                };
                default : throw std::invalid_argument("Invalid Pixel Type For Depth");
            }        
    }

    template <>
    inline auto to_glFormat<texture::type::stencil>(const utils::pixel::Format  &format) -> std::uint32_t {
        using namespace utils;
#if defined(OPENGL_CORE) || OPENGL_ES >= 30100 
        if(format.pixel_type == pixel::type::r || format.pixel_type == pixel::type::grey || format.pixel_type == pixel::type::stencil)
        {
    
            return GL_STENCIL_INDEX;
        }
        else
        {
            log::Logger()->error("Stencil Format doesn't match STENCIL8");
            throw std::invalid_argument("Invalid Pixel Type For Stencil");
        }
#else
        throw std::invalid_argument("Stencil Alone is Not Supported by OpenGL ES 3.0");
#endif
    }

    template <>
    inline auto to_glFormat<texture::type::depth_stencil>(const utils::pixel::Format  &format) -> std::uint32_t {
        using namespace utils;
        if(format.pixel_type != pixel::type::grey_stencil)
        {
            throw std::invalid_argument("Pixel Type Not Grey Stencil");
        }

        using namespace utils;
        switch(format.pixel_type)
        {
            case pixel::type::rgb : return GL_RGB;
            case pixel::type::rgba: return GL_RGBA;
    #if defined(OPENGL_CORE)
            case pixel::type::bgr : return GL_BGR;
            case pixel::type::bgra: return GL_BGRA;
    #else
            case pixel::type::bgr : return GL_RGB;
            case pixel::type::bgra: return GL_RGBA;
    #endif
            case pixel::type::r : return GL_RED;
            case pixel::type::rg: return GL_RG;
            case pixel::type::grey : 
            {
                if( format.pixel_layout.bytes == 1 )
                    return GL_RED;
                else
                    return GL_DEPTH_COMPONENT;
            };
            case pixel::type::grey_stencil : 
            {
                if (format.pixel_layout.bytes == 3 && !format.pixel_layout.normalized){
                    return GL_DEPTH_STENCIL;
                }
                else if (format.pixel_layout.bytes == 4 && !format.pixel_layout.normalized){
                    return GL_DEPTH_STENCIL;
                }
                else if (format.pixel_layout.bytes == 8 && format.pixel_layout.normalized){
                    return GL_DEPTH_STENCIL;
                }
                else {
                    log::Logger()->warn("Using Default DEPTH_STENCIL for Depth Stencil");
                    return GL_DEPTH_STENCIL;
                }
            }
            default : throw std::invalid_argument("Invalid Pixel Type For Depth Stencil");
        }        
    }


    inline auto glFormat_pixel_type(const std::int32_t  &gl_format) -> utils::pixel::type{
        
        using namespace utils;
        switch(gl_format)
        {
            case GL_RGB  : return pixel::type::rgb ;
            case GL_RGBA : return pixel::type::rgba;
        #if defined(OPENGL_CORE)
            case GL_BGR  : return pixel::type::bgr ;
            case GL_BGRA : return pixel::type::bgra;
        #else
            //case GL_RGB  : return pixel::type::bgr ;
            //case GL_RGBA : return pixel::type::bgra;
        #endif
            case GL_RED : return pixel::type::r;
            case GL_RG  : return pixel::type::rg;
            case GL_DEPTH_COMPONENT : return pixel::type::grey;
            case GL_DEPTH_STENCIL   : return pixel::type::grey_stencil;

            default:
                return pixel::type::rgba;
        }
    }

    inline auto to_glType(const utils::pixel::Format   &format) {
        
        using namespace utils;
        auto bitDepth = format.pixel_layout.bytes/ format.pixel_layout.channels;

        if(format.pixel_type == pixel::type::grey_stencil)
        {
            if(format == pixel::GREY_STENCIL_16_8::value)
            {
                return GL_UNSIGNED_BYTE;
            }
            else if(format == pixel::GREY_STENCIL_24_8::value)
            {
                return GL_UNSIGNED_INT_24_8;
            }
            else if(format == pixel::GREY_STENCIL_32f_8::value)
            {
                return GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
            }
            else
            {
                return GL_UNSIGNED_BYTE;
            }
        }
        
        if(!format.pixel_layout.normalized) {
            switch(bitDepth)
            {
                case 1: return GL_UNSIGNED_BYTE;
                case 2: return GL_UNSIGNED_SHORT;
                case 4: return GL_UNSIGNED_INT;
                default: throw std::runtime_error("Unsupported bit Depth");
            }
        }
        else{
            switch(bitDepth)
            {
                case 4: return GL_FLOAT;
                default: throw std::runtime_error("Unsupported bit Depth");
            }
        }
    }

    inline constexpr auto to_glType(const texture::target  &type_)
    {
        switch (type_)
        {
            case texture::target::texture_2D : return GL_TEXTURE_2D;
            case texture::target::cube_map : return GL_TEXTURE_CUBE_MAP;
            default: 
                log::Logger()->warn("Requested texture target not present, Giving Texture 2D");
            return GL_TEXTURE_2D;
        }
    }

    inline constexpr auto to_glType(const Rasterizer::capability  &type){
        switch (type)
        {
        case Rasterizer::capability::blend      : return GL_BLEND;
        case Rasterizer::capability::cull_face  : return GL_CULL_FACE;
        case Rasterizer::capability::depth_test : return GL_DEPTH_TEST;
        case Rasterizer::capability::scissor_test : return GL_SCISSOR_TEST;
        default:
            log::Logger()->warn("Requested Rasterizer Capability not present");
            return GL_SCISSOR_TEST;
        }
    }

    inline constexpr auto to_glType(const FrameBuffer::comparison  &type){
        switch (type)
        {
        case FrameBuffer::comparison::always : return GL_ALWAYS;
        case FrameBuffer::comparison::never  : return GL_NEVER;

        case FrameBuffer::comparison::equal     : return GL_EQUAL;
        case FrameBuffer::comparison::not_equal : return GL_NOTEQUAL;

        case FrameBuffer::comparison::greater          : return GL_GREATER;
        case FrameBuffer::comparison::greater_or_equal : return GL_GEQUAL;

        case FrameBuffer::comparison::less          : return GL_LESS;
        case FrameBuffer::comparison::less_or_equal : return GL_LEQUAL;

        default:
            log::Logger()->warn("Requested FrameBuffer Comparison not present");
            return GL_LESS;
        }
    }

    inline constexpr auto to_glType(FrameBuffer::factor fact) {
        using factor = FrameBuffer::factor;
        switch (fact)
        {
        case factor::zero :  return GL_ZERO;
        case factor::one :  return GL_ONE;

        case factor::src_color :  return GL_SRC_COLOR;
        case factor::dst_color :  return GL_DST_COLOR;
        case factor::src_alpha :  return GL_SRC_ALPHA;
        case factor::dst_alpha :  return GL_DST_ALPHA;

        case factor::one_minus_src_color :  return GL_ONE_MINUS_SRC_COLOR;
        case factor::one_minus_dst_color :  return GL_ONE_MINUS_DST_COLOR;
        case factor::one_minus_src_alpha :  return GL_ONE_MINUS_SRC_ALPHA;
        case factor::one_minus_dst_alpha :  return GL_ONE_MINUS_DST_ALPHA;
            
        case factor::constant_color :  return GL_CONSTANT_COLOR;
        case factor::constant_alpha :  return GL_CONSTANT_ALPHA;
        case factor::one_minus_constant_color :  return GL_ONE_MINUS_CONSTANT_COLOR;
        case factor::one_minus_constant_alpha :  return GL_ONE_MINUS_CONSTANT_ALPHA;

        default:
            log::Logger()->warn("Requested FrameBuffer Factor not present");
            return GL_ZERO;
        }
    }
} // namespace nitros::glcore


#endif