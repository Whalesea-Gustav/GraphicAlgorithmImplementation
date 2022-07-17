#pragma once
#include <vector>
#include <array>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <opencv2/core.hpp>

class SphericcalHarmonicSampler
{
public:

    SphericcalHarmonicSampler();

    SphericcalHarmonicSampler(std::vector<std::string>& _cubemaps_path);

    void SetSHOrder(int _order) { this->SH_Order = _order; };

    void SetCubemapPath(std::vector<std::string>& _cubemaps_path);

    void LoadCubemapImages();

    void PrecomputeLightSH();

    std::vector<glm::vec3> GetLightCoeffs()
    {
        return this->m_lightCoeffs;
    }

    cv::Mat RecoverCubemap(int width, int height);

    glm::vec3 RecoverDirColor(const glm::vec3& dir);

    //hard code
    std::vector<float> Basis(const glm::vec3 & pos)
    {
        int m_Degree = (SH_Order + 1) * (SH_Order + 1);

        std::vector<float> Y(m_Degree);

        glm::vec3 normal = glm::normalize(pos);
        float x = normal.x;
        float y = normal.y;
        float z = normal.z;

        if (m_Degree >= 0)
        {
            Y[0] = 1.f / 2.f * sqrt(1.f / PI);
        }
        if (m_Degree >= 1)
        {
            Y[1] = sqrt(3.f / (4.f * PI)) * z;
            Y[2] = sqrt(3.f / (4.f * PI)) * y;
            Y[3] = sqrt(3.f / (4.f * PI)) * x;
        }
        if (m_Degree >= 2)
        {
            Y[4] = 1.f / 2.f * sqrt(15.f / PI) * x * z;
            Y[5] = 1.f / 2.f * sqrt(15.f / PI) * z * y;
            Y[6] = 1.f / 4.f * sqrt(5.f / PI) * (-x * x - z * z + 2 * y*y);
            Y[7] = 1.f / 2.f * sqrt(15.f / PI) * y * x;
            Y[8] = 1.f / 4.f * sqrt(15.f / PI) * (x * x - z * z);
        }
        if (m_Degree >= 3)
        {
            Y[9]  = 1.f / 4.f * sqrt(35.f / (2.f * PI)) * (3 * x * x - z * z) * z;
            Y[10] = 1.f / 2.f * sqrt(105.f / PI) * x * z * y;
            Y[11] = 1.f / 4.f * sqrt(21.f / (2.f * PI)) * z * (4 * y * y - x * x - z * z);
            Y[12] = 1.f / 4.f * sqrt(7.f / PI)*y*(2 * y*y - 3 * x*x - 3 * z * z);
            Y[13] = 1.f / 4.f * sqrt(21.f / (2.f * PI)) * x * (4 * y * y - x * x - z * z);
            Y[14] = 1.f / 4.f * sqrt(105.f / PI) * (x * x - z * z) * y;
            Y[15] = 1.f / 4.f * sqrt(35.f / (2 * PI)) * (x * x - 3 * z * z) * x;
        }
        return Y;
    }

    static double surfaceArea(double x, double y)
    {
        return atan2(x * y, sqrt(x * x + y * y + 1.0));
    }

    static glm::vec3 CubeUV2XYZ(int index, float u, float v)
    {
        switch (index)
        {
            case 0: return { 1,  v, -u }; 	// +x
            case 1: return { -1,  v,  u }; 	// -x
            case 2: return { u,  1, -v };  // +y
            case 3: return { u, -1,  v };	// -y
            case 4: return { u,  v,  1 };  // +z
            case 5: return { -u,  v, -1 };	// -z
        }
        return glm::vec3();
    }

private:
    //start from 0
    int SH_Order = 3;
    std::vector<std::string> cubemaps_path;
    std::string dir;
    static float PI;
    std::vector<glm::vec3> m_lightCoeffs;
    std::array<cv::Mat, 6> m_Images;
};