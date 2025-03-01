#include "moon.h"
BS::thread_pool Calculatepool(4);

inline Vec3 Transform::ToCameraSpace(const Vec3& Vertex_WorldSpace) {
    Vec3 vec_P = Vertex_WorldSpace - Context.Receive_camera->Pos;
    return Vec3(dot(vec_P, Context.Receive_camera->Forward_vec),
                        dot(vec_P, Context.Receive_camera->Y_vec),
                        dot(vec_P, Context.Receive_camera->Z_vec) );
};



//计算线在近平面的交点坐标                          相机                            
 inline Vertex2D Transform::Clip(const Vertex2D& origin_1, const Vertex2D& origin_2) {
     double alpha = (Context.Receive_camera->NearPlane - origin_1._3D.x) / (origin_2._3D.x - origin_1._3D.x);

     Vec3 tmp = { Context.Receive_camera->NearPlane,
                           (origin_2._3D.y - origin_1._3D.y) * alpha + origin_1._3D.y,
                           (origin_2._3D.z - origin_1._3D.z) * alpha + origin_1._3D.z };


     Vertex2D back = CameraSpace_To_ScreenSpace(tmp);

     back.norVector = ((origin_2.norVector - origin_1.norVector )* alpha + origin_1.norVector).normalize();

     return std::move(back);

 }

// 计算面的法向量                                                  mesh
inline void Transform::Get_FaceNormalVector(VertexBufferObject& list) {
    Vec3 normal_vectors, veca, vecb;
    
    for (const Face& each_face : Context.Original_Mesh->faces) {

        veca = Context.Original_Mesh->vertices[each_face.index[1]] - Context.Original_Mesh->vertices[each_face.index[0]];
        vecb = Context.Original_Mesh->vertices[each_face.index[2]] - Context.Original_Mesh->vertices[each_face.index[1]];
        normal_vectors = cross(veca, vecb);
        normal_vectors.normalize();

        list.FaceNorVec.emplace_back(normal_vectors);
    }
    return;
}


inline Vertex2D Transform::CameraSpace_To_ScreenSpace(const Vec3& vertex_CameraSpace) {
    Vec3 VecPlane = vertex_CameraSpace * (Context.Receive_camera->NearPlane / vertex_CameraSpace.x);
    double b = (Context.Receive_camera->F * (Context.screen_in[0] >> 1)) / (tan(Context.Receive_camera->FOV) * Context.Receive_camera->NearPlane);
    //这里会装载上CameraSpace的坐标
    return Vertex2D( (Context.screen_in[0] >> 1) - (b * VecPlane.y),
                                (Context.screen_in[1] >> 1) - (b * VecPlane.z),
                                vertex_CameraSpace,     //x3d存储camera_Space中深度
                                Vec2(0, 0 ) );
    //平面映射,坐标已适配direcX坐标系

}

inline void Transform::Perspective(unsigned begin, unsigned end) {
    double b = (Context.Receive_camera->F * (Context.screen_in[0] >> 1)) / (tan(Context.Receive_camera->FOV) * Context.Receive_camera->NearPlane);
    for (; begin < end; ++begin) {
        Vertex2D Transformed_P;
        Vec3 CameraSpace_P = ToCameraSpace(Context.Original_Mesh->vertices[begin]);//转换到camera space

        if (CameraSpace_P.x >= Context.Receive_camera->NearPlane) {
            (*Context.Vertices_info)[begin] = true;
            Vec3 VecPlane = CameraSpace_P * (Context.Receive_camera->NearPlane / CameraSpace_P.x);

            
            Transformed_P = { (Context.screen_in[0] >> 1) - (b * VecPlane.y),
                                        (Context.screen_in[1] >> 1) - (b * VecPlane.z),
                                        CameraSpace_P,
                                        Vec2(0, 0) };

        }
        else {
            //不可见情况
            Transformed_P._3D = CameraSpace_P;
        }
        Context.VBO->vertex2d[begin] = Transformed_P;
    }
}



inline bool Transform::TestScreenOutside(Vertex2D& v0, Vertex2D& v1, Vertex2D& v2) {
    double triMinX = std::min({ v0.x, v1.x, v2.x });
    double triMaxX = std::max({ v0.x, v1.x, v2.x });
    double triMinY = std::min({ v0.y, v1.y, v2.y });
    double triMaxY = std::max({ v0.y, v1.y, v2.y });

    if (triMaxX < 0 || triMinX > Context.screen_in[0] || triMaxY < 0 || triMinY > Context.screen_in[1]) {
        return true;
    }
    else return false;
}


//face组装+透视转换                                               摄像机                           分辨率                     原始model                  输出mesh
void Transform::VertexShader(const Camera& Receive_camera, const long screen_in[2], const Model &MDL, TriangleList& out) {

    unsigned int vertexNUM = MDL.WorldSpaceMesh.vertices.size();
    unsigned int faceNUM = MDL.WorldSpaceMesh.faces.size();

    //此处不使用bool类型是因为多线程下更改bool值会有问题！
    std::vector <unsigned char>Vertices_info;//点可见性检查

    Vertices_info.resize(vertexNUM, false);
    out.vertices.reserve(faceNUM * 3);

    VertexBufferObject VBO;
    VBO.vertex2d.resize(vertexNUM, Vertex2D{});

    Context.Receive_camera = &Receive_camera;
    Context.screen_in[0] = screen_in[0];
    Context.screen_in[1] = screen_in[1];
    Context.VBO = &VBO;
    Context.Vertices_info = &Vertices_info;
    Context.Original_Mesh = &MDL.WorldSpaceMesh;
    Context.out = &out;

    if (vertexNUM < 2000) {
        Perspective(0, vertexNUM);
    }
    else {
        Calculatepool.submit_blocks(0, vertexNUM, [&](unsigned int begin, unsigned int end) {
            Perspective(begin, end);
            });
        Calculatepool.wait();
    }
    


    //直接跳过完全不可见的mesh
    if (not std::any_of(Vertices_info.begin(), Vertices_info.end(), [](bool value) { return value; })) return;

    //计算法向量
    VBO.FaceNorVec.reserve(faceNUM);
    Get_FaceNormalVector(VBO);

    for (unsigned int INDEX = 0; INDEX < faceNUM; ++INDEX) {
        const Face* ptrFace = &MDL.WorldSpaceMesh.faces[INDEX];
        VBO.vertex2d[ptrFace->index[0]].norVector += VBO.FaceNorVec[INDEX];
        VBO.vertex2d[ptrFace->index[1]].norVector += VBO.FaceNorVec[INDEX];
        VBO.vertex2d[ptrFace->index[2]].norVector += VBO.FaceNorVec[INDEX];
    }

    for (unsigned int num = 0; num < vertexNUM; ++num) {
        if (VBO.vertex2d[num].norVector.length() == 0) continue;
        VBO.vertex2d[num].norVector.normalize();
    }



    //此处组装三角形有·性能问题，找时间把这里优化和多线程化了
    //组装三角形，并二次分割处理有不可见点的平面（一个点不可见的情况；两个点不可见的情况；三个点都不可见的情况）
    for (unsigned int i = 0; i < faceNUM; ++i) {
        const Face* ptrFace = &MDL.WorldSpaceMesh.faces[i];
        unsigned int index0 = ptrFace->index[0];
        if (dot( (MDL.WorldSpaceMesh.vertices[index0] - Receive_camera.Pos), VBO.FaceNorVec[i]) >= 0) continue;

        unsigned int index1 = ptrFace->index[1];
        unsigned int index2 = ptrFace->index[2];

        int num = (!Vertices_info[index0]) + (!Vertices_info[index1]) + (!Vertices_info[index2]);
        if (num == 3) continue;

        //面全可见的情况
        if (num == 0) {
            //后面此处加入uv索引的结果
            if (TestScreenOutside(VBO.vertex2d[index0], VBO.vertex2d[index1], VBO.vertex2d[index2])) continue;

            out.vertices.emplace_back(VBO.vertex2d[index0]);
            out.vertices.emplace_back(VBO.vertex2d[index1]);
            out.vertices.emplace_back(VBO.vertex2d[index2]);
            
            //out.normal_vectors.emplace_back(VBO.FaceNorVec[i]);
            out.ptrTexture.emplace_back(MDL.ptrTexture);
            out.ptrMtl.emplace_back(MDL.ptrMaterial);
        }
        else {
            //不完整面处理
            bool num_bool = (num == 1);

            unsigned int previous{}, next{}, medium{};//          previous Vec3, next Vec3, medium Vec3    (看情况各自分配是不可见点还是可见点)(索引值)
            //Vec3 ClipOut_3d_1, ClipOut_3d_2;            //          3d Vec3 1        3d Vec3 2
            Vertex2D ClipOut_1, ClipOut_2;            //          Vec3 1        Vec3 2

            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Vertices_info[ptrFace->index[e]]) {//获得待处理点            //此处的异或运算分辨是1情况还是2情况
                    medium = ptrFace->index[e];
                    previous = ptrFace->index[(e + 2) % 3];
                    next = ptrFace->index[(e + 1) % 3];
                    break;
                }
            }

            /*
            ClipOut_3d_1 = Clip(VBO.vertex2d[previous], VBO.vertex2d[medium]);
            ClipOut_3d_2 = Clip(VBO.vertex2d[next], VBO.vertex2d[medium]);
            ClipOut_1 = CameraSpace_To_ScreenSpace(ClipOut_3d_1);
            ClipOut_2 = CameraSpace_To_ScreenSpace(ClipOut_3d_2);
            */
            ClipOut_1 = Clip(VBO.vertex2d[previous], VBO.vertex2d[medium]);
            ClipOut_2 = Clip(VBO.vertex2d[next], VBO.vertex2d[medium]);
            

            if (num_bool) {
                //后续光栅化加入纹理映射时，记得在这加uv坐标转换！
                if (!TestScreenOutside(ClipOut_1, ClipOut_2, VBO.vertex2d[previous])) {
                    out.vertices.emplace_back(VBO.vertex2d[previous]);
                    out.vertices.emplace_back(ClipOut_1);
                    out.vertices.emplace_back(ClipOut_2);

                    //out.normal_vectors.emplace_back(VBO.FaceNorVec[i]);
                    out.ptrTexture.emplace_back(MDL.ptrTexture);
                    out.ptrMtl.emplace_back(MDL.ptrMaterial);

                }

                
                //此处输出第二个面
                if (!TestScreenOutside(ClipOut_2, VBO.vertex2d[next], VBO.vertex2d[previous])) {
                    out.vertices.emplace_back(ClipOut_2);
                    out.vertices.emplace_back(VBO.vertex2d[next]);
                    out.vertices.emplace_back(VBO.vertex2d[previous]);

                    //out.normal_vectors.emplace_back(VBO.FaceNorVec[i]);
                    out.ptrTexture.emplace_back(MDL.ptrTexture);
                    out.ptrMtl.emplace_back(MDL.ptrMaterial);
                }
                
            }
            else {
                //裁切得一个面的情况
                if (!TestScreenOutside(ClipOut_1, ClipOut_2, VBO.vertex2d[medium])){
                    out.vertices.emplace_back(ClipOut_1);
                    out.vertices.emplace_back(VBO.vertex2d[medium]);
                    out.vertices.emplace_back(ClipOut_2);

                    //out.normal_vectors.emplace_back(VBO.FaceNorVec[i]);
                    out.ptrTexture.emplace_back(MDL.ptrTexture);
                    out.ptrMtl.emplace_back(MDL.ptrMaterial);
                }

            }
        };

    }

    return;
    //developing now
}

