

#include "glcore/staging_buffer.hpp"
#include "glcore/commands.hpp"
#include "./utils/gl_conversions.hpp"
#include "./platform/gl.hpp"
#include "./logger.hpp"

namespace nitros::glcore
{
    StageBufferWrite::StageBufferWrite()
        :GLobj{}
        ,_mapped{false}
        ,_meta_data{}
    {
        #if OPENGL_CORE >= 40500
            glCreateBuffers(1, &_id);
        #else
            glGenBuffers(1, &_id);
        #endif
    }

    StageBufferWrite::~StageBufferWrite()
    {
        glDeleteBuffers(1, &_id);
    }

    auto StageBufferWrite::is_mapped() const noexcept -> bool {
        return _mapped;
    }

    auto StageBufferWrite::map(const utils::ImageMetaData  &img_meta) -> gsl::span<std::uint8_t>
    {
        if(_mapped) {
            LOG_W("Mapped Already ");
            return {};
        }

        _meta_data = std::make_unique<utils::ImageMetaData>(img_meta);
        auto [width, height] = img_meta.size;
        auto step = img_meta.step;

#if OPENGL_CORE >= 40500
        glNamedBufferData(_id, height * step, NULL, GL_STREAM_DRAW);
        auto map_buffer = glMapNamedBuffer( _id, GL_WRITE_ONLY );
#else
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _id);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, height * step, NULL, GL_STREAM_DRAW);
        auto map_buffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
#endif

        if(map_buffer == NULL) {
            LOG_E("Staging Write buffer Map returns NULL");
            return {};
        }

        _mapped = true;
        return { static_cast<std::uint8_t*>(map_buffer), gsl::narrow_cast<std::ptrdiff_t>( width * step ) };
    }
    
    void StageBufferWrite::unmap()
    {
#if OPENGL_CORE >= 40500
        glUnmapNamedBuffer(_id);
#else
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _id);
        glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
#endif
    _mapped = false;
    }

    template <texture::type  T>
    void StageBufferWrite::stage_data(texture::ImageView<T>  &img_view)
    {
        if (_mapped) {
            LOG_I("Can't Stage as buffer is in Mapped Mode");
            return;
        }
        if(!_meta_data) {
            LOG_I("No Data in Staging Buffer");
            return ;
        }

        auto meta_data = img_view.get_metaData();

        if(meta_data != *_meta_data) {
            LOG_W("Staging MetaData Doesn't match");
            return ;
        }

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _id);
        command::error();

        auto [w, h] = meta_data.size;

#if OPENGL_CORE >= 40500
        glTextureSubImage2D(img_view.get_id(), img_view.get_level(), 0, 0, w, h, to_glFormat<T>(meta_data.format), to_glType(meta_data.format), 0);
#else
        glBindTexture(GL_TEXTURE_2D, img_view.get_id());
        glTexSubImage2D(GL_TEXTURE_2D, img_view.get_level(), 0, 0, w, h, to_glFormat<T>(meta_data.format), to_glType(meta_data.format), 0);
#endif
        command::error();

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        command::error();
    }

    template void StageBufferWrite::stage_data(texture::ImageView<texture::type::color>  &image);
    template void StageBufferWrite::stage_data(texture::ImageView<texture::type::depth>  &image);
    template void StageBufferWrite::stage_data(texture::ImageView<texture::type::depth_stencil>  &image);
    template void StageBufferWrite::stage_data(texture::ImageView<texture::type::stencil>        &image);


    Fence::Fence()
        :_sync_ptr{glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0)}
    {
        glClientWaitSync(static_cast<GLsync>(_sync_ptr), GL_SYNC_FLUSH_COMMANDS_BIT, 0);
    }

    Fence::Fence(Fence &&other)
        :_sync_ptr{other._sync_ptr}
    {
        other._sync_ptr = nullptr;
    }

    Fence::~Fence() {
        if(_sync_ptr) {
            glDeleteSync( static_cast<GLsync>(_sync_ptr) );
        }
    }

    auto Fence::operator=(Fence &&other) -> Fence& {
        _sync_ptr = std::move(other._sync_ptr);
        other._sync_ptr = nullptr;
        return *this;
    }

    auto Fence::commands_complete() const -> bool 
    {
        auto status = GLint{};
        //glClientWaitSync(static_cast<GLsync>(_sync_ptr) , GL_SYNC_FLUSH_COMMANDS_BIT, 0);
        glGetSynciv(static_cast<GLsync>(_sync_ptr) , GL_SYNC_STATUS, sizeof(status), NULL, &status );
        command::error();
        
        if(status == GL_SIGNALED) {
            return true;
        }
        else if (status == GL_UNSIGNALED) {
            return false;
        }
        else {
            return false;
        }
    }


    StageBufferRead::StageBufferRead()
        :GLobj{}
        ,_mapped{false}
        ,_meta_data{}
    {
        #if OPENGL_CORE >= 40500
            glCreateBuffers(1, &_id);
        #else
            glGenBuffers(1, &_id);
        #endif
    }

    StageBufferRead::~StageBufferRead()
    {
        glDeleteBuffers(1, &_id);
    }

    template <texture::type  T>
    auto StageBufferRead::stage_data(texture::ImageView<T>  &img_view) -> utils::Uptr<Fence>
    {
        auto&& meta_data = img_view.get_metaData();
        auto buf_size = meta_data.step * meta_data.size.height;

        _meta_data = std::make_unique<utils::ImageMetaData>( meta_data );

        glBindBuffer(GL_PIXEL_PACK_BUFFER, _id);
        glBufferData(GL_PIXEL_PACK_BUFFER, buf_size, NULL, GL_STREAM_COPY );

#if OPENGL_CORE >= 40500
        glGetTextureSubImage(
            img_view.get_id(),
            img_view.get_level(),
            0,
            0,
            0,
            meta_data.size.width,
            meta_data.size.height,
            1,
            to_glFormat<T>(meta_data.format),
            to_glType(meta_data.format),
            buf_size,
            NULL);
#else
        glBindTexture(GL_TEXTURE_2D, img_view.get_id());
        glGetTexImage(GL_TEXTURE_2D, 
                    img_view.get_level(),
                    to_glFormat<T>(meta_data.format),
                    to_glType(meta_data.format),
                    NULL);
#endif
        
        command::error();

        return std::make_unique<Fence>();
    }

    auto StageBufferRead::map() -> gsl::span<const std::uint8_t>
    {
        if(_mapped) {
            LOG_W("Mapped Already ");
            return {};
        }

        if(!_meta_data) {
            LOG_W("No Textures Staged for Read");
            return {};
        }

#if OPENGL_CORE >= 40500
        auto map_buffer = glMapNamedBuffer( _id, GL_READ_ONLY );
#else
        glBindBuffer(GL_PIXEL_PACK_BUFFER, _id);
        auto map_buffer = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
#endif
        command::error();
        if(map_buffer == NULL) {
            LOG_E("Staging Read buffer Map returns NULL");
            return {};
        }

        auto [width, height] = _meta_data->size;

        _mapped = true;
        return { static_cast<std::uint8_t*>(map_buffer), gsl::narrow_cast<std::ptrdiff_t>( height * _meta_data->step ) };
    }

    void StageBufferRead::unmap()
    {
        if(!_mapped) {
            return ;
        }
#if OPENGL_CORE >= 40500
        glUnmapNamedBuffer(_id);
#else
        glBindBuffer(GL_PIXEL_PACK_BUFFER, _id);
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
#endif
        command::error();
        _mapped = false;
    }

    auto StageBufferRead::is_mapped() const noexcept -> bool
    {
        return _mapped;
    }

    auto StageBufferRead::get_meta_data() const noexcept -> utils::ImageMetaData
    {
        if(!_meta_data) {
            LOG_E("Meta Data Read Before Stage");
            throw std::runtime_error("Meta Data Read Before Stage");
        }
        return *_meta_data;
    }

    template auto StageBufferRead::stage_data(texture::ImageView<texture::type::color>  &image) -> utils::Uptr<Fence>;
    template auto StageBufferRead::stage_data(texture::ImageView<texture::type::depth>  &image) -> utils::Uptr<Fence>;
    template auto StageBufferRead::stage_data(texture::ImageView<texture::type::depth_stencil>  &image) -> utils::Uptr<Fence>;
    template auto StageBufferRead::stage_data(texture::ImageView<texture::type::stencil>        &image) -> utils::Uptr<Fence>;
} // namespace nitros::glcore
