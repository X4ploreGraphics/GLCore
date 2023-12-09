

#include "glcore/common_processing.hpp"
#include "platform/gl.hpp"

namespace nitros::glcore
{
    void ViewPort::dimension(const ViewDim  &view_dim)
    {
        glViewport( 
            gsl::narrow_cast<std::int32_t>(view_dim.left_bottom[0]), 
            gsl::narrow_cast<std::int32_t>(view_dim.left_bottom[1]), 
            gsl::narrow_cast<std::int32_t>(view_dim.dimension[0]), 
            gsl::narrow_cast<std::int32_t>(view_dim.dimension[1]) );
    }

    auto ViewPort::dimension() -> ViewDim
    {
        auto dim = utils::vec4i{};
        glGetIntegerv(GL_VIEWPORT, dim.data());
        
        auto view_dim = ViewDim{};
        view_dim.left_bottom = { gsl::narrow_cast<std::uint32_t>(dim[0]) , gsl::narrow_cast<std::uint32_t>(dim[1]) };
        view_dim.dimension   = { gsl::narrow_cast<std::uint32_t>(dim[2]) , gsl::narrow_cast<std::uint32_t>(dim[3]) };

        return view_dim;
    }


    void Scissor::sissor(const ViewDim  &view_dim)
    {
        glScissor(
            gsl::narrow_cast<std::int32_t>(view_dim.left_bottom[0]), 
            gsl::narrow_cast<std::int32_t>(view_dim.left_bottom[1]), 
            gsl::narrow_cast<std::int32_t>(view_dim.dimension[0]), 
            gsl::narrow_cast<std::int32_t>(view_dim.dimension[1]) );
    }

    auto Scissor::dimension() -> ViewDim
    {
        auto dim = utils::vec4i{};
        glGetIntegerv(GL_SCISSOR_BOX, dim.data());

        auto view_dim = ViewDim{};
        view_dim.left_bottom = { gsl::narrow_cast<std::uint32_t>(dim[0]) , gsl::narrow_cast<std::uint32_t>(dim[1]) };
        view_dim.dimension   = { gsl::narrow_cast<std::uint32_t>(dim[2]) , gsl::narrow_cast<std::uint32_t>(dim[3]) };

        return view_dim;
    }
} // namespace nitros::glcore
