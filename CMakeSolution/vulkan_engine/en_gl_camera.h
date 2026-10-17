#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/gtc/constants.hpp"
#include "glm/gtx/norm.hpp"
#if(defined(GLM_MESSAGES) && !defined(GLM_EXT_INCLUDED))
# pragma message("GLM: GLM_GTX_quaternion extension included")
#endif
#include "en_gl_draw_object.h"
#include "pe_physic_engine.h"
#include "en_vulkan_engine_export.h"

class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual void resize(float width, float height) = 0;

    virtual glm::mat4 view() const = 0;
    virtual glm::mat4 project() const = 0;

    virtual glm::vec3 position() const = 0;
};

class  VULKAN_ENGINE CFreeCamera : public ICamera
{
public:
    CFreeCamera();
    ~CFreeCamera();

    void initialize(glm::vec3 cameraOrigin);
    void updateProject();
    void updateAngle(glm::vec3 deltaOrient, bool bUpdate);
    void updatePos(glm::vec3 origin);
    glm::vec3 position() const override
    {
        return m_origin;
    }
    glm::vec3 up();
    glm::vec3 front();
    glm::vec3 orientation();
    glm::vec3 origin();
    void resize(float width, float height) override;
    float width() const
    {
        return m_width;
    };
    float height() const
    {
        return m_height;
    };
    glm::mat4 view() const override;
    glm::mat4 project() const override;

    glm::mat4 viewRt();
    glm::mat4 projectRt();

private:
    glm::mat4 m_viewRt{1};
    glm::mat4 m_projectRt{1};
    glm::mat4 m_view{1};
    glm::mat4 m_project{1};
    float m_width = 1280.0f;
    float m_height = 720.0f;
    glm::vec3 m_origin = {0.0f, 0.0f, 1.0f};
    glm::vec3 m_up = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 m_front = glm::normalize(glm::vec3(-2.0f, -2.0f, -2.0f));
    glm::vec3 m_orientation{0.0f,0.0f,0.0f};
};

class VULKAN_ENGINE COrbitCamera: public ICamera
{
public:
    COrbitCamera() = default;

    void resize(float width, float height) override;

    void rotate(float deltaX, float deltaY);
    void zoom(float offset);
    void pan(float deltaX, float deltaY);

    glm::mat4 view() const override
    {
        return m_view;
    }
    glm::mat4 project() const override
    {
        return m_projection;
    }
    glm::vec3 position() const {return m_position;}
    void updateView();
    float width() const {return m_width;};
    float height() const {return m_height;};
private:
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};

    float m_distance = 5.0f;
    float m_yaw = -90.0f;
    float m_pitch = 20.0f;

    float m_width = 1280.0f;
    float m_height = 720.0f;

    glm::vec3 m_position{0.0f};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};

    float m_rotateSpeed = 0.2f;
    float m_zoomSpeed = 0.01f;
    float m_panSpeed = 0.005f;
};


