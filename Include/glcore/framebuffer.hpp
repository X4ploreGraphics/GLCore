

#ifndef NITROS_GLCORE_FRAMEBUFFER_HPP
#define NITROS_GLCORE_FRAMEBUFFER_HPP

#include "glcore/globj.hpp"
#include "glcore/textures.h"
#include "glcore/renderbuffer.hpp"
#include "utilities/data/vecs.hpp"

#include <variant>

namespace nitros::glcore
{
    namespace framebuffer
    {
        class GLCORE_EXPORT Attachment
        {
            public:
            template <texture::type T_>
            using View = std::variant< utils::Sptr< texture::ImageView<T_> > , utils::Sptr< RenderBuffer<T_> > >;

            using ColorView   = View<texture::type::color>;
            using DepthView   = std::variant< View<texture::type::depth>, View<texture::type::depth_stencil> >;
            using StencilView = View<texture::type::stencil>;

            std::vector<ColorView>          color_views;
            utils::Uptr<DepthView>          depth_view;
            utils::Uptr<StencilView>        stencil_view;
        };
    }

    /**
     * OpenGL ES may change active FrameBuffer when creating 
     * and using Member functions.
     * Note: Always bind the framebuffer before drawing in case when different framebuffers are called
     * The class Doesn't check Current Bound Framebuffer for performance reasons
     * 
     * Texture Size: Use texture of the screen size for the framebuffer to render the whole screen
     * Else, only a part of the screen gets rendered into texture.
     * If using texture of small size or large size, control them using view port and scissor appropriately
     * 
     * Dont set mip map level for Texture, as the min filter may choose different level Image View.
     * FrameBuffer only writes to the single mipmap level and not to the whole texture
     * */
    class GLCORE_EXPORT FrameBuffer : public GLobj
    {
        public:
        enum class bind_mode{
            read, draw, both
        };
        enum class format{
            color, depth
        };
        enum class bitFields{
            color, depth, stencil
        };
        enum class factor {
            zero, one, 
            src_color, one_minus_src_color,
            dst_color, one_minus_dst_color,
            src_alpha, one_minus_src_alpha,
            dst_alpha, one_minus_dst_alpha,
            constant_color, one_minus_constant_color,
            constant_alpha, one_minus_constant_alpha
        };

        enum class comparison
        {
            never, always, equal, not_equal, less, less_or_equal, greater, greater_or_equal
        };

        explicit FrameBuffer(utils::Uptr<framebuffer::Attachment>  &&attachment);
        FrameBuffer(const FrameBuffer &) = delete;
        FrameBuffer(FrameBuffer &&) = default;
        ~FrameBuffer();

        FrameBuffer &operator=(const FrameBuffer &) = delete;
        FrameBuffer &operator=(FrameBuffer &&) = default;

        void bind(bind_mode mode = bind_mode::both) const noexcept;

        [[deprecated]] utils::ImageCpu get_pixels(std::uint32_t  x, std::uint32_t y, std::uint32_t width, std::uint32_t height, format  fmt) const;

        [[nodiscard]] auto get_color_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim, std::uint32_t  level = 0) const -> utils::ImageCpu;
        [[nodiscard]] auto get_depth_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim) const -> std::optional<utils::ImageCpu>;
        [[nodiscard]] auto get_stencil_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim) const -> std::optional<utils::ImageCpu>;
        [[nodiscard]] auto get_depth_stencil_pixels(const utils::vec2Ui  &offset, const utils::ImgSize  &dim) const -> std::optional<utils::ImageCpu>;
        

        // Don't Use this Function, use other clear functions. If you are using other clear fncs, remove this function
        [[deprecated]] void clear(std::initializer_list<bitFields>  fields);
        
        void clear_depth(float d = 1.0f);
        void clear_color(const utils::vec4f&  color);
        void clear_stencil(std::int32_t  s);

        void set_blend_func(factor source, factor destination);
        void set_blend_func_separate(factor col_source, factor col_destination, factor alpha_source, factor alpha_destination);

        //Whole Framebuffers
        void depth_mask(bool enable);
        void depthFunc(const comparison  &fnc);

        [[nodiscard]] static auto get_default() noexcept -> FrameBuffer;
        static void bind_default() noexcept;

        private:
        explicit FrameBuffer(const std::uint32_t  id) noexcept;

        utils::Uptr<framebuffer::Attachment>    _attachment;
        mutable bind_mode   _mode;
    };
}

#endif