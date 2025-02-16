#include "moon.h"
Transform Transform_func;
Graphics Graphics_func;

BS::thread_pool Draw_pool(10);


std::vector <splitValue> s_list = {
    {0.0, 0.25, 0.0, 0.5}, {0.25, 0.5, 0.0, 0.5}, {0.5, 0.75, 0.0, 0.5}, {0.75, 1.0, 0.0, 0.5},
    {0.0, 0.25, 0.5, 1.0}, {0.25, 0.5, 0.5, 1.0}, {0.5, 0.75, 0.5, 1.0}, {0.75, 1.0, 0.5, 1.0}};


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
void Rasterize_thread(const Camera Receive_camera, Buffer& FrameBuffer, splitValue s, const Mesh_R& list) {
    for (unsigned int k = 0; k < list.vertices.size(); k += 3) {
        Color c = list.color[k / 3];
        Graphics_func.DrawTriangle(Receive_camera, FrameBuffer, s, list.vertices[k], list.vertices[k + 1], list.vertices[k + 2], c);
    }
}

unsigned int Render(const Camera Receive_camera, Buffer &FrameBuffer, const long screen_in[2], const std::vector <Mesh>& mesh_list) {

    Mesh_R TransformOut;
    for (Mesh each_mesh : mesh_list) {
        Transform_func.VertexShader(Receive_camera, screen_in, each_mesh, TransformOut);
    }
    
    for (splitValue each_chunk : s_list) {
        Draw_pool.submit_task([&, chunk = std::move(each_chunk)]() {
            Rasterize_thread(Receive_camera, FrameBuffer, chunk, TransformOut);
            });
    }
    Draw_pool.wait();
    return TransformOut.normal_vectors.size();
    
}