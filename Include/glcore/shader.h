

#ifndef GLCORE_SHADER_H
#define GLCORE_SHADER_H

#include <string>
#include "glcore/glcore_export.h"
#include <glm/glm.hpp>
#include <array>
#include "utilities/data/vecs.hpp"

namespace nitros::glcore
{
    namespace shader
    {
        struct GLCORE_EXPORT Stages
        {
            std::string  vertex;
            std::string  fragment;
            std::string  geometry;
            std::string  tess_control;
            std::string  tess_evaluation;
        };
    }

    class GLCORE_EXPORT Shader
    {
    public:
        explicit Shader(const std::string  &vertex_pgm, const std::string &frag_pgm);
        explicit Shader(const shader::Stages &stages);
        explicit Shader(const std::uint32_t  &id, bool owning = false);

        Shader(const Shader&) = delete;
        Shader(Shader &&) = default;
        ~Shader();

        Shader& operator=(const Shader&) = delete;
        Shader& operator=(Shader&&) = default;

        void use() const;
        void set_uniform_matrix4fv(const std::string &name, const glm::mat4  &mat, bool transpose = false);

        template<class type, std::size_t N>
        void set_uniform(const std::string &name, const std::array<type, N>  &value);

        template<typename type, typename = std::enable_if_t< std::is_arithmetic_v<type> > >
        void set_uniform(const std::string &name, const type &value){
            set_uniform(name, std::array<type, 1>{value} );
        }

        std::uint32_t    get_program() const;

        static std::uint32_t    get_current_shader();
        static Shader           get_current_shader_t();

        private:
    	std::uint32_t  program;
        bool            _owning;
    };

    //extern template GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec1i &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec1i &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec2i &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec3i &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec4i &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec1f &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec2f &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec3f &value);
    template <> GLCORE_EXPORT void Shader::set_uniform(const std::string &name, const utils::vec4f &value);
}


#endif // GLCORE_SHADER_H
