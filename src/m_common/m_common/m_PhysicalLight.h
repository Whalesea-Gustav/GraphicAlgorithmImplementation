#pragma once
#include <glm/glm.hpp>

struct PhysicalLight
{
    glm::vec3 position;
    glm::vec3 radiance;
    PhysicalLight(glm::vec3 _pos, glm::vec3 _radiance) : position(_pos), radiance(_radiance)
    {}
};