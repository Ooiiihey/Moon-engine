#include "moon.h"

Graphics Graphics_func;
Transform Tf_func;

void Render(const Camera_data& Receive_camera, Buffer &FrameBuffer, const long screen_in[2], Mmesh& TargetMesh) {

	mesh_tf TransformOut;
    

	Tf_func.Perspective(Receive_camera, screen_in, TargetMesh, TransformOut);

    //temporary Rasterize
    for (unsigned int k = 0; k < TransformOut.vertices_2d.size(); k += 3) {
        Color c = TransformOut.color[k / 3];
        Graphics_func.DrawTriangle(Receive_camera, FrameBuffer, TransformOut.vertices_2d[k], TransformOut.vertices_2d[k + 1], TransformOut.vertices_2d[k + 2], c);
    }


}