#pragma once
#include "Data.h"
#include <algorithm>

#include "BS_thread_pool.h"

#include <SDL.h>
#include <SDL_ttf.h>
//use SDL 2


#define ENGINE_NAME "Moon_engine_Alpha_v0.6.2"
#define PI 3.14159265358979


//camera
class Camera {
public:
    const double NearPlane = 0.04;
    double FarPlane = 4000;
    double FOV = 45 * PI / 180;
    double F = 1;    //f焦距
    Vec3 Pos = { 0, 0, 0 };

    // 初始化方向向量为默认坐标系
    const Vec3 Forward = { 1, 0, 0 },
        Y = { 0, 1, 0 },
        Z = { 0, 0, 1 },
        move = { 0, -1, 0 };  //移动辅助点(固定值)
    //以camera为参照的初始相机方向坐标（不可更改！）

    Vec3 Forward_vec = { 1, 0, 0 },
        Y_vec = { 0, 1, 0 },
        Z_vec = { 0, 0, 1 },
        move_vec = { 0, -1, 0 };

    void Set_CameraPos(const double x, const double y, const double z);
    void Move_ForwardBack(const double step);
    void Move_LeftRight(const double step);
    void Move_UpDown(const double step);

    void Set_Direction(const double a, const double B, const double c);

};






struct ShaderContext {

    const Camera* Receive_camera = nullptr;
    long screen_in[2];

    VertexBufferObject* VBO = nullptr;
    std::vector <unsigned char>* Vertices_info = nullptr;

    const Mesh* Original_Mesh = nullptr;
    TriangleList* out = nullptr;

};

class VertexShader {
private:
    ShaderContext Context;

    inline Vec3 ToCameraSpace(const Vec3& Vertex_WorldSpace);
    //Vec3 ToCameraSpace_Normal(const Vec3& normal);
    inline Vertex2D Clip(const Vertex2D& origin_1, const Vertex2D& origin_2);

    inline void Get_FaceNorVector(VertexBufferObject& list);
    inline Vertex2D CameraSpace_To_ScreenSpace(const Vec3& vertex_CameraSpace);

    inline void Perspective(unsigned begin, unsigned end);
    inline bool TestScreenOutside(Vertex2D& v0, Vertex2D& v1, Vertex2D& v2);

    
public:
    void transform(const Camera& Receive_camera, const long screen_in[2], const MoonModel& MDL, TriangleList& out);


};




struct GraphicContext {

    const Camera* Receive_camera = nullptr;

    MoonBuffer* buffer = nullptr;
    splitValue* Buffer_spilit = nullptr;

    MoonTexture* ptrTexture = nullptr;
    MoonMaterial* ptrMateral0 = nullptr;

    ParallelLight PL;

    int ChunkWidth_Begin, ChunkWidth_End, ChunkHeight_Begin, ChunkHeight_End;
};


class Graphics {
private:
    GraphicContext GC;

    

    double To_unLineDepth(double depth);


    void PreComputeTriangle(double coeffs_[30], double& reciprocal_area, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);
    void Interporate(double coeffs_[30], double& reciprocal_area, double a, double B, Vertex2D& back);

    RGBa Calculate_ParallelLight(RGBa inputColor, Vertex2D& target);

    RGBa CalculateLight(RGBa inputColor, const Vec3& targetCam3D, const Vec3& norVec, const MoonMaterial* ptrMtl);

    void DrawFlatTopTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2 );
    void DrawFlatBottomTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2 );

    

public:

    void Refresh_GraphicContext(const Camera& Receive_camera, MoonBuffer& buffer, splitValue& Buffer_spilit, const ParallelLight& PL0);
    void DrawTriangleTex(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, MoonTexture* ptrTexture0, MoonMaterial* ptrMtl1);

    void Deferred_RenderLight();

};


class Load {
public:
    Mesh LoadMesh(const std::string& filename);

    MoonTexture LoadTextureImage(const std::string& filename);

};


unsigned int Render(const Camera Receive_camera, MoonBuffer& FrameBuffer, const long screen_in[2], const std::vector <MoonModel>& Models_list, const ParallelLight& light);
