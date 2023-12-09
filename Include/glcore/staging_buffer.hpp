

#ifndef  GLCORE_STAGING_BUFFER_HPP
#define  GLCORE_STAGING_BUFFER_HPP

#include <glcore/globj.hpp>
#include "glcore/buffer.hpp"
#include "glcore/textures.h"

namespace nitros::glcore
{
    class GLCORE_EXPORT StageBufferWrite : public GLobj
    {
        public:
        StageBufferWrite();
        ~StageBufferWrite();

        [[nodiscard]] auto is_mapped() const noexcept -> bool;

        auto map(const utils::ImageMetaData  &img_meta) -> gsl::span<std::uint8_t>;
        void unmap();

        template <texture::type  T>
        void stage_data(texture::ImageView<T>  &img_view);
        
        private:
        bool     _mapped;
        utils::Uptr<utils::ImageMetaData>    _meta_data;
    };

    extern template GLCORE_EXPORT void StageBufferWrite::stage_data(texture::ImageView<texture::type::color>  &image);
    extern template GLCORE_EXPORT void StageBufferWrite::stage_data(texture::ImageView<texture::type::depth>  &image);
    extern template GLCORE_EXPORT void StageBufferWrite::stage_data(texture::ImageView<texture::type::depth_stencil>  &image);
    extern template GLCORE_EXPORT void StageBufferWrite::stage_data(texture::ImageView<texture::type::stencil>        &image);

    class GLCORE_EXPORT Fence
    {
        public:
        explicit Fence();
        Fence(const Fence &) = delete;
        Fence(Fence &&);
        ~Fence();

        auto operator=(const Fence &) -> Fence& = delete;
        auto operator=(Fence &&) -> Fence&;

        auto commands_complete() const -> bool;

        private:
        void*   _sync_ptr;  
    };

    class GLCORE_EXPORT StageBufferRead : public GLobj
    {
        public:
        StageBufferRead();
        ~StageBufferRead();

        template <texture::type  T>
        auto stage_data(texture::ImageView<T>  &img_view) -> utils::Uptr<Fence>;

        auto map() -> gsl::span<const std::uint8_t>;
        void unmap();

        [[nodiscard]] auto is_mapped() const noexcept -> bool;
        [[nodiscard]] auto get_meta_data() const noexcept -> utils::ImageMetaData;

        private:
        bool     _mapped;
        utils::Uptr<utils::ImageMetaData>    _meta_data;
    };

    extern template GLCORE_EXPORT auto StageBufferRead::stage_data(texture::ImageView<texture::type::color>  &image) -> utils::Uptr<Fence>;
    extern template GLCORE_EXPORT auto StageBufferRead::stage_data(texture::ImageView<texture::type::depth>  &image) -> utils::Uptr<Fence>;
    extern template GLCORE_EXPORT auto StageBufferRead::stage_data(texture::ImageView<texture::type::depth_stencil>  &image) -> utils::Uptr<Fence>;
    extern template GLCORE_EXPORT auto StageBufferRead::stage_data(texture::ImageView<texture::type::stencil>        &image) -> utils::Uptr<Fence>;
} // namespace nitros::glcore


#endif