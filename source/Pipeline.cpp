#include "Moon.h"

VertexShader Transform_func;


BS::thread_pool Draw_pool(12);


std::vector <splitValue> s_list = {
    {0.0, 0.25, 0.0, 0.5}, {0.25, 0.5, 0.0, 0.5}, {0.5, 0.75, 0.0, 0.5}, {0.75, 1.0, 0.0, 0.5},
    {0.0, 0.25, 0.5, 1.0}, {0.25, 0.5, 0.5, 1.0}, {0.5, 0.75, 0.5, 1.0}, {0.75, 1.0, 0.5, 1.0}};


/*
std::vector <splitValue> s_list = {
    {0.0, 1.0, 0.0, 1.0}};
*/

/*
std::vector <spilitValue> s_list = {
    {0.0, 0.5, 0.0, 0.5}, {0.5, 1.0, 0.0, 0.5}, 
    {0.0, 0.5, 0.5, 1.0}, {0.5, 1.0, 0.5, 1.0} };


*/

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


  //temporary Rasterize
void Rasterize_thread(const Camera& Receive_camera, MoonBuffer& FrameBuffer, splitValue s, const TriangleList& list, const ParallelLight& light) {
    Graphics GraphicalFunc;
    GraphicalFunc.Refresh_GraphicContext(Receive_camera, FrameBuffer, s, light);
    unsigned int size = (unsigned int)list.vertices.size();
    for (unsigned int k = 0; k < size; k += 3) {
        GraphicalFunc.DrawTriangleTex(list.vertices[k], list.vertices[k + 1], list.vertices[k + 2], list.ptrTexture[k/3], list.ptrMtl[k/3]);

    }
    GraphicalFunc.Deferred_RenderLight();
}

void RenderLight(const Camera& Receive_camera, MoonBuffer& FrameBuffer, splitValue s, const ParallelLight& light) {
    Graphics GraphicalFunc;
    GraphicalFunc.Refresh_GraphicContext(Receive_camera, FrameBuffer, s, light);
    GraphicalFunc.Deferred_RenderLight();
}

unsigned int Render(const Camera Receive_camera, MoonBuffer &FrameBuffer, const long screen_in[2], const std::vector <MoonModel>& Models_list, const ParallelLight & light) {

    TriangleList LIST;
    for (const MoonModel &each_Model : Models_list) {
        Transform_func.transform(Receive_camera, screen_in, each_Model, LIST);
    }
    
    for (splitValue each_chunk : s_list) {
        Draw_pool.submit_task([&, chunk = std::move(each_chunk)]() {

            Rasterize_thread(Receive_camera, FrameBuffer, chunk, LIST, light);
            });
    }
    Draw_pool.wait();

    /*
    for (splitValue each_chunk : s_list_2) {
        Draw_pool.submit_task([&, chunk = std::move(each_chunk)]() {

            RenderLight(Receive_camera, FrameBuffer, chunk, light);
            });
    }
    Draw_pool.wait();
*/
    return static_cast<unsigned int> (LIST.vertices.size() / 3);
    
}