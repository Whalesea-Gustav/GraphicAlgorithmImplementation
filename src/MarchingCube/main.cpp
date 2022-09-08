//
// Created by whalesea on 2022/5/25.
//

//#define AGZ_ENABLE_GL //if set in CMakeLists.txt, this variable will defined in the command line
#ifndef AGZ_ENABLE_GL
#define AGZ_ENABLE_GL
#endif

#include <agz-utils/graphics_api.h>


//#include  "MarchingCubeApp.h"
#include "GridIO.h"
#include "MarchingCubes.h"
using namespace agz::gl;

#include <ticktock.h>

int main()
{
//    window_desc_t windowDesc {
//        .size = {1024, 1024},
//        .title = "test viewer",
//        .multisamples = 4
//    };
//    MarchingCubeApp test(windowDesc);
//    test.run();

    GridReader<double> io(500);
    vector<vector<vector<double>>> field = io.read_grid_from_file("K:\\OpenGL\\Viewer1.06\\asset\\MarchingCube\\a036.grid");
    MarchingCubes<double> test {};
    test.init(Eigen::Vector3<double>(0.0, 0.0, 0.0), Eigen::Vector3<double>(64.0, 64.0, 64.0));
    TICK(MarchingCubesSerial)
    vector<vector<Vector3d>> result = test.GetSurfaceFromField(field, 0.5f);
    TOCK(MarchingCubesSerial)

    TICK(MarchingCubesParallelDebug)
    vector<vector<Vector3d>> parallel_debug_result = test.GetSurfaceFromField_parallel_debug(field, 0.5f);
    TOCK(MarchingCubesParallelDebug)

    TICK(MarchingCUbesParallel)
    vector<vector<Vector3d>> parallel_result = test.GetSurfaceFromField_parallel(field, 0.5f);
    TOCK(MarchingCUbesParallel)

    MarchingCubes<double>::write_triangleList_to_obj_simple(parallel_result, "K:\\OpenGL\\Viewer1.06\\output\\a036_parallel.obj");
}






