

#include "glcore/shader.h"
#include "platform/gl.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "logger.hpp"

namespace nitros::glcore
{
    namespace 
    {
        class ShaderCompiler
        {
            public:

            auto compile_shader_stages(const shader::Stages  &stage) -> std::uint32_t
            {
                auto pgms = std::vector<std::uint32_t>{};

                if(stage.vertex.size() > 0){
                    pgms.push_back(compile_vertex(stage.vertex));
                }
                if(stage.fragment.size() > 0){
                    pgms.push_back(compile_fragment(stage.fragment));
                }
            #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
                if(stage.geometry.size() > 0){
                    pgms.push_back(compile_geometry(stage.geometry));
                }
                if(stage.tess_control.size() > 0){
                    pgms.push_back(compile_tessellation_control(stage.tess_control));
                }
                if(stage.tess_evaluation.size() > 0){
                    pgms.push_back(compile_tessellation_evaluation(stage.tess_evaluation));
                }
            #endif

                const auto start = pgms.data();
                
                auto [passed, program] = create_program( pgms );

                if(!passed)
                {
                    log::Logger()->error("Failed shaders \n{} \n{} \n{} \n{} \n{}", stage.vertex, stage.fragment, stage.tess_control, stage.tess_evaluation, stage.geometry);
                }

                for(auto p : pgms)
                    glDeleteShader(p);

                return program;                
            }

            private:

            auto compile_vertex(const std::string &pgm) -> std::uint32_t
            {
                const auto shader = glCreateShader(GL_VERTEX_SHADER);
                create_shader_check(shader, "Vertex Shader Error");
                const GLchar*  shader_code = pgm.c_str();

                auto success = compiler_shader(shader, shader_code);
                shader_log(shader, success);
                return shader;
            }

            auto compile_fragment(const std::string &pgm) -> std::uint32_t
            {
                const auto shader = glCreateShader(GL_FRAGMENT_SHADER);
                create_shader_check(shader, "Fragment Shader Error");
                const GLchar*  shader_code = pgm.c_str();

                auto success = compiler_shader(shader, shader_code);
                shader_log(shader, success);
                return shader;
            }

        #if defined(OPENGL_CORE) || OPENGL_ES >= 30200
            auto compile_geometry(const std::string &pgm) -> std::uint32_t
            {
                const auto shader = glCreateShader(GL_GEOMETRY_SHADER);
                create_shader_check(shader, "Geometry Shader Error");
                const GLchar*  shader_code = pgm.c_str();

                auto success = compiler_shader(shader, shader_code);
                shader_log(shader, success);
                return shader;
            }

            auto compile_tessellation_control(const std::string &pgm) -> std::uint32_t
            {
                const auto shader = glCreateShader(GL_TESS_CONTROL_SHADER);
                create_shader_check(shader, "Tessellation Control Error");
                const GLchar*  shader_code = pgm.c_str();

                auto success = compiler_shader(shader, shader_code);
                shader_log(shader, success);
                return shader;
            }

            auto compile_tessellation_evaluation(const std::string &pgm) -> std::uint32_t
            {
                const auto shader = glCreateShader(GL_TESS_EVALUATION_SHADER);
                create_shader_check(shader, "Tessellation Evaluation Error");
                const GLchar*  shader_code = pgm.c_str();

                auto success = compiler_shader(shader, shader_code);
                shader_log(shader, success);
                return shader;
            }
        #endif

            auto create_program( const std::vector<std::uint32_t> pgms) -> std::pair<bool, std::uint32_t>
            {
                auto program = glCreateProgram();
                for(auto &pgm : pgms)
                {
                    glAttachShader(program, pgm);
                }
                glLinkProgram(program);

                GLint   p_success;
                glGetProgramiv(program, GL_LINK_STATUS, &p_success);
                if(p_success == GL_FALSE) {
                    char infoLog[512];
                    glGetProgramInfoLog(program, 512, nullptr, infoLog);
                    log::Logger()->error("Error in Linking Program {} \n associated shaders", infoLog);
                    
                    for(auto p : pgms)
                        log::Logger()->error("{}", p);

                    return {false, program};
                }else
                {
                    log::Logger()->debug("Shader Linking Successful");
                    return {true, program};
                }
            }

            void create_shader_check(GLuint shader_code, const std::string_view  &error_str) {
                if(shader_code < 1)
                    throw std::runtime_error(error_str.data());    
            }

            auto compiler_shader(GLuint shader, const GLchar* shader_code) -> GLint
            {
                glShaderSource(shader, 1, &shader_code, nullptr);
                glCompileShader(shader);
                GLint   success;
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);  
                return success;
            }

            void shader_log(GLuint shader, GLint success)
            {
                char infoLog[512];
                if(success == GL_FALSE) {
                    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
                    log::Logger()->error("Error {}", infoLog);
                }
            };
        };
    }

    Shader::Shader(const std::string &vertex_pgm, const std::string &frag_pgm)
        :_owning{true}
    {
        auto stage = shader::Stages{};
        stage.vertex = vertex_pgm;
        stage.fragment = frag_pgm;

        auto shader_compiler = ShaderCompiler{};
        program = shader_compiler.compile_shader_stages(stage);
    }

    Shader::Shader(const shader::Stages &stages)
        :_owning{true}
    {
        auto shader_compiler = ShaderCompiler{};
        program = shader_compiler.compile_shader_stages(stages);
    }

    Shader::Shader(const std::uint32_t  &id, bool owning)
        :program{id}
        ,_owning{owning}
    {}

    Shader::~Shader()
    {
        if(_owning)
            glDeleteProgram(program);
    }

    void Shader::use() const
    {
        glUseProgram(program);
    }

    void Shader::set_uniform_matrix4fv(const std::string &name, const glm::mat4  &mat, bool transpose)
    {
        GLint mat_loc = glGetUniformLocation(program, name.c_str());
        auto var = transpose ? GL_TRUE: GL_FALSE;
        glUniformMatrix4fv(mat_loc, 1, var, glm::value_ptr(mat));
    }

    std::uint32_t   Shader::get_current_shader(){
        auto id = std::int32_t{0};
        glGetIntegerv(GL_CURRENT_PROGRAM, &id);
        return static_cast<std::uint32_t>(id);
    }

    Shader   Shader::get_current_shader_t(){
        auto id = std::int32_t{0};
        glGetIntegerv(GL_CURRENT_PROGRAM, &id);
        return Shader{ static_cast<std::uint32_t>(id), false};
    }

    template <class type, std::size_t N> void Shader::set_uniform(const std::string &name, const std::array<type, N> &value)
    {
        ;
    }

    uint32_t    Shader::get_program() const
    {
        return program;
    }

    template <> void Shader::set_uniform(const std::string &name, const utils::vec1i &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform1i(loc, value[0]);
    }
    template <> void Shader::set_uniform(const std::string &name, const utils::vec2i &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform2i(loc, value[0], value[1]);
    }
    template <> void Shader::set_uniform(const std::string &name, const utils::vec3i &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform3i(loc, value[0], value[1], value[2]);
    }
    template <> void Shader::set_uniform(const std::string &name, const utils::vec4i &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform4i(loc, value[0], value[1], value[2], value[3]);
    }

    //==============================================================================================

    template <> void Shader::set_uniform(const std::string &name, const utils::vec1f &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform1f(loc, value[0]);
    }
    template <> void Shader::set_uniform(const std::string &name, const utils::vec2f &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform2f(loc, value[0], value[1]);
    }
    template <> void Shader::set_uniform(const std::string &name, const utils::vec3f &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform3f(loc, value[0], value[1], value[2]);
    }
    template <> void Shader::set_uniform(const std::string &name, const utils::vec4f &value)
    {
        GLint loc = glGetUniformLocation(program, name.c_str());
        glUniform4f(loc, value[0], value[1], value[2], value[3]);
    }
}