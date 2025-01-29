#include "moon.h"


//计算线在近平面的交点坐标                          相机                                                                                                一个点                         另一个点            输出(屏幕上坐标)
 inline void Transform::Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const Vertex& origin_1, const Vertex& origin_2, Vertex& out_vec_3d) {
    //待处理点和camera的front点的向量
    Vertex vec1 = { Receive_camera.Forward_vec.x - origin_2.x + Receive_camera.CameraPos[0],
                                Receive_camera.Forward_vec.y - origin_2.y + Receive_camera.CameraPos[1],
                                Receive_camera.Forward_vec.z - origin_2.z + Receive_camera.CameraPos[2] };
    //待处理点和另一点的向量
    Vertex vec2 = { origin_1.x - origin_2.x,
                                origin_1.y - origin_2.y,
                                origin_1.z - origin_2.z };
    //double vec1_dotValue = dot(Receive_camera.Forward_vec, vec1);
    double a = dot(Receive_camera.Forward_vec, vec1) / dot(vec2, Receive_camera.Forward_vec);    //转换算法(已化简)
    if (a == 0) {
        //恰好距离零时处理
        out_vec_3d.x = origin_2.x - Receive_camera.CameraPos[0];
        out_vec_3d.y = origin_2.y - Receive_camera.CameraPos[1];
        out_vec_3d.z = origin_2.z - Receive_camera.CameraPos[2];
    }
    else {
        /*old
                double Tmp[] = { (vec2.x * distance) / (cosV * vec2_Len) + origin_2.x,
                                            (vec2.y * distance) / (cosV * vec2_Len) + origin_2.y,
                                            (vec2.z * distance) / (cosV * vec2_Len) + origin_2.z };
        */
        out_vec_3d.x = vec2.x * a + origin_2.x - Receive_camera.CameraPos[0];
        out_vec_3d.y = vec2.y * a + origin_2.y - Receive_camera.CameraPos[1];
        out_vec_3d.z = vec2.z * a + origin_2.z - Receive_camera.CameraPos[2];
    };
    return;
}



// 计算面的法向量                                                  mesh
inline void Transform::Get_NormalVector(Mmesh& cMesh) {
    Vertex normal_vectors;
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
        double Length = 1 / GetLength(normal_vector);//法向量的模
        normal_vectors.x = normal_vector[0] * Length;
        normal_vectors.y = normal_vector[1] * Length;
        normal_vectors.z = normal_vector[2] * Length;
        cMesh.normal_vectors.push_back(normal_vectors);
    }
    return;
}


inline void Transform::CameraSpace_to_ScreenSpace(const Camera_data& Receive_camera, const long screen_in[2], const Vertex& vertex_origin, Vertex& out) {
    Vertex VecPlane = { vertex_origin.x - Receive_camera.Forward_vec.x,
                                    vertex_origin.y - Receive_camera.Forward_vec.y,
                                    vertex_origin.z - Receive_camera.Forward_vec.z };
    double b = (Receive_camera.F * (screen_in[0] >> 1))  / (tan(Receive_camera.FOV) * Receive_camera.NearPlane);
    /*old 
    * 保留原理
    out.x = (screen_in[0] >> 1) + ( Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Y_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane * GetLength(Receive_camera.Y_vec)));
    out.y = (screen_in[1] >> 1) - ( Receive_camera.F * (screen_in[0] >> 1) * dot(Receive_camera.Z_vec, VecPlane) / (tan(Receive_camera.FOV) * Receive_camera.NearPlane * GetLength(Receive_camera.Z_vec)));
    */
    out.x = (screen_in[0] >> 1) + ((b * dot(Receive_camera.Y_vec, VecPlane)) / GetLength(Receive_camera.Y_vec));
    out.y = (screen_in[1] >> 1) - (b * dot(Receive_camera.Z_vec, VecPlane) / GetLength(Receive_camera.Z_vec));
    //平面坐标轴映射,坐标已适配direcX坐标系

}


//三角形组装+透视转换                                               摄像机                           分辨率                                原始mesh                   输出mesh
void Transform::Perspective(const Camera_data& Receive_camera, const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh) {
    std::vector <bool> Pinfo;//点的可见性检查数组
    std::vector <Vertex> Transformed_vertices;//顶点缓冲对象
    //计算法向量
    TargetMesh.normal_vectors.clear();
    TargetMesh.normal_vectors.reserve(TargetMesh.faces.size());
    Get_NormalVector(TargetMesh);

    for (const Vertex& each_point : TargetMesh.vertices) {
        //临时值
        Vertex  vec_P;
        Vertex Transformed_P;
        vec_P.x = each_point.x - Receive_camera.CameraPos[0];
        vec_P.y = each_point.y - Receive_camera.CameraPos[1];
        vec_P.z = each_point.z - Receive_camera.CameraPos[2];

        double dotValue = dot(Receive_camera.Forward_vec, vec_P);
        if (double depth = (dotValue / Receive_camera.NearPlane); depth >= Receive_camera.NearPlane) {
            Pinfo.push_back(true);//添加点可见的信息

            double a = (Receive_camera.NearPlane * Receive_camera.NearPlane) / dotValue;             //此处算法已经化简
            vec_P = { vec_P.x * a , vec_P.y * a, vec_P.z * a };                                                                     //计算映射到三维camera_sapce平面的坐标
            
            CameraSpace_to_ScreenSpace(Receive_camera, screen_in, vec_P, Transformed_P);            //转换为屏幕坐标
            Transformed_P.z = 1/ depth;     //用来存储camera_Space中逆Depth后面制作Dbuffer用       //逆深度
 
        } else { 
             //不可见情况处理
            Pinfo.push_back(false);
        }
        Transformed_vertices.emplace_back(Transformed_P);//把转换或占位的vertex放入数组
    }
    
    if (not std::any_of(Pinfo.begin(), Pinfo.end(), [](bool value) { return value; })) return;//直接跳过完全不可见的mesh

    //以面为单位将每个角顶索引，并二次分割处理有不可见点的平面（一个点不可见的情况；两个点不可见的情况；三个点都不可见的情况）
    for (int i = 0; i < TargetMesh.faces.size(); ++i) {
        double vecTest[3] = { TargetMesh.vertices[TargetMesh.faces[i].index[0]].x - Receive_camera.CameraPos[0], TargetMesh.vertices[TargetMesh.faces[i].index[0]].y - Receive_camera.CameraPos[1], TargetMesh.vertices[TargetMesh.faces[i].index[0]].z - Receive_camera.CameraPos[2] };
        //剔除背面
        //bool can_see = (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) < 0 ? true : false;
        if ( (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) >= 0) continue;
        //面全可见的情况

        if (Pinfo[TargetMesh.faces[i].index[0]] && Pinfo[TargetMesh.faces[i].index[1]] && Pinfo[TargetMesh.faces[i].index[2]] ) {
            out_mesh.vertices_2d.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[0]]);
            out_mesh.vertices_2d.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[1]]);
            out_mesh.vertices_2d.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[2]]);

            out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[TargetMesh.faces[i].index[0]]);
            out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[TargetMesh.faces[i].index[1]]);
            out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[TargetMesh.faces[i].index[2]]);

            out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);

            out_mesh.color.emplace_back(TargetMesh.color[i]);
        }
        else {
            //不完整面处理(仍然检查总体面正面可见性)
            int num = static_cast<int>(not Pinfo[TargetMesh.faces[i].index[0]]) + static_cast<int>(not Pinfo[TargetMesh.faces[i].index[1]]) + static_cast<int>(not Pinfo[TargetMesh.faces[i].index[2]]);
            if (num == 3) continue;
            bool num_bool = (num == 1) ? true : false;
            
            unsigned int previous{}, next{}, medium{};//          previous Vertex, next Vertex, medium Vertex    (看情况各自分配是不可见点还是可见点)(索引值)
            Vertex ClipOut_3d_1, ClipOut_3d_2;            //          3d Vertex 1        3d Vertex 2
            Vertex ClipOut_1, ClipOut_2;            //          Vertex 1        Vertex 2
            //为了最后生成的点的数组仍然是正面顺时针的顺序
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[TargetMesh.faces[i].index[e]]) {//获得待处理点            //此处的异或运算分辨是1情况还是2情况
                    medium = TargetMesh.faces[i].index[e];
                    previous = TargetMesh.faces[i].index[(e + 5) % 3];
                    next = TargetMesh.faces[i].index[(e + 7) % 3];
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[previous], TargetMesh.vertices[medium], ClipOut_3d_1);
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[next], TargetMesh.vertices[medium], ClipOut_3d_2);
                    break;
                }
            }
            CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_1, ClipOut_1);
            CameraSpace_to_ScreenSpace(Receive_camera, screen_in, ClipOut_3d_2, ClipOut_2);

            ClipOut_1.z = 1 / Receive_camera.NearPlane; //逆深度
            ClipOut_2.z = 1 / Receive_camera.NearPlane;
            if (num_bool) {
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[previous]);
                out_mesh.vertices_2d.emplace_back(ClipOut_1);
                out_mesh.vertices_2d.emplace_back(ClipOut_2);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[previous]);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_1);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_2);

                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                out_mesh.color.emplace_back(TargetMesh.color[i]);
                
                //此处输出第二个面
                out_mesh.vertices_2d.emplace_back(ClipOut_2);
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[next]);
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[previous]);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_2);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[next]);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[previous]);
                
                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                out_mesh.color.emplace_back(TargetMesh.color[i]);
                //后续光栅化加入纹理映射时，记得在这加uv坐标转换！
            }
            else {
                //裁切得一个面的情况
                out_mesh.vertices_2d.emplace_back(ClipOut_1);
                out_mesh.vertices_2d.emplace_back(Transformed_vertices[medium]);
                out_mesh.vertices_2d.emplace_back(ClipOut_2);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_1);
                out_mesh.vertices_3d.emplace_back(TargetMesh.vertices[medium]);
                out_mesh.vertices_3d.emplace_back(ClipOut_3d_2);

                out_mesh.normal_vectors.emplace_back(TargetMesh.normal_vectors[i]);
                out_mesh.color.emplace_back(TargetMesh.color[i]);
            }
        };

    }
    return;
    //developing now
}




