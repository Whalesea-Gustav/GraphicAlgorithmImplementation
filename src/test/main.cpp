//
// Created by whalesea on 2022/5/25.
//

//#define AGZ_ENABLE_GL //if set in CMakeLists.txt, this variable will defined in the command line
#ifndef AGZ_ENABLE_GL
#define AGZ_ENABLE_GL
#endif

#include <agz-utils/graphics_api.h>
#include <m_common/m_Shader.h>
#include  "DrawModelWithTextureApp.h"


using namespace agz::gl;


int main()
{
    window_desc_t windowDesc {
        .size = {2400, 1680},
        .title = "test viewer",
        .multisamples = 4
    };
    ModelApplication test(windowDesc);

    test.run();
}

//Todo:
//



