#pragma once
#include "Data.h"
#include "Camera.h"
#include "Shadow.h"
#include "Lights.h"

#include <algorithm>
#include "BS_thread_pool.h"

#include <SDL.h>
#include <SDL_ttf.h>
//use SDL 2


#define ENGINE_NAME "Moon_engine_Alpha_v0.6.7"




struct ShaderContext {

    const Camera* Receive_camera = nullptr;
    long screen_in[2] = {0, 0};

    VertexBufferObject* VBO = nullptr;
    std::vector <unsigned char>* Vertices_info = nullptr;

    const Mesh* Original_Mesh = nullptr;
    std::vector<Triangle>* List = nullptr;

};

class BaseVertexShader {
protected:
    ShaderContext Context;

    inline Vec3 ToCameraSpace(const Vec3& Vertex_WorldSpace);
    inline Vertex2D CameraSpace_To_ScreenSpace(const Vec3& vertex_CameraSpace);
    Vertex2D Clip(const Vertex2D& origin_1, const Vertex2D& origin_2);
    inline void Get_FaceNorVector(VertexBufferObject& list);
    inline void Perspective(unsigned begin, unsigned end);

    inline bool TestScreenOutside(Vertex2D& v0, Vertex2D& v1, Vertex2D& v2);


public:

    Vertex2D PerspectiveOneVertex(const Camera& Receive_camera, int screen_in[2], const Vec3& target);

    virtual void Transform(const Camera& Receive_camera, const int screen_in[2], const MoonModel& MDL, std::vector<Triangle>& List);
    

};




struct GraphicContext {

    const Camera* Receive_camera = nullptr;

    BufferCollection* buffer = nullptr;
    DepthBuffer* depthbuffer = nullptr;
    splitValue* Buffer_spilit = nullptr;

    MoonTexture* ptrTexture = nullptr;
    MoonMaterial* ptrMateral0 = nullptr;

    int ChunkWidth_Begin, ChunkWidth_End, ChunkHeight_Begin, ChunkHeight_End = 0;
};


class BaseGraphics {
protected:
    GraphicContext GC;

    inline double To_unLineDepth(double depthBuffersList);

    inline void PreComputeTriangle(double coeffs_[30], double& reciprocal_area, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);
    inline Vertex2D Interporate(double coeffs_[30], double& reciprocal_area, double a, double B);


    void DrawFlatTopTriangle(bool direct, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);
    void DrawFlatBottomTriangle(bool direct, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);

public:

    void RefreshGraphicContext(const Camera& Receive_camera, BufferCollection& buffer, splitValue& Buffer_spilit);

    void DrawTriangleTexture(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, MoonTexture* ptrTexture0, MoonMaterial* ptrMtl1);

    void DrawTriangleTexture_direct(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, MoonTexture* ptrTexture0, MoonMaterial* ptrMtl1);

    void DeferredRender_AmbientLight(AmbientLight& AL);
    void DeferredRender_ParallelLight(ParallelLight& PL);
    void DeferredRender_PointLight(PointLight& PL);


};



class Graphics_forShadow : public BaseGraphics{
protected:
    

    inline Vertex2D Interporate(double coeffs_[30], double& reciprocal_area, double a, double B);

    inline void DrawFlatTopTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);
    inline void DrawFlatBottomTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);

public:
    void RefreshGraphicContext(const Camera& Receive_camera, DepthBuffer& buffer, splitValue Buffer_spilit);
    void DrawTriangleTexture(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, MoonTexture* ptrTexture0, MoonMaterial* ptrMtl1);

    void DeferredRender_AmbientLight(AmbientLight& AL) = delete;
    void DeferredRender_ParallelLight(ParallelLight& PL) = delete;
    void DeferredRender_PointLight(PointLight& PL) = delete;
};



class Load {
public:
    Mesh LoadMesh(const std::string& filename);

    MoonTexture LoadTextureImage(const std::string& filename);

};


unsigned int Render(const Camera Receive_camera, BufferCollection& FrameBuffer, const int screen_in[2], const std::vector <MoonModel>& Models_list, LightsCollection & lightsList);
unsigned int Render(const Camera Receive_camera, BufferCollection& FrameBuffer, const int screen_in[2], const std::vector <MoonModel>& Models_list);
