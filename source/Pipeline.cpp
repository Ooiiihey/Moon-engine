#include "Moon.h"

BS::thread_pool Draw_pool(64);


std::vector <splitValue> s_list = {
    {0.0, 0.25, 0.0, 0.5}, {0.25, 0.5, 0.0, 0.5}, {0.5, 0.75, 0.0, 0.5}, {0.75, 1.0, 0.0, 0.5},
    {0.0, 0.25, 0.5, 1.0}, {0.25, 0.5, 0.5, 1.0}, {0.5, 0.75, 0.5, 1.0}, {0.75, 1.0, 0.5, 1.0}};



std::vector <splitValue> s_list_0 = {
    {0.0, 1.0, 0.0, 1.0}};



std::vector<splitValue> s_list_2 = {
    {0.0, 0.25, 0.0, 0.3333333},
    {0.25, 0.5, 0.0, 0.3333333},
    {0.5, 0.75, 0.0, 0.3333333},
    {0.75, 1.0, 0.0, 0.3333333},
    {0.0, 0.25, 0.3333333, 0.6666667},
    {0.25, 0.5, 0.3333333, 0.6666667},
    {0.5, 0.75, 0.3333333, 0.6666667},
    {0.75, 1.0, 0.3333333, 0.6666667},
    {0.0, 0.25, 0.6666667, 1.0},
    {0.25, 0.5, 0.6666667, 1.0},
    {0.5, 0.75, 0.6666667, 1.0},
    {0.75, 1.0, 0.6666667, 1.0}
};


std::vector<splitValue> s_list_24 = {
    // 水平分 6 列，垂直分 4 层
    // 第 1 层 (0.0 - 0.25)
    {0.0,       1.0 / 6.0,  0.0,    0.25},
    {1.0 / 6.0,   2.0 / 6.0,  0.0,    0.25},
    {2.0 / 6.0,   3.0 / 6.0,  0.0,    0.25},
    {3.0 / 6.0,   4.0 / 6.0,  0.0,    0.25},
    {4.0 / 6.0,   5.0 / 6.0,  0.0,    0.25},
    {5.0 / 6.0,   1.0,      0.0,    0.25},

    // 第 2 层 (0.25 - 0.5)
    {0.0,       1.0 / 6.0,  0.25,   0.5},
    {1.0 / 6.0,   2.0 / 6.0,  0.25,   0.5},
    {2.0 / 6.0,   3.0 / 6.0,  0.25,   0.5},
    {3.0 / 6.0,   4.0 / 6.0,  0.25,   0.5},
    {4.0 / 6.0,   5.0 / 6.0,  0.25,   0.5},
    {5.0 / 6.0,   1.0,      0.25,   0.5},

    // 第 3 层 (0.5 - 0.75)
    {0.0,       1.0 / 6.0,  0.5,    0.75},
    {1.0 / 6.0,   2.0 / 6.0,  0.5,    0.75},
    {2.0 / 6.0,   3.0 / 6.0,  0.5,    0.75},
    {3.0 / 6.0,   4.0 / 6.0,  0.5,    0.75},
    {4.0 / 6.0,   5.0 / 6.0,  0.5,    0.75},
    {5.0 / 6.0,   1.0,      0.5,    0.75},

    // 第 4 层 (0.75 - 1.0)
    {0.0,       1.0 / 6.0,  0.75,   1.0},
    {1.0 / 6.0,   2.0 / 6.0,  0.75,   1.0},
    {2.0 / 6.0,   3.0 / 6.0,  0.75,   1.0},
    {3.0 / 6.0,   4.0 / 6.0,  0.75,   1.0},
    {4.0 / 6.0,   5.0 / 6.0,  0.75,   1.0},
    {5.0 / 6.0,   1.0,      0.75,   1.0}
};

std::vector<splitValue> s_list_64 = {
    // 水平分 8 列，垂直分 8 层
    // 第 1 层 (0.0 - 0.125)
    {0.0,      0.125,  0.0,    0.125},
    {0.125,    0.25,   0.0,    0.125},
    {0.25,     0.375,  0.0,    0.125},
    {0.375,    0.5,    0.0,    0.125},
    {0.5,      0.625,  0.0,    0.125},
    {0.625,    0.75,   0.0,    0.125},
    {0.75,     0.875,  0.0,    0.125},
    {0.875,    1.0,    0.0,    0.125},

    // 第 2 层 (0.125 - 0.25)
    {0.0,      0.125,  0.125,  0.25},
    {0.125,    0.25,   0.125,  0.25},
    {0.25,     0.375,  0.125,  0.25},
    {0.375,    0.5,    0.125,  0.25},
    {0.5,      0.625,  0.125,  0.25},
    {0.625,    0.75,   0.125,  0.25},
    {0.75,     0.875,  0.125,  0.25},
    {0.875,    1.0,    0.125,  0.25},

    // 第 3 层 (0.25 - 0.375)
    {0.0,      0.125,  0.25,   0.375},
    {0.125,    0.25,   0.25,   0.375},
    {0.25,     0.375,  0.25,   0.375},
    {0.375,    0.5,    0.25,   0.375},
    {0.5,      0.625,  0.25,   0.375},
    {0.625,    0.75,   0.25,   0.375},
    {0.75,     0.875,  0.25,   0.375},
    {0.875,    1.0,    0.25,   0.375},

    // 第 4 层 (0.375 - 0.5)
    {0.0,      0.125,  0.375,  0.5},
    {0.125,    0.25,   0.375,  0.5},
    {0.25,     0.375,  0.375,  0.5},
    {0.375,    0.5,    0.375,  0.5},
    {0.5,      0.625,  0.375,  0.5},
    {0.625,    0.75,   0.375,  0.5},
    {0.75,     0.875,  0.375,  0.5},
    {0.875,    1.0,    0.375,  0.5},

    // 第 5 层 (0.5 - 0.625)
    {0.0,      0.125,  0.5,    0.625},
    {0.125,    0.25,   0.5,    0.625},
    {0.25,     0.375,  0.5,    0.625},
    {0.375,    0.5,    0.5,    0.625},
    {0.5,      0.625,  0.5,    0.625},
    {0.625,    0.75,   0.5,    0.625},
    {0.75,     0.875,  0.5,    0.625},
    {0.875,    1.0,    0.5,    0.625},

    // 第 6 层 (0.625 - 0.75)
    {0.0,      0.125,  0.625,  0.75},
    {0.125,    0.25,   0.625,  0.75},
    {0.25,     0.375,  0.625,  0.75},
    {0.375,    0.5,    0.625,  0.75},
    {0.5,      0.625,  0.625,  0.75},
    {0.625,    0.75,   0.625,  0.75},
    {0.75,     0.875,  0.625,  0.75},
    {0.875,    1.0,    0.625,  0.75},

    // 第 7 层 (0.75 - 0.875)
    {0.0,      0.125,  0.75,   0.875},
    {0.125,    0.25,   0.75,   0.875},
    {0.25,     0.375,  0.75,   0.875},
    {0.375,    0.5,    0.75,   0.875},
    {0.5,      0.625,  0.75,   0.875},
    {0.625,    0.75,   0.75,   0.875},
    {0.75,     0.875,  0.75,   0.875},
    {0.875,    1.0,    0.75,   0.875},

    // 第 8 层 (0.875 - 1.0)
    {0.0,      0.125,  0.875,  1.0},
    {0.125,    0.25,   0.875,  1.0},
    {0.25,     0.375,  0.875,  1.0},
    {0.375,    0.5,    0.875,  1.0},
    {0.5,      0.625,  0.875,  1.0},
    {0.625,    0.75,   0.875,  1.0},
    {0.75,     0.875,  0.875,  1.0},
    {0.875,    1.0,    0.875,  1.0}
};

  //temporary Rasterize
void Rasterize_thread(const Camera& Receive_camera, BufferCollection& FrameBuffer, splitValue s, const std::vector <Triangle>& List, LightsCollection& lightsList) {
    BaseGraphics GraphicalFunc;
    GraphicalFunc.RefreshGraphicContext(Receive_camera, FrameBuffer, s);
    unsigned int size = (unsigned int)List.size();

    for (unsigned int k = 0; k < size; ++k) {
        GraphicalFunc.DrawTriangleTexture(List[k].v0, List[k].v1, List[k].v2, List[k].ptrTexture, List[k].ptrMtl);

    }


    
    for (AmbientLight& each : lightsList.AmbientLights) {
       GraphicalFunc.DeferredRender_AmbientLight(each);
    }
    for (ParallelLight& each : lightsList.ParallelLights) {
       GraphicalFunc.DeferredRender_ParallelLight(each);
    }
    for (PointLight& each : lightsList.PointLights) {
        GraphicalFunc.DeferredRender_PointLight(each);
    }

}

void Rasterize_thread_withoutLight(const Camera& Receive_camera, BufferCollection& FrameBuffer, splitValue s, const std::vector <Triangle>& List) {
    BaseGraphics GraphicalFunc;
    GraphicalFunc.RefreshGraphicContext(Receive_camera, FrameBuffer, s);
    unsigned int size = (unsigned int)List.size();

    for (unsigned int k = 0; k < size; ++k) {
        GraphicalFunc.DrawTriangleTexture_direct(List[k].v0, List[k].v1, List[k].v2, List[k].ptrTexture, List[k].ptrMtl);

    }

}


unsigned int Render(const Camera Receive_camera, BufferCollection &FrameBuffer, const int screen_in[2], const std::vector <MoonModel>& Models_list, LightsCollection & lightsList) {
    BaseVertexShader Transform_func;
    std::vector <Triangle> List;

    for (const MoonModel &each_Model : Models_list) {
        Transform_func.Transform(Receive_camera, screen_in, each_Model, List);
    }
    
    for (splitValue each_chunk : s_list_64) {
        Draw_pool.submit_task([&, chunk = std::move(each_chunk)]() {

            Rasterize_thread(Receive_camera, FrameBuffer, chunk, List, lightsList);
            });
    }
    Draw_pool.wait();

    return static_cast<unsigned int> (List.size());
    
}

unsigned int Render(const Camera Receive_camera, BufferCollection& FrameBuffer, const int screen_in[2], const std::vector <MoonModel>& Models_list) {
    BaseVertexShader Transform_func;
    std::vector <Triangle> List;

    for (const MoonModel& each_Model : Models_list) {
        Transform_func.Transform(Receive_camera, screen_in, each_Model, List);
    }

    for (splitValue each_chunk : s_list_24) {
        Draw_pool.submit_task([&, chunk = std::move(each_chunk)]() {

            Rasterize_thread_withoutLight(Receive_camera, FrameBuffer, chunk, List);
            });
    }
    Draw_pool.wait();

    return static_cast<unsigned int> (List.size());

}