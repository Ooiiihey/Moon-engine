#pragma once
#include <iostream>
#include <vector>
#include <algorithm>

#include "BS_thread_pool.h"

#include <SDL.h>
#include <SDL_ttf.h>
//use SDL 2



#define ENGINE_NAME "Moon_engine_Alpha_v0.6.0"
#define PI 3.14159265358979

double KQsqrt(double number);

//数据类型

struct Vec3 {
    double x = 0, y = 0, z = 0;

    Vec3() = default;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    inline Vec3 operator+(const Vec3& _vec_) const {
        return Vec3(x + _vec_.x, y + _vec_.y, z + _vec_.z);
    }

    inline Vec3 operator-(const Vec3& _vec_) const {
        return Vec3(x - _vec_.x, y - _vec_.y, z - _vec_.z);
    }

    inline Vec3 operator*(double num) const {
        return Vec3(x * num, y * num, z * num);
    }

    inline Vec3 operator/(double num) const {
        return Vec3(x / num, y / num, z / num);
    }

    inline Vec3& operator+=(const Vec3& _vec_) {
        x += _vec_.x;
        y += _vec_.y;
        z += _vec_.z;
        return *this;
    }

    inline Vec3& operator-=(const Vec3& _vec_) {
        x -= _vec_.x;
        y -= _vec_.y;
        z -= _vec_.z;
        return *this;
    }

    inline Vec3& operator*=(double num) {
        x *= num;
        y *= num;
        z *= num;
        return *this;
    }

    inline Vec3& operator/=(double num) {
        x /= num;
        y /= num;
        z /= num;
        return *this;
    }

    double length() const {
        return KQsqrt(x * x + y * y + z * z);
    }

    // 单位向量化
    Vec3& normalize() {
        double len = length();
        if (len != 0) {
            x /= len;
            y /= len;
            z /= len;
        }
        return *this;
    }

};

//math

double dot(const Vec3& vec1, const Vec3& vec2);
double dot(const Vec3& vec1, const double vec2[3]);
Vec3 cross(const Vec3& vec1, const Vec3& vec2);

Vec3 rotate(const Vec3& p, const Vec3& k, const double angle);
Vec3 rotate_all(const Vec3& j, Vec3& angle);



struct Vec2 {
    double u = 0, v = 0;
};

//2D顶点
struct Vertex2D{
    double x = 0, y = 0;
    Vec3 _3D;
    Vec2 UVCoords;
    Vec3 norVector;
};

//顶点缓冲对象
class VertexBufferObject {
public:
    std::vector <Vertex2D> vertex2d;
    std::vector <Vec3> FaceNorVec;

    void put3DIndex(Vec3& in, unsigned int index);
    Vec3 get3D(unsigned int index);
};

//面
struct Face {
    unsigned int index[3] = { 0,0,0 };
};

//RGB 
struct RGBa {
    float R = 0, G = 0, B = 0, a = 0;

 

    bool operator==(RGBa otherC) {
        if (R == otherC.R && G == otherC.G && B == otherC.B && a == otherC.a) {
            return true;
        }
        else
        {
            return false;
        }
    }


    RGBa operator*(const RGBa& other) const {
        return { R * other.R, G * other.G, B * other.B, a * other.a };
    }

    // 颜色加法运算符
    RGBa operator+(const RGBa& other) const {
        return { R + other.R, G + other.G, B + other.B, a + other.a };
    }

    // 颜色除法运算符
    RGBa operator/(float scalar) const {
        return { R / scalar, G / scalar, B / scalar, a / scalar };
    }

    // 颜色拷贝操作
    RGBa& operator=(const RGBa& other) {
        if (this != &other) {
            R = other.R;
            G = other.G;
            B = other.B;
            a = other.a;
        }
        return *this;
    }

};

//mesh
class Mesh {
public:
    std::vector <Vec3> vertices = {};
    std::vector <Face> faces = {};

    std::vector <Face> facesUVindex = {};
    std::vector <Vec2> UVCoords = {};

    //std::vector <RGBa> color = {};

    void VectorMovePos(Vec3 pos);
    
};



class ParallelLight {
public:
    Vec3 direction = { -0.5, -0.5, -0.3 };
    RGBa LightColor = { 0.3, 0.3, 0.3 };

};


//developing now
class Texture {
private:
    std::vector <RGBa> Pixels = {};

public:
    unsigned int width = 0, height = 0;
    void SetTexture(unsigned int width_in, unsigned int height_in);
    RGBa getPixel(unsigned int u, unsigned int v);

};


class Material {
public:

    // 材质环境光系数 (Ka)
    RGBa ambientColor = { 0.4f, 0.4f, 0.4f };

    // 材质漫反射系数 (Kd)
    RGBa diffuseColor = { 0.7f, 0.7f, 0.7f };

    // 材质镜面反射系数 (Ks)
    RGBa specularColor = { 0.3f, 0.3f, 0.3f };

    // 材质镜面反射指数 (Ns)
    double specularExponent = 4.0;

    RGBa SelfColor = {0.5, 0.5, 0.5};
};



class Model {
public:
    Mesh* ptrOriginalMesh = nullptr;
    Mesh WorldSpaceMesh;

    Texture* ptrTexture = nullptr;
    Material* ptrMaterial = nullptr;

    double size = 1.0;
    Vec3 Positon;

    Vec3 X = {1, 0, 0},
            Y = {0, 1, 0},
            Z = {0, 0, 1};

    void LinkMesh(Mesh& m);
    void LinkMaterial(Material& mtl);
    void ZoomSize(double alpha);
    void SetMeshPos(Vec3 pos);
    void RotateMesh(Vec3 angle);
    void UpdateMesh();
};

//转换出来的三角形组
struct TriangleList {
    std::vector <Vertex2D> vertices = {};

    std::vector<Texture*> ptrTexture = {};
    std::vector<Material*> ptrMtl = {};

};

struct splitValue {
    double widthBegin = 0.0;
    double widthEnd = 1.0;
    double heightBegin = 0.0;
    double heightEnd = 1.0;
};



//camera结构数据类型
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


class Buffer {
public:

    long Buffer_size[2] = {0, 0};

    std::vector<RGBa> PixelColor;
    std::vector<double> Depth;
    std::vector<Vec3> NorVector;

    Buffer() = default;
    
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            Buffer_size[0] = other.Buffer_size[0];
            Buffer_size[1] = other.Buffer_size[1];
            PixelColor = other.PixelColor;
            Depth = other.Depth;
            NorVector = other.NorVector;
        }
        return *this;
    }

    Buffer(Buffer&& other) noexcept : 
        Buffer_size{ other.Buffer_size[0], other.Buffer_size[1] },
        PixelColor(std::move(other.PixelColor)),
        Depth(std::move(other.Depth)),
        NorVector(std::move(other.NorVector))
    {
    }


    void SetBuffer(long width, long height);
    void CleanBuffer();

    void PutPixel(long x, long y, double depth, RGBa c, Vec3& NorVec);
    RGBa GetPixelColor(const long x, const long y);

    bool CompareDepth_Smaller(const long x, const long y, double depth);

    double GetDepth(const long x, const long y);

    void merge(const Buffer& Buffer_chunk);
};




struct RenderContext {

    const Camera* Receive_camera = nullptr;
    long screen_in[2];

    VertexBufferObject* VBO = nullptr;
    std::vector <unsigned char>* Vertices_info = nullptr;

    const Mesh* Original_Mesh = nullptr;
    TriangleList* out = nullptr;

};

class Transform {
private:
    RenderContext Context;

    inline Vec3 ToCameraSpace(const Vec3& Vertex_WorldSpace);
    //Vec3 ToCameraSpace_Normal(const Vec3& normal);
    inline Vertex2D Clip(const Vertex2D& origin_1, const Vertex2D& origin_2);

    inline void Get_FaceNormalVector(VertexBufferObject& list);
    inline Vertex2D CameraSpace_To_ScreenSpace(const Vec3& vertex_CameraSpace);

    inline void Perspective(unsigned begin, unsigned end);
    inline bool TestScreenOutside(Vertex2D& v0, Vertex2D& v1, Vertex2D& v2);

    
public:
    void VertexShader(const Camera& Receive_camera, const long screen_in[2], const Model& MDL, TriangleList& out);


};


struct GraphicContext {

    const Camera* Receive_camera = nullptr;

    Buffer* buffer = nullptr;
    splitValue* Buffer_spilit = nullptr;

    Texture* ptrTexture = nullptr;
    Material* ptrMateral0 = nullptr;

    ParallelLight PL;
};


class Graphics {
private:
    GraphicContext GC;

    double To_unLineDepth(double depth);

    Vec3 ToCameraSpace_Normal(const Vec3& normal);

    void PreComputeTriangle(double coeffs_[30], double& reciprocal_area, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);
    void Interporate(double coeffs_[30], double& reciprocal_area, double a, double B, Vertex2D& back);

    RGBa Calculate_ParallelLight(RGBa inputColor, Vertex2D& target);

    void DrawFlatTopTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2 );
    void DrawFlatBottomTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2 );

    

public:

    void DrawTriangle(const Camera& Receive_camera, Buffer& buffer, splitValue& Buffer_spilit, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2,Texture* ptrTexture0, Material* ptrMtl1, const ParallelLight& PL0);


};


class Load {
public:
    Mesh LoadMesh(const std::string& filename);
};


unsigned int Render(const Camera Receive_camera, Buffer& FrameBuffer, const long screen_in[2], const std::vector <Model>& Models_list, const ParallelLight& light);
