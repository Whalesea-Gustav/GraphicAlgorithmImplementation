#pragma once

#include <agz-utils/graphics_api.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


constexpr float PI = agz::math::PI_f;

class m_Camera
{
public:

    struct UpdateParams
    {
        bool front = false;
        bool left  = false;
        bool right = false;
        bool back  = false;

        bool up   = false;
        bool down = false;

        float cursorRelX = 0;
        float cursorRelY = 0;
    };

    m_Camera() noexcept;

    void setPosition(const glm::vec3 &position) noexcept;

    void setDirection(float horiRad, float vertRad) noexcept;

    void setSpeed(float speed) noexcept;

    float getSpeed() noexcept;

    void setViewRotationSpeed(float speed) noexcept;

    void update(const UpdateParams &params) noexcept;

    void setPerspective(float fovDeg, float nearZ, float farZ) noexcept;

    void setWOverH(float wOverH) noexcept;

    void recalculateMatrics() noexcept;

    float getNearZ() const noexcept;

    float getFarZ() const noexcept;

    float getFovDegree() const noexcept;

    const glm::vec3 &getPosition() const noexcept;

    glm::vec2 getDirection() const noexcept;

    const glm::mat4 &getView() const noexcept;

    const glm::mat4 &getProj() const noexcept;

    const glm::mat4 &getViewProj() const noexcept;

private:

    glm::vec3 computeDirection() const;

    glm::vec3 pos_;
    float vertRad_;
    float horiRad_;

    float fovDeg_;
    float nearZ_;
    float farZ_;

    float wOverH_;

    float speed_;
    float cursorSpeed_;

    glm::mat4 view_;
    glm::mat4 proj_;
    glm::mat4 viewProj_;
};
