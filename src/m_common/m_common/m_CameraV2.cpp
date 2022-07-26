#include <m_common/m_CameraV2.h>

m_Camera::m_Camera() noexcept
        : vertRad_(0), horiRad_(0),
          fovDeg_(60), nearZ_(0.1f), farZ_(100.0f), wOverH_(1),
          speed_(0.02f), cursorSpeed_(0.003f)
{

}

void m_Camera::setPosition(const glm::vec3 &position) noexcept
{
    pos_ = position;
}

void m_Camera::setDirection(float horiRad, float vertRad) noexcept
{
    horiRad_ = horiRad; //pitch
    vertRad_ = vertRad; //yaw
}

void m_Camera::setSpeed(float speed) noexcept
{
    speed_ = speed;
}

float m_Camera::getSpeed() noexcept
{
    return speed_;
}

void m_Camera::setViewRotationSpeed(float speed) noexcept
{
    cursorSpeed_ = speed;
}

void m_Camera::update(const UpdateParams &params) noexcept
{
    // direction

    vertRad_ -= cursorSpeed_ * params.cursorRelY;
    horiRad_ -= cursorSpeed_ * params.cursorRelX;

    const float PI = agz::math::PI_f;
    while(horiRad_ < 0)       horiRad_ += 2 * PI;
    while(horiRad_ >= 2 * PI) horiRad_ -= 2 * PI;
    vertRad_ = agz::math::clamp(vertRad_, -PI / 2 + 0.01f, PI / 2 - 0.01f);

    // position

    const glm::vec3 dir   = computeDirection();
    const glm::vec3 front = glm::normalize(glm::vec3(dir.x, 0, dir.z));
    const glm::vec3 left  = glm::normalize(glm::cross({ 0, 1, 0 }, front));

    const int frontStep = params.front - params.back;
    const int leftStep  = params.left - params.right;

    pos_ += speed_ * (
            static_cast<float>(frontStep) * front +
            static_cast<float>(leftStep)  * left);

    if(params.up)
        pos_.y += speed_;
    if(params.down)
        pos_.y -= speed_;
}

void m_Camera::setPerspective(float fovDeg, float nearZ, float farZ) noexcept
{
    fovDeg_ = fovDeg;
    nearZ_  = nearZ;
    farZ_   = farZ;
}

void m_Camera::setWOverH(float wOverH) noexcept
{
    wOverH_ = wOverH;
}

void m_Camera::recalculateMatrics() noexcept
{
    //modify view and proj matrix to meet SSR template project
    const glm::vec3 dir = computeDirection();
    view_ = glm::lookAt(pos_, pos_+dir, {0, 1, 0});
    //view_ = m_lookat(pos_, pos_+dir, {0, 1, 0});
//    view_[0][2] = -view_[0][2];
//    view_[1][2] = -view_[1][2];
//    view_[2][2] = -view_[2][2];
//    view_[3][2] = -view_[3][2];

    proj_ = glm::perspective(
            agz::math::deg2rad(fovDeg_),
            wOverH_,
            nearZ_,
            farZ_);

//    proj_[2][2] = -proj_[2][2];
//    proj_[2][3] = -proj_[2][3];

    viewProj_ = view_ * proj_;
}

float m_Camera::getNearZ() const noexcept
{
    return nearZ_;
}

float m_Camera::getFarZ() const noexcept
{
    return farZ_;
}

const glm::vec3 &m_Camera::getPosition() const noexcept
{
    return pos_;
}

glm::vec2 m_Camera::getDirection() const noexcept
{
    return { horiRad_, vertRad_ };
}

const glm::mat4 &m_Camera::getView() const noexcept
{
    return view_;
}

const glm::mat4 &m_Camera::getProj() const noexcept
{
    return proj_;
}

const glm::mat4 &m_Camera::getViewProj() const noexcept
{
    return viewProj_;
}

glm::vec3 m_Camera::computeDirection() const
{
    return {
            std::cos(horiRad_) * std::cos(vertRad_),
            std::sin(vertRad_),
            std::sin(horiRad_) * std::cos(vertRad_)
    };
}

float m_Camera::getFovDegree() const noexcept {
    return fovDeg_;
}

glm::mat4 m_Camera::m_lookat(glm::vec3 eye, glm::vec3 center, glm::vec3 up) {
    glm::vec3 f(glm::normalize(center - eye));
    glm::vec3 s(glm::normalize(cross(up, f)));
    glm::vec3 u(cross(f, s));

    glm::mat4 Result(1);
    Result[0][0] = s.x;
    Result[1][0] = s.y;
    Result[2][0] = s.z;
    Result[3][0] = 0.0f;
    Result[0][1] = u.x;
    Result[1][1] = u.y;
    Result[2][1] = u.z;
    Result[3][1] = 0.0f;
    Result[0][2] = f.x;
    Result[1][2] = f.y;
    Result[2][2] = f.z;
    Result[3][2] = 0.0f;
    Result[0][3] = eye.x;
    Result[1][3] = eye.y;
    Result[2][3] = eye.z;
    Result[3][3] = 1.0f;

    return glm::inverse(Result);
}

void m_Camera::recalculateMatrics_posz() noexcept {

    //View Coordinate with positive Z axis point to Objects
    const glm::vec3 dir = computeDirection();
    view_ = glm::lookAt(pos_, pos_+dir, {0, 1, 0});
    //view_ = m_lookat(pos_, pos_+dir, {0, 1, 0});
    view_[0][2] = -view_[0][2];
    view_[1][2] = -view_[1][2];
    view_[2][2] = -view_[2][2];
    view_[3][2] = -view_[3][2];

    proj_ = glm::perspective(
            agz::math::deg2rad(fovDeg_),
            wOverH_,
            nearZ_,
            farZ_);

    proj_[2][2] = -proj_[2][2];
    proj_[2][3] = -proj_[2][3];

    viewProj_ = view_ * proj_;
}
