#include <array>
#include "en_draw_object.h"
#include "vk_struct_builders.h"
#include <glm/gtx/quaternion.hpp>

namespace en_vk_draw
{

glm::mat4 makeScaleMatrix()
{
    glm::mat4 result(1);
    //result = glm::scale(result, scale_);
    return result;
}

glm::mat4 makeRotationMatrixQuaternion(glm::vec3 orientation)
{
    const glm::quat rot = glm::quat(glm::vec3(glm::radians(orientation.x), glm::radians(orientation.y), glm::radians(orientation.z)));
    return glm::toMat4(rot);
}

glm::mat4 makeTranslationMatrix(glm::vec3 cameraPos)
{
    glm::mat4 result(1);
    result = glm::translate(result, cameraPos);
    return result;
}

}