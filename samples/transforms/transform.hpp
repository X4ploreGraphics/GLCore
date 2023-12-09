

#ifndef GLCORE_DEV_TRANSFORMS_HPP
#define GLCORE_DEV_TRANSFORMS_HPP


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>


#include  <tuple>

namespace nitros
{
    struct SRT
    {
        glm::vec3   scale;
        glm::vec3   rotate;
        glm::vec3   translate;
    };

    auto model_matrix(const SRT &srt) -> glm::mat4{
        auto&& [scale, rotate, translate] = srt;

        glm::mat4x4 ident{1.0};
        glm::mat4 scale_mat = glm::scale(ident, scale);
        const auto my_quaternion = glm::quat{rotate};
        glm::mat4 rotate_mat = glm::mat4_cast(my_quaternion);
        glm::mat4 translate_mat  = glm::translate(ident, translate);
        glm::mat4 model = translate_mat * (rotate_mat *  scale_mat);
        return model;
    }

    auto perspective_projection_matrix(const float& fov_y_radians, const float&  width, const float&  height, const float &near_plane, const float &far_plane )
    {
        return glm::perspective(fov_y_radians, width/ height , near_plane, far_plane);
    }
}
#endif