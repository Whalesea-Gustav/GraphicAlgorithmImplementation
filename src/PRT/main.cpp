//
// Created by whalesea on 2022/5/25.
//

//#define AGZ_ENABLE_GL //if set in CMakeLists.txt, this variable will defined in the command line
#ifndef AGZ_ENABLE_GL
#define AGZ_ENABLE_GL
#endif

#include <agz-utils/graphics_api.h>
#include "PCSSApp.h"
#include "SH_Sampler.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

using namespace agz::gl;


int main()
{
    string dir = "..\\..\\..\\asset\\CornellBox";
    if (dir[dir.length() - 1] != '\\')
        dir.push_back('\\');
    array<string, 6> faces = { "right", "left", "top", "bottom", "front", "back" };
    std::vector<std::string> imgFiles;
    imgFiles.resize(6);
    string format = "jpg";

    for (int i = 0; i < 6; i++)
        imgFiles[i] = dir + faces[i] + "." + format;
    string outdir = dir;

    SphericcalHarmonicSampler sh_sampler(imgFiles);
    int SH_order = 3;
    sh_sampler.SetSHOrder(SH_order);
    sh_sampler.PrecomputeLightSH();
    cv::Mat expand_cubemap = sh_sampler.RecoverCubemap(128, 128);
    string expandfile = outdir + "Order" + std::to_string(SH_order) + "SHExpand." + format;
    cv::imwrite(expandfile, expand_cubemap * 255);

//    window_desc_t windowDesc {
//        .size = {2400, 1680},
//        .title = "test viewer",
//        .multisamples = 4
//    };
//    PCSSApp test(windowDesc);
//    test.run();
}

//Todo:
// PCSS



