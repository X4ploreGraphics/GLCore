

#include "glcore/renderbuffer.hpp"
#include "platform/gl.hpp"
#include "utils/gl_conversions.hpp"

namespace nitros::glcore 
{
    template <texture::type T_>
    RenderBuffer<T_>::RenderBuffer(const utils::ImageMetaData  &meta_data)
    {
    #if OPENGL_CORE >= 40500
        glCreateRenderbuffers(1, &_id);
    #else
        glGenRenderbuffers(1, &_id);
    #endif
        alloc(meta_data);
    }

    template <texture::type T_>
    RenderBuffer<T_>::~RenderBuffer() {
        glDeleteRenderbuffers(1, &_id);
    }

    template <texture::type T_>
    void RenderBuffer<T_>::alloc(const utils::ImageMetaData  &meta_data)  
    {
    #if OPENGL_CORE >= 40500
        glNamedRenderbufferStorage(_id, to_internal_glFormat<T_>(meta_data.format), meta_data.size.width, meta_data.size.height);
    #else
        glBindRenderbuffer(GL_RENDERBUFFER, _id);
        glRenderbufferStorage(GL_RENDERBUFFER, to_internal_glFormat<T_>(meta_data.format), meta_data.size.width, meta_data.size.height);
    #endif
        _meta_data = std::make_unique<utils::ImageMetaData>(meta_data);
    }

    template <texture::type T_>
    auto  RenderBuffer<T_>::get_metaData() const -> const utils::ImageMetaData&
    {
        return *_meta_data;
    }

    template class RenderBuffer<texture::type::color>;
    template class RenderBuffer<texture::type::depth>;
    template class RenderBuffer<texture::type::stencil>;
    template class RenderBuffer<texture::type::depth_stencil>;
}