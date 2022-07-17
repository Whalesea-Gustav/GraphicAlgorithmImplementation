#pragma once
#include <glm/glm.hpp>
struct Light
{
    glm::vec3 position;
    float constant;

    glm::vec3 ambient;
    float linear;

    glm::vec3 diffuse;
    float quadratic;

    glm::vec3 specular;
    float pending;
};

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 radiance;
};