#include "SH_Sampler.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "SH_Calculator.h"
#include <glm/vec3.hpp>

float SphericcalHarmonicSampler::PI = 3.14159265358979323846;

SphericcalHarmonicSampler::SphericcalHarmonicSampler() {};

SphericcalHarmonicSampler::SphericcalHarmonicSampler(std::vector<std::string> &_cubemaps_path)
: cubemaps_path(_cubemaps_path)
{
    LoadCubemapImages();
};

void SphericcalHarmonicSampler::SetCubemapPath(std::vector<std::string> &_cubemaps_path)
{
    this->cubemaps_path = _cubemaps_path;
}

void SphericcalHarmonicSampler::LoadCubemapImages()
{
    for (int i = 0; i < 6; i++)
    {
        cv::Mat img = cv::imread(cubemaps_path[i]);
        if (!img.data)
            throw std::runtime_error("read image failed: " + cubemaps_path[i]);
        //CV_{bits}{type}C{channel number}
        img.convertTo(m_Images[i], CV_32FC3, 1.0 / 255.0);
    }
}

void SphericcalHarmonicSampler::PrecomputeLightSH()
{
    int m_Degree = (SH_Order + 1) * (SH_Order + 1);
    m_lightCoeffs = std::vector<glm::vec3>(m_Degree, glm::vec3());
    int w = m_Images[0].cols;
    int h = m_Images[0].rows;

    for (int k = 0; k < 6; k++)
    {
        cv::Mat img = m_Images[k];
        for (int j = 0; j < w; j++)
        {
            for (int i = 0; i < h; i++)
            {
                float px = (float)j + 0.5;
                float py = (float)i + 0.5;
                float u = 2.0 * (px / (float)w) - 1.0;
                float v = 2.0 * (py / (float)h) - 1.0;
                float d_x = 1.0 / (float)w;
                float x0 = u - d_x;
                float y0 = v - d_x;
                float x1 = u + d_x;
                float y1 = v + d_x;

                //d_a = d_w : area=solid angle
                float d_a = surfaceArea(x0, y0) - surfaceArea(x0, y1) - surfaceArea(x1, y0) + surfaceArea(x1, y1);

                glm::vec3 p = CubeUV2XYZ(k, u, v );
                auto c = img.at<cv::Vec3f>(i, j);
                //opencv use bgr format
                glm::vec3 color = { c[2], c[1],c[0] };

                for (int l = 0; l <= SH_Order; ++l)
                {
                    for (int m = -l; m <= l; ++m)
                    {
                        int coeffIndex = SHCoeffCalculator::GetIndex(l, m);
                        m_lightCoeffs[coeffIndex] += static_cast<float>(SHCoeffCalculator::EvalSHBasis_Formula(l, m, glm::normalize(p)) * d_a) * color;
                    }
                }
            }
        }
    }

}
glm::vec3 SphericcalHarmonicSampler::RecoverDirColor(const glm::vec3& dir)
{
    glm::vec3 color = glm::vec3(0.0);
    for (int l = 0; l <= SH_Order; ++l)
    {
        for (int m = -l; m <= l; ++m)
        {
            int coeffIndex = SHCoeffCalculator::GetIndex(l, m);
            color += static_cast<float>(SHCoeffCalculator::EvalSHBasis_Formula(l, m, dir)) * m_lightCoeffs[coeffIndex];
        }
    }
    return color;
}

cv::Mat SphericcalHarmonicSampler::RecoverCubemap(int width, int height)
{
    std::array<cv::Mat, 6> imgs;
    for (int k = 0; k < 6; k++)
    {
        imgs[k] = cv::Mat(height, width, CV_32FC3);
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
            {
                float px = (float)j + 0.5;
                float py = (float)i + 0.5;
                float u = 2.0 * (px / (float)width) - 1.0;
                float v = 2.0 * (py / (float)height) - 1.0;
                glm::vec3 pos = CubeUV2XYZ(k, u, v);
                glm::vec3 color = RecoverDirColor(glm::normalize(pos));
                imgs[k].at<cv::Vec3f>(i, j) = { color.b, color.g, color.r };
            }
        }
    }

    int xarr[6] = { 2 * width, 0, width, width, width, 3 * width };
    int yarr[6] = { height, height, 0, 2 * height, height, height };
    cv::Mat expandimg(3 * height, 4 * width, CV_32FC3);

    for (int i = 0; i < 6; i++)
    {
        cv::Rect region(xarr[i], yarr[i], width, height);
        cv::Mat temp;
        cv::resize(imgs[i], temp, cv::Size(width, height));
        temp.copyTo(expandimg(region));
    }
    return expandimg;
}