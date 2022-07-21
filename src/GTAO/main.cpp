//
// Created by whalesea on 2022/5/25.
//

//#define AGZ_ENABLE_GL //if set in CMakeLists.txt, this variable will defined in the command line
#ifndef AGZ_ENABLE_GL
#define AGZ_ENABLE_GL
#endif

#include <agz-utils/graphics_api.h>
#include  "HBAOApp.h"


using namespace agz::gl;


int main()
{
    window_desc_t windowDesc {
        .size = {1200, 900},
        .title = "test viewer",
        .multisamples = 4
    };
    //largely reuse HBAO code, but only the HBAO shader is replaced by GTAO shader
    HBAOApp test(windowDesc);
    test.run();
}



