

#include "glcore/commands.hpp"
#include "platform/gl.hpp"
#include "logger.hpp"

namespace nitros::glcore
{
    namespace command
    {
        std::vector<error_type>  error(){
            auto errors = std::vector<error_type>{};
            auto gl_error = glGetError();

            if(gl_error == GL_INVALID_ENUM){
                errors.push_back(error_type::invalid_enum);
                log::Logger()->warn("GL Error {}", "invalid enum");
            }
            if(gl_error == GL_INVALID_VALUE){
                errors.push_back(error_type::invalid_value);
                log::Logger()->warn("GL Error {}", "invalid value");
            }
            if(gl_error == GL_INVALID_OPERATION){
                errors.push_back(error_type::invalid_operation);
                log::Logger()->warn("GL Error {}", "invalid operation");
            }
            if(gl_error == GL_INVALID_FRAMEBUFFER_OPERATION){
                errors.push_back(error_type::invalid_framebuffer_operation);
                log::Logger()->warn("GL Error {}", "invalid framebuffer operation");
            }
            if(gl_error == GL_OUT_OF_MEMORY){
                errors.push_back(error_type::out_of_memory);
                log::Logger()->warn("GL Error {}", "invalid enum");
            }

            if(gl_error == GL_NO_ERROR) {
                errors.push_back(error_type::no_error);
            }

            return errors;
        }
        
    } // namespace command
} // namespace nitros::glcore
