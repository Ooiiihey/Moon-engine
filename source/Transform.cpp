#include "moon.h"

inline Vec3 Transform::To_CameraSpace(const Camera& Receive_camera, const Vec3& Vertex_WorldSpace) {
    Vec3 vec_P = Vertex_WorldSpace - Receive_camera.Pos;
    return Vec3(dot(vec_P, Receive_camera.Forward_vec),
                        dot(vec_P, Receive_camera.Y_vec),
                        dot(vec_P, Receive_camera.Z_vec) );
};

//计算线在近平面的交点坐标                          相机                                                           一个点                         另一个点       
 inline Vec3 Transform::Get_CrossPoint(const Camera& Receive_camera, const Vec3& origin_1, const Vec3& origin_2) {
     double alpha = (Receive_camera.NearPlane - origin_1.x) / (origin_2.x - origin_1.x);
     return Vec3(Receive_camera.NearPlane,
                        (origin_2.y - origin_1.y) * alpha + origin_1.y,
                        (origin_2.z - origin_1.z) * alpha + origin_1.z);

 }

// 计算面的法向量                                                  mesh
inline void Transform::Get_NormalVector(Mesh& cMesh, DataCollection& list) {
    Vec3 normal_vectors;
    for (Face& each_face : cMesh.faces) {
        double vec_a[3] = { cMesh.vertices[each_face.index[1]].x - cMesh.vertices[each_face.index[0]].x ,
                                    cMesh.vertices[each_face.index[1]].y - cMesh.vertices[each_face.index[0]].y,
                                    cMesh.vertices[each_face.index[1]].z - cMesh.vertices[each_face.index[0]].z
        };
        double vec_b[3] = { cMesh.vertices[each_face.index[2]].x - cMesh.vertices[each_face.index[1]].x,
                                    cMesh.vertices[each_face.index[2]].y - cMesh.vertices[each_face.index[1]].y,
                                    cMesh.vertices[each_face.index[2]].z - cMesh.vertices[each_face.index[1]].z
        };
        double normal_vector[3] = { (vec_b[1] * vec_a[2] - vec_b[2] * vec_a[1]), (vec_b[2] * vec_a[0] - vec_b[0] * vec_a[2]), (vec_b[0] * vec_a[1] - vec_b[1] * vec_a[0]) };
        double Length = 1 / GetLength(normal_vector);
        normal_vectors.x = normal_vector[0] * Length;
        normal_vectors.y = normal_vector[1] * Length;
        normal_vectors.z = normal_vector[2] * Length;
        list.norVec.emplace_back(normal_vectors);
    }
    return;
}


inline Vertex2D Transform::CameraSpace_to_ScreenSpace(const Camera& Receive_camera, const long screen_in[2], const Vec3& vertex_CameraSpace) {
    Vec3 VecPlane = vertex_CameraSpace * (Receive_camera.NearPlane / vertex_CameraSpace.x);
    double b = (Receive_camera.F * (screen_in[0] >> 1)) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane);
    //这里会装载上CameraSpace的坐标
    return Vertex2D((screen_in[0] >> 1) - (b * VecPlane.y), (screen_in[1] >> 1) - (b * VecPlane.z), vertex_CameraSpace.x, vertex_CameraSpace.y, vertex_CameraSpace.z, 0, 0);//x存储camera_Space中深度
    //平面映射,坐标已适配direcX坐标系

}


//face组装+透视转换                                               摄像机                           分辨率                                原始mesh                   输出mesh
void Transform::VertexShader(const Camera& Receive_camera, const long screen_in[2], Mesh& Original_Mesh, Mesh_R& out) {
    unsigned int faceNUM = (unsigned int)Original_Mesh.faces.size();
    std::vector <bool> Vertices_info;//点可见性检查
    //std::vector <bool> CameraSpaceVertex_exist_info;//存在性检查
    Vertices_info.reserve(faceNUM);
    //CameraSpaceVertex_exist_info.reserve(faceNUM);
    out.vertices.reserve(faceNUM * 3);

    DataCollection VerticesData;
    VerticesData.vertex2d.reserve(Original_Mesh.vertices.size());

    //preCompute
    double b = (Receive_camera.F * (screen_in[0] >> 1)) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane);

    for (const Vec3& each_point : Original_Mesh.vertices) {
        Vertex2D Transformed_P;
        Vec3 CameraSpace_P = To_CameraSpace(Receive_camera, each_point);//转换到camera space

        if (CameraSpace_P.x >= Receive_camera.NearPlane) {
            Vertices_info.push_back(true);
            Vec3 VecPlane = CameraSpace_P * (Receive_camera.NearPlane / CameraSpace_P.x);

            Transformed_P = { (screen_in[0] >> 1) - (b * VecPlane.y),
                                        (screen_in[1] >> 1) - (b * VecPlane.z),
                                        CameraSpace_P.x, CameraSpace_P.y, CameraSpace_P.z,
                                        0, 0 };

            //Transformed_P = CameraSpace_to_ScreenSpace(Receive_camera, screen_in, CameraSpace_P );            //屏幕坐标
        } else { 
             //不可见情况
            Vertices_info.push_back(false);
            Transformed_P = { 0, 0, CameraSpace_P.x, CameraSpace_P.y, CameraSpace_P.z, 0, 0 };
        }
        VerticesData.vertex2d.emplace_back(Transformed_P);
    }
    
    if (not std::any_of(Vertices_info.begin(), Vertices_info.end(), [](bool value) { return value; })) return;//直接跳过完全不可见的mesh

    VerticesData.norVec.reserve(faceNUM);
    Get_NormalVector(Original_Mesh, VerticesData);

    //以面为单位将每个角顶索引，并二次分割处理有不可见点的平面（一个点不可见的情况；两个点不可见的情况；三个点都不可见的情况）
    for (unsigned int i = 0; i < faceNUM; ++i) {
        Face* ptrFace = &Original_Mesh.faces[i];
        unsigned int index0 = ptrFace->index[0];
        if (dot( (Original_Mesh.vertices[index0] - Receive_camera.Pos), VerticesData.norVec[i]) >= 0) continue;

        unsigned int index1 = ptrFace->index[1];
        unsigned int index2 = ptrFace->index[2];
        //面全可见的情况
        if (Vertices_info[index0] && Vertices_info[index1] && Vertices_info[index2] ) {
            //后面此处加入uv索引的结果

            out.vertices.emplace_back(VerticesData.vertex2d[index0]);
            out.vertices.emplace_back(VerticesData.vertex2d[index1]);
            out.vertices.emplace_back(VerticesData.vertex2d[index2]);
            
            out.normal_vectors.emplace_back(VerticesData.norVec[i]);

            out.color.emplace_back(Original_Mesh.color[i]);
        }
        else {
            //不完整面处理
            int num = (!Vertices_info[index0]) + (!Vertices_info[index1]) + (!Vertices_info[index2]);
            if (num == 3) continue;
            bool num_bool = (num == 1);

            unsigned int previous{}, next{}, medium{};//          previous Vec3, next Vec3, medium Vec3    (看情况各自分配是不可见点还是可见点)(索引值)
            Vec3 ClipOut_3d_1, ClipOut_3d_2;            //          3d Vec3 1        3d Vec3 2
            Vertex2D ClipOut_1, ClipOut_2;            //          Vec3 1        Vec3 2
            //为了最后生成的点的数组仍然是正面顺时针的顺序
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Vertices_info[ptrFace->index[e]]) {//获得待处理点            //此处的异或运算分辨是1情况还是2情况
                    medium = ptrFace->index[e];
                    previous = ptrFace->index[(e + 2) % 3];
                    next = ptrFace->index[(e + 1) % 3];
                    break;
                }
            }

            ClipOut_3d_1 = Get_CrossPoint(Receive_camera, VerticesData.get3D(previous), VerticesData.get3D(medium));
            ClipOut_3d_2 = Get_CrossPoint(Receive_camera, VerticesData.get3D(next), VerticesData.get3D(medium));
            ClipOut_1 = CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_1);
            ClipOut_2 = CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_2);
            //后续光栅化加入纹理映射时，记得在这加uv坐标转换！

            if (num_bool) {

                out.vertices.emplace_back(VerticesData.vertex2d[previous]);
                out.vertices.emplace_back(ClipOut_1);
                out.vertices.emplace_back(ClipOut_2);

                out.normal_vectors.emplace_back(VerticesData.norVec[i]);
                out.color.emplace_back(Original_Mesh.color[i]);
                
                //此处输出第二个面
                out.vertices.emplace_back(ClipOut_2);
                out.vertices.emplace_back(VerticesData.vertex2d[next]);
                out.vertices.emplace_back(VerticesData.vertex2d[previous]);

                out.normal_vectors.emplace_back(VerticesData.norVec[i]);
                out.color.emplace_back(Original_Mesh.color[i]);
                
            }
            else {
                //裁切得一个面的情况
                out.vertices.emplace_back(ClipOut_1);
                out.vertices.emplace_back(VerticesData.vertex2d[medium]);
                out.vertices.emplace_back(ClipOut_2);

                out.normal_vectors.emplace_back(VerticesData.norVec[i]);
                out.color.emplace_back(Original_Mesh.color[i]);
            }
        };

    }
    return;
    //developing now
}

