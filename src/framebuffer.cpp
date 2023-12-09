

#include "glcore/framebuffer.hpp"
#include "platform/gl.hpp"
#include "utils/gl_conversions.hpp"
#include "glcore/commands.hpp"
#include <stdexcept>

namespace nitros::glcore
{
    namespace
    {
        template<class T> struct always_false : std::false_type {};

        void check_status(const std::uint32_t   &id)
        {
            auto error_switch = [](const std::uint32_t  &error){
                switch (error)
                {
                case GL_FRAMEBUFFER_UNDEFINED:
                    log::Logger()->warn("Framebuffer Undefined");
                    break;

                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    log::Logger()->warn("Framebuffer Incomplete Attachment");
                    break;

                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    log::Logger()->warn("Framebuffer InComplete Missing Attachment");
                    break;

                case GL_FRAMEBUFFER_UNSUPPORTED:
                    log::Logger()->warn("Framebuffer UnSupported");
                    break;

                case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                    log::Logger()->warn("Framebuffer Incomplete MultiSample");
                    break;
            
            #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    log::Logger()->warn("Framebuffer Incomplete Layer Targets");
                    break;
            #endif

            #if defined(OPENGL_CORE)
                case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                    log::Logger()->warn("Framebuffer InComplete Draw Buffer");
                    break;

                case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                    log::Logger()->warn("Framebuffer InComplete Read Buffer");
                    break;
            #endif
                
                default:
                    break;
                }
            };

        #if OPENGL_CORE >= 40500
            auto error_id = glCheckNamedFramebufferStatus(id, GL_FRAMEBUFFER);
        #else
            auto error_id = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        #endif

            if(error_id != GL_FRAMEBUFFER_COMPLETE)
            {
                error_switch(error_id);
                command::error();
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                throw std::runtime_error("Frame Buffer is not complete");
            }
        }
    }

    namespace 
    {
        template <texture::type T>
        auto attach_texture(const std::uint32_t  &id, std::uint32_t  attachment, const texture::ImageView<T>  &data)
        {
        #if OPENGL_CORE >= 40500
            glNamedFramebufferTexture(id, attachment, data.get_id(), data.get_level() );
        #else
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, to_glType(data.get_target()), data.get_id(), data.get_level());
        #endif
        }

        template <texture::type T>
        auto attach_cube_map_texture(const std::uint32_t  &id, std::uint32_t  attachment, const texture::ImageView<T>  &data)
        {
        #if OPENGL_CORE >= 40500
            glNamedFramebufferTexture(id, attachment, data.get_id(), data.get_level());
        #elif OPENGL_CORE >= 40300
            glFramebufferTexture(GL_FRAMEBUFFER, attachment, data.get_id(), data.get_level());
        #else
            for(auto i = 0 ; i < 6 ; i++)
            {
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, data.get_id(), data.get_level());
                //glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, data.get_id(), data.get_level(), i);
            }
        #endif
        }

        template <texture::type T>
        auto attach_renderbuffer(const std::uint32_t  &id, std::uint32_t  attachment, const RenderBuffer<T>  &data)
        {
            #if OPENGL_CORE >= 40500
                glNamedFramebufferRenderbuffer(id, attachment, GL_RENDERBUFFER, data.get_id() );
            #else
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, data.get_id());
            #endif
        }
    }

    FrameBuffer::FrameBuffer( utils::Uptr<framebuffer::Attachment>  &&attachment_ ) 
        :_attachment{std::move(attachment_)}
        ,_mode{bind_mode::both}
    {
        auto& attachment = _attachment;
    #if OPENGL_CORE >= 40500
        glCreateFramebuffers(1, &_id);
    #else
        glGenFramebuffers(1, &_id);
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
    #endif

        auto color_count = 0;
        for(const auto &color : attachment->color_views)
        {
            std::visit(
                [&color_count, &id = _id](auto&& args)
                {
                    using T = std::decay_t<decltype(args)>;

                    if constexpr(std::is_same_v<T, utils::Sptr< ColorTexture::ImageView >>)
                    {
                        if(args->get_target() == texture::target::cube_map) {
                            attach_cube_map_texture(id, GL_COLOR_ATTACHMENT0 + color_count, *args);
                        }
                        else {
                            attach_texture(id, GL_COLOR_ATTACHMENT0 + color_count, *args);    
                        }
                    }
                    else if constexpr( std::is_same_v<T, utils::Sptr< ColorRenderBuffer >> )
                    {
                        attach_renderbuffer(id, GL_COLOR_ATTACHMENT0 + color_count, *args);
                    }
                    else
                    {
                        static_assert(always_false<T>{}, "False type");
                    }
                }, color
            );
            color_count++;
        }

            if(attachment->depth_view)
            {
              auto&&  depth_stencil = *attachment->depth_view;

                std::visit([&id = _id](auto&& args)
                {
                    using T = std::decay_t<decltype(args)>;

                    if constexpr(std::is_same_v<T, framebuffer::Attachment::View<texture::type::depth> >) 
                    {
                        std::visit([id](auto&& args)
                        {
                            using T = std::decay_t<decltype(args)>;

                            if constexpr(std::is_same_v<T, utils::Sptr< DepthTexture::ImageView >>) {

                                if(args->get_target() == texture::target::cube_map) {
                                    attach_cube_map_texture(id, GL_DEPTH_ATTACHMENT, *args);
                                }
                                else {
                                    attach_texture(id, GL_DEPTH_ATTACHMENT, *args);
                                }
                            }
                            else if constexpr( std::is_same_v<T, utils::Sptr< DepthRenderBuffer >> ) {
                                attach_renderbuffer(id, GL_DEPTH_ATTACHMENT, *args);
                            }
                            else {
                                static_assert(always_false<T>{}, "False Type");
                            }
                        }, args);  
                        
                    }
                    else if constexpr( std::is_same_v<T, framebuffer::Attachment::View<texture::type::depth_stencil> > ) 
                    {
                        std::visit([id](auto&& args)
                        {
                            using T = std::decay_t<decltype(args)>;

                            if constexpr(std::is_same_v<T, utils::Sptr< DepthStencilTexture::ImageView >>)
                            {
                                if(args->get_target() == texture::target::cube_map) {
                                    attach_cube_map_texture(id, GL_DEPTH_STENCIL_ATTACHMENT, *args);
                                }
                                else {
                                    attach_texture(id, GL_DEPTH_STENCIL_ATTACHMENT, *args);
                                }
                            }
                            else if constexpr( std::is_same_v<T, utils::Sptr< DepthStencilRenderBuffer >> )
                            {
                                attach_renderbuffer(id, GL_DEPTH_STENCIL_ATTACHMENT, *args);
                            }
                            else
                            {
                                static_assert(always_false<T>{}, "False Type");
                            }
                        }, args);    
                    }
                    else {
                        static_assert(always_false<T>{}, "False Type");
                    }

                }, depth_stencil);
            }
            if(attachment->stencil_view)
            {
              auto&&  stencil = *attachment->stencil_view;
            
                std::visit(
                    [&id = _id](auto&& args)
                    {
                        using T = std::decay_t<decltype(args)>;

                        if constexpr(std::is_same_v<T, utils::Sptr< StencilTexture::ImageView >>) {

                            if(args->get_target() == texture::target::cube_map) {
                                attach_cube_map_texture(id, GL_STENCIL_ATTACHMENT, *args);
                            }
                            else {
                                attach_texture(id, GL_STENCIL_ATTACHMENT, *args);
                            }
                        }
                        else if constexpr( std::is_same_v<T, utils::Sptr< StencilRenderBuffer >> ) {
                            attach_renderbuffer(id, GL_STENCIL_ATTACHMENT, *args);
                        }
                        else {
                            static_assert(always_false<T>{}, "False Type");
                        }
                    }, stencil
                );  
            }

        check_status(_id);
    }

    FrameBuffer::FrameBuffer(const std::uint32_t  id) noexcept 
        :_mode{bind_mode::both}
    {    
        _id = id;
        glBindFramebuffer(GL_FRAMEBUFFER, _id);
    }

    FrameBuffer::~FrameBuffer(){
        if(_id != 0)
            glDeleteFramebuffers(1, &_id);
    }

    void FrameBuffer::bind(bind_mode  mode) const noexcept{
        _mode = mode;
        switch (mode)
        {
            case bind_mode::read: glBindFramebuffer(GL_READ_FRAMEBUFFER, _id);
                                    break;
            case bind_mode::draw: glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _id);
                                    break;
            case bind_mode::both: glBindFramebuffer(GL_FRAMEBUFFER, _id);
                                    break;
            default: glBindFramebuffer(GL_FRAMEBUFFER, _id);
                        break;
        }
    }

    utils::ImageCpu FrameBuffer::get_pixels(std::uint32_t  x, std::uint32_t y, std::uint32_t width, std::uint32_t height, format  fmt) const
    {
        auto g_format = [](auto fmt){
            switch (fmt)
            {
                case format::color : return GL_RGBA;
                case format::depth : return GL_DEPTH_COMPONENT;

                default:
                    throw std::invalid_argument("Format Not Handled");
            }
        };

        auto g_type = [](auto fmt){
            switch (fmt)
            {
                case format::color : return utils::pixel::RGBA8::value;
                case format::depth : return utils::pixel::GREY32f::value;

                default:
                    throw std::invalid_argument("Format Pixel not handled");
            }
        };

        auto type = g_type(fmt);
        
    #if OPENGL_CORE >= 40500
        glNamedFramebufferReadBuffer(_id, GL_COLOR_ATTACHMENT0);
    #else
        glBindFramebuffer(GL_READ_BUFFER, _id);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
    #endif
        auto color_image = utils::image::create_cpu({width, height}, g_type(fmt));
        glReadPixels(x, y, width, height, g_format(fmt),  type.pixel_layout.normalized? GL_FLOAT : GL_UNSIGNED_BYTE, color_image.buffer().data());
        return color_image;
    }

    auto FrameBuffer::get_color_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim, std::uint32_t  layer) const -> utils::ImageCpu
    {
    #if OPENGL_CORE >= 40500
        glNamedFramebufferReadBuffer(_id, GL_COLOR_ATTACHMENT0 + layer);
    #else
        auto mode = _mode;
        glBindFramebuffer(GL_READ_BUFFER, _id);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + layer);
    #endif

        auto px_fmt = [id = _id, &attachment = _attachment, layer](){
            if(id == 0)
                return utils::pixel::RGBA8::value;
            else
            {
                const auto &color = attachment->color_views.at(layer);
                return std::visit(
                    [id](auto&& args)
                    {
                        using T = std::decay_t<decltype(args)>;
                        if constexpr(std::is_same_v<T, utils::Sptr< ColorTexture::ImageView >>) {
                            auto&& data = static_cast<utils::Sptr< ColorTexture::ImageView >>(args);
                            return data->get_metaData().format;
                        }
                        else if constexpr( std::is_same_v<T, utils::Sptr< ColorRenderBuffer >> ) {
                            auto&& data = static_cast<utils::Sptr< ColorRenderBuffer >>(args);
                            return data->get_metaData().format;
                        }
                        else {
                            static_assert(always_false<T>{}, "False type");
                        }
                    }, color
                );
            }
        }();
        auto image = utils::image::create_cpu(dim, px_fmt);
        glReadPixels(offset[0], offset[1], dim.width, dim.height, to_glFormat<texture::type::color>(image.meta_data().format), 
                    to_glType( image.meta_data().format ), image.buffer().data() );

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        bind(mode);
    #endif
        return image;    
    }

    auto FrameBuffer::get_depth_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim) const -> std::optional<utils::ImageCpu>
    {
        if(_id != 0 && !_attachment->depth_view){
            return std::nullopt;
        }

        if( !std::holds_alternative<framebuffer::Attachment::View<texture::type::depth> >(*_attachment->depth_view) ) {
            return std::nullopt;
        }

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        auto mode = _mode;
        glBindFramebuffer(GL_READ_BUFFER, _id);
    #endif

        auto px_fmt = [id = _id, &attachment = _attachment](){
            if(id == 0)
                return utils::pixel::GREY16::value;
            else
            {
                const auto &depth = std::get<0>(*attachment->depth_view);
                return std::visit(
                    [id](auto&& args)
                    {
                        using T = std::decay_t<decltype(args)>;
                        if constexpr(std::is_same_v<T, utils::Sptr< DepthTexture::ImageView >>) {
                            return args->get_metaData().format;
                        }
                        else if constexpr( std::is_same_v<T, utils::Sptr< DepthRenderBuffer >> ) {
                            return args->get_metaData().format;
                        }
                        else {
                            static_assert(always_false<T>{}, "False type");
                        }
                    }, depth
                );
            }
        }();
        auto image = utils::image::create_cpu(dim, px_fmt);
        glReadPixels(offset[0], offset[1], dim.width, dim.height, to_glFormat<texture::type::depth>(image.meta_data().format), 
                    to_glType( image.meta_data().format ), image.buffer().data() );

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        bind(mode);
    #endif
        return image;
    }

    auto FrameBuffer::get_stencil_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim) const -> std::optional<utils::ImageCpu>
    {
        if(_id != 0 && !_attachment->stencil_view){
            return std::nullopt;
        }

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        auto mode = _mode;
        glBindFramebuffer(GL_READ_BUFFER, _id);
    #endif

        auto px_fmt = [id = _id, &attachment = _attachment](){
            if(id == 0)
                return utils::pixel::GREY8::value;
            else
            {
                const auto &stencil = *attachment->stencil_view;
                return std::visit(
                    [id](auto&& args)
                    {
                        using T = std::decay_t<decltype(args)>;
                        if constexpr(std::is_same_v<T, utils::Sptr< StencilTexture::ImageView >>) {
                            return args->get_metaData().format;
                        }
                        else if constexpr( std::is_same_v<T, utils::Sptr< StencilRenderBuffer >> ) {
                            return args->get_metaData().format;
                        }
                        else {
                            static_assert(always_false<T>{}, "False type");
                        }
                    }, stencil
                );
            }
        }();
        auto image = utils::image::create_cpu(dim, px_fmt);
        glReadPixels(offset[0], offset[1], dim.width, dim.height, to_glFormat<texture::type::stencil>(image.meta_data().format), 
                    to_glType( image.meta_data().format ), image.buffer().data() );

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        bind(mode);
    #endif
        return image;
    }

    auto FrameBuffer::get_depth_stencil_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim) const -> std::optional<utils::ImageCpu>
    {
        if(_id != 0 && !_attachment->depth_view ){
            return std::nullopt;
        }

        if( !std::holds_alternative<framebuffer::Attachment::View<texture::type::depth_stencil> >(*_attachment->depth_view) ) {
            return std::nullopt;
        }

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        auto mode = _mode;
        glBindFramebuffer(GL_READ_BUFFER, _id);
    #endif

        auto px_fmt = [id = _id, &attachment = _attachment](){
            if(id == 0)
                return utils::pixel::GREY_STENCIL_24_8::value;
            else
            {
                const auto &depth_stencil = std::get<1>(*attachment->depth_view) ;
                return std::visit(
                    [id](auto&& args)
                    {
                        using T = std::decay_t<decltype(args)>;
                        if constexpr(std::is_same_v<T, utils::Sptr< DepthStencilTexture::ImageView >>) {
                            return args->get_metaData().format;
                        }
                        else if constexpr( std::is_same_v<T, utils::Sptr< DepthStencilRenderBuffer >> ) {
                            return args->get_metaData().format;
                        }
                        else {
                            static_assert(always_false<T>{}, "False type");
                        }
                    }, depth_stencil
                );
            }
        }();
        auto image = utils::image::create_cpu(dim, px_fmt);
        glReadPixels(offset[0], offset[1], dim.width, dim.height, to_glFormat<texture::type::depth_stencil>(image.meta_data().format), 
                    to_glType( image.meta_data().format ), image.buffer().data() );

    #if OPENGL_CORE <= 40300 || OPENGL_ES
        bind(mode);
    #endif
        return image;
    }

    void FrameBuffer::clear(std::initializer_list<bitFields>  fields)
    {
        std::uint32_t   flags = 0x00;
        for(auto& field : fields){
            switch(field)
            {
                case bitFields::color : flags |= GL_COLOR_BUFFER_BIT;  break;
                case bitFields::depth : flags |= GL_DEPTH_BUFFER_BIT;  break;
                case bitFields::stencil : flags |= GL_STENCIL_BUFFER_BIT;  break;
            }
        }
        glClear(flags);
    }

    void FrameBuffer::clear_depth(float d){
    #if OPENGL_CORE >= 40500
            glClearNamedFramebufferfv(_id, GL_DEPTH, 0, &d);
    #else
            bind(bind_mode::both);
            glClearBufferfv(GL_DEPTH, 0, &d);
    #endif
    }

    void FrameBuffer::clear_color(const utils::vec4f&  color){
        //glClearColor(color[0], color[1], color[2], color[3]);
        if(_id == 0){
    #if OPENGL_CORE >= 40500
        #if defined(_WIN32)
            glClearNamedFramebufferfv(_id, GL_COLOR, 0, const_cast<GLfloat *>( color.data() ) );
        #else
            glClearNamedFramebufferfv(_id, GL_COLOR, 0, color.data());
        #endif
    #else
            bind(bind_mode::both);
            glClearBufferfv(GL_COLOR, 0, color.data());
    #endif
        }
        else
        {
    #if OPENGL_CORE >= 40500
            for (auto i = 0; i < _attachment->color_views.size(); i++)
            {
        #if defined(_WIN32)
                glClearNamedFramebufferfv(_id, GL_COLOR, i, const_cast<GLfloat*>(color.data()));
        #else
                glClearNamedFramebufferfv(_id, GL_COLOR, i, color.data());
        #endif
            }
    #else
            bind(bind_mode::both);
            for(auto i = 0; i < _attachment->color_views.size(); i++)
                glClearBufferfv(GL_COLOR, i, color.data());
    #endif
        } 
    }

    void FrameBuffer::clear_stencil(std::int32_t  s){
        //glClearStencil(s);
        #if OPENGL_CORE >= 40500
            glClearNamedFramebufferiv(_id, GL_STENCIL, 0, &s);
        #else
            bind(bind_mode::both);
            glClearBufferiv(GL_STENCIL, 0, &s);
        #endif
    }

    void FrameBuffer::set_blend_func(factor source, factor destination) {

        auto get_gl_factor = [](const factor &fact){
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
                throw std::runtime_error("Factor not present");
            }
        };
    //#if defined(OPENGL_CORE) || !(OPENGL_ES < 30200)
    //    glBlendFunci(_id, get_gl_factor(source), get_gl_factor(destination));
    //#else
    //    glBindFramebuffer(GL_FRAMEBUFFER, _id);
        
        glBlendFunc(to_glType(source), to_glType(destination));
    //#endif
    }

    void FrameBuffer::set_blend_func_separate(factor col_source, factor col_destination, factor alpha_source, factor alpha_destination)
    {
        return glBlendFuncSeparate(to_glType(col_source), to_glType(col_destination), to_glType(alpha_source), to_glType(alpha_destination));
    }

    void FrameBuffer::depth_mask(bool enable)
    {
        glDepthMask(enable);
    }

    void FrameBuffer::depthFunc(const comparison  &fnc)
    {
        glDepthFunc(to_glType(fnc));
    }

    auto  FrameBuffer::get_default() noexcept -> FrameBuffer {
        return FrameBuffer{0};
    }

    void FrameBuffer::bind_default() noexcept{
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}