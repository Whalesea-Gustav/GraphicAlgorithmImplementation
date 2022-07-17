//
// Created by whalesea on 2022/5/25.
//

//#define AGZ_ENABLE_GL //if set in CMakeLists.txt, this variable will defined in the command line
#ifndef AGZ_ENABLE_GL
#define AGZ_ENABLE_GL
#endif

#include <agz-utils/graphics_api.h>
#include  "RSMApp.h"



using namespace agz::gl;


int main()
{
    window_desc_t windowDesc {
        .size = {1024, 1024},
        .title = "test viewer",
        .multisamples = 4
    };
    RSMApp test(windowDesc);
    test.run();
}
// Todo:
// Reflective Shadow Maps(RSM)





