//
// Created by whalesea on 2022/5/25.
//

//#define AGZ_ENABLE_GL //if set in CMakeLists.txt, this variable will defined in the command line
#ifndef AGZ_ENABLE_GL
#define AGZ_ENABLE_GL
#endif

#include <agz-utils/graphics_api.h>
#include  "CSMApp.h"



using namespace agz::gl;


int main()
{
    window_desc_t windowDesc {
        .size = {2560 / 2, 1440 / 2},
        .title = "test viewer",
        .multisamples = 4
    };
    CSMApp test(windowDesc);
    test.run();
}
// Todo:
// Cascaded Shadow Map
// To solve : ShadowMap Resolution Problems





