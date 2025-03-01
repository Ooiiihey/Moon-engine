#include "moon.h"
Transform Transform_func;


BS::thread_pool Draw_pool(8);


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

std::vector<spilitValue> s_list = {
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
*/

  //temporary Rasterize
void Rasterize_thread(const Camera Receive_camera, Buffer& FrameBuffer, splitValue s, const TriangleList& list, const ParallelLight& light) {
    Graphics Graphics_func;
    for (unsigned int k = 0; k < list.vertices.size(); k += 3) {
        //RGBa c = list.color[k / 3];

        Graphics_func.DrawTriangle(Receive_camera, FrameBuffer, s, list.vertices[k], list.vertices[k + 1], list.vertices[k + 2], list.ptrTexture[k/3], list.ptrMtl[k/3], light);
    }

}

unsigned int Render(const Camera Receive_camera, Buffer &FrameBuffer, const long screen_in[2], const std::vector <Model>& Models_list, const ParallelLight & light) {

    TriangleList LIST;
    for (const Model &each_Model : Models_list) {
        Transform_func.VertexShader(Receive_camera, screen_in, each_Model, LIST);
    }
    
    for (splitValue each_chunk : s_list) {
        Draw_pool.submit_task([&, chunk = std::move(each_chunk)]() {

            Rasterize_thread(Receive_camera, FrameBuffer, chunk, LIST, light);
            });
    }
    Draw_pool.wait();

    return LIST.vertices.size() / 3;
    
}