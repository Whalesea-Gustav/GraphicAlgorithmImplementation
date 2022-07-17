#pragma once

#include <agz-utils/graphics_api.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace agz::gl;

constexpr float PI = agz::math::PI_f;

//consistent with d3d api

using Float3 = vec3;
using Float2 = vec2;
using Mat4 = mat4;
using Trans4 = trans4;

inline Float3 computeTangent(
        const Float3 &BA,
        const Float3 &CA,
        const Float2 &uvBA,
        const Float2 &uvCA,
        const Float3 &nor)
{
    const float m00 = uvBA.x, m01 = uvBA.y;
    const float m10 = uvCA.x, m11 = uvCA.y;
    const float det = m00 * m11 - m01 * m10;
    if(std::abs(det) < 0.0001f)
        return agz::math::tcoord3<float>::from_z(nor).x;
    const float inv_det = 1 / det;
    return (m11 * inv_det * BA - m01 * inv_det * CA).normalize();
}


class Camera
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

    Camera() noexcept;

    void setPosition(const Float3 &position) noexcept;

    void setDirection(float horiRad, float vertRad) noexcept;

    void setSpeed(float speed) noexcept;

    void setViewRotationSpeed(float speed) noexcept;

    void update(const UpdateParams &params) noexcept;

    void setPerspective(float fovDeg, float nearZ, float farZ) noexcept;

    void setWOverH(float wOverH) noexcept;

    void recalculateMatrics() noexcept;

    float getNearZ() const noexcept;

    float getFarZ() const noexcept;

    const Float3 &getPosition() const noexcept;

    Float2 getDirection() const noexcept;

    const Mat4 &getView() const noexcept;

    const Mat4 &getProj() const noexcept;

    const Mat4 &getViewProj() const noexcept;

    glm::mat4 getViewGLM()
    {
        const Float3 dir = computeDirection();
        glm::vec3 dirGLM(dir.x, dir.y, dir.z);
        glm::vec3 posGLM(pos_.x, pos_.y, pos_.z);

        glm::vec3 upGLM(0,1,0);

        return glm::lookAt(posGLM, posGLM + dirGLM, upGLM);
    }
    glm::mat4 getProjGLM()
    {
        return glm::perspective(agz::math::deg2rad(fovDeg_),
                                wOverH_, nearZ_, farZ_);
    }


private:

    Float3 computeDirection() const;

    Float3 pos_;
    float vertRad_;
    float horiRad_;

    float fovDeg_;
    float nearZ_;
    float farZ_;

    float wOverH_;

    float speed_;
    float cursorSpeed_;

    Mat4 view_;
    Mat4 proj_;
    Mat4 viewProj_;
};
