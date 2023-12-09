

#ifndef NITROS_GLCORE_RENDERBUFFER_HPP
#define NITROS_GLCORE_RENDERBUFFER_HPP

#include "globj.hpp"
#include "textures.h"

namespace nitros::glcore 
{
    template <texture::type T_ = texture::type::color>
    class GLCORE_EXPORT RenderBuffer : public GLobj
    {
        public:
        explicit RenderBuffer(const utils::ImageMetaData  &meta_data);
        RenderBuffer(const RenderBuffer &) = delete;
        RenderBuffer(RenderBuffer &&) = default;
        ~RenderBuffer();

        RenderBuffer &operator=(const RenderBuffer &) = delete;
        RenderBuffer &operator=(RenderBuffer &&) = default;

        void alloc(const utils::ImageMetaData  &meta_data);
        [[nodiscard]] auto  get_metaData() const -> const utils::ImageMetaData&;

        private:
        utils::Uptr<utils::ImageMetaData>   _meta_data;
    };

    using ColorRenderBuffer   = RenderBuffer<texture::type::color>;
    using DepthRenderBuffer   = RenderBuffer<texture::type::depth>;
    using StencilRenderBuffer = RenderBuffer<texture::type::stencil>;
    using DepthStencilRenderBuffer = RenderBuffer<texture::type::depth_stencil>;

    extern template class GLCORE_EXPORT RenderBuffer<texture::type::color>;
    extern template class GLCORE_EXPORT RenderBuffer<texture::type::depth>;
    extern template class GLCORE_EXPORT RenderBuffer<texture::type::stencil>;
    extern template class GLCORE_EXPORT RenderBuffer<texture::type::depth_stencil>;
}

#endif