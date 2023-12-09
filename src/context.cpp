

#include "glcore/context.hpp"
#include "platform/gl.hpp"
#include <stdexcept>

namespace nitros::glcore
{
    static auto process_ptr = std::any{};

    void load_context(std::any ptr)
    {
        process_ptr = ptr;
#if !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
        using glproc = std::add_pointer_t<void()>;
        using gl_funct_t = std::add_pointer_t<glproc(const char *)>;
        auto glfw_proc_address = std::any_cast<gl_funct_t>(ptr);
        if ( !gladLoadGLLoader( (GLADloadproc) glfw_proc_address ) )
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }
#endif 
    }

    auto get_context() -> std::any
    {
        return process_ptr;
    }

    auto get_version() -> std::pair<std::uint32_t, std::string>
    {
    #if defined( OPENGL_V_40500 )
        return {
            40'500,
            "core"
        };

    #elif defined( OPENGL_V_40300 )
        return {
            40'300,
            "core"
        };

    #elif defined( OPENGL_ES )
        return {
            30'000,
            "es"
        };

    #else
        static_assert(false);
    #endif
    }
}