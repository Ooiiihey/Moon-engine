#include "moon.h"
Transform Transform_func;


void Render(const Camera Receive_camera, Buffer &FrameBuffer, const long screen_in[2], const std::vector <Mesh>& mesh_list) {
    Graphics Graphics_func;

    Mesh_R TransformOut;
    for (Mesh each_mesh : mesh_list) {
        Transform_func.Perspective(Receive_camera, screen_in, each_mesh, TransformOut);
    }
    
    //temporary Rasterize
    for (unsigned int k = 0; k < TransformOut.vertices.size(); k += 3) {

        Color c = TransformOut.color[k / 3];
        Graphics_func.DrawTriangle(Receive_camera, FrameBuffer, TransformOut.vertices[k], TransformOut.vertices[k + 1], TransformOut.vertices[k + 2], c);
    }
    
}