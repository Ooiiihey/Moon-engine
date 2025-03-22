#pragma once

#include <iostream>
#include <vector>

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
Vec3 rotate_all(const Vec3& j, Vec3 angle);



struct Vec2 {
    double u = 0, v = 0;

    inline Vec2 operator+(const Vec2& _vec_) const {
        return Vec2(u + _vec_.u, v + _vec_.v);
    }

    inline Vec2 operator-(const Vec2& _vec_) const {
        return Vec2(u - _vec_.u, v - _vec_.v);
    }

    inline Vec2 operator*(const Vec2& _vec_) const {
        return Vec2(u * _vec_.u, v * _vec_.v);
    }

    inline Vec2 operator/(const Vec2& _vec_) const {
        return Vec2(u / _vec_.u, v / _vec_.v);
    }

    inline Vec2 operator+(double num) const {
        return Vec2(u + num, v + num);
    }

    inline Vec2 operator-(double num) const {
        return Vec2(u - num, v - num);
    }

    inline Vec2 operator*(double num) const {
        return Vec2(u * num, v * num);
    }

    inline Vec2 operator/(double num) const {
        return Vec2(u / num, v / num);
    }
};

//2D顶点
struct Vertex2D {
    double x = 0, y = 0;
    Vec3 v3D;
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
    unsigned int index[3] = { 0, 0, 0 };
};




//RGBa
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

    RGBa operator/(const RGBa& other) const {
        return { R / other.R, G / other.G, B / other.B, a / other.a };
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
    std::vector <Face> facesIndex = {};

    std::vector <Face> texIndex = {};
    std::vector <Vec2> UVCoords = {};

    void VectorMovePos(Vec3 pos);

};






//developing now
class MoonTexture {
public:
    std::vector <RGBa> Pixels = {};
    unsigned int width = 0, height = 0;

    void SetTexture(unsigned int width_in, unsigned int height_in);
    RGBa getPixel(double u, double v);

};


class MoonMaterial {
public:
    bool SmoothShader = true;

    // 材质环境光吸收
    // 此属性用来调整环境光的吸收强度，而不是ka
    RGBa ambientColor = { 1.0f, 1.0f, 1.0f };

    // 材质漫反射系数 (Kd)
    RGBa diffuseColor = { 0.7f, 0.7f, 0.7f };

    // 材质镜面反射系数 (Ks)
    RGBa specularColor = { 0.3f, 0.3f, 0.3f };

    // 材质镜面反射指数 (Ns)
    double specularExponent = 4.0;

    RGBa SelfColor = { 0.5, 0.5, 0.5 };
};



class MoonModel {
public:
    Mesh* ptrOriginalMesh = nullptr;
    Mesh WorldSpaceMesh;

    MoonTexture* ptrTexture = nullptr;
    MoonMaterial* ptrMaterial = nullptr;

    double size = 1.0;
    Vec3 Positon;


    Vec3 X = { 1, 0, 0 },
             Y = { 0, 1, 0 },
             Z = { 0, 0, 1 };

    void LinkMesh(Mesh& m);
    void linkMaterial(MoonMaterial& mtl);
    void linkTexture(MoonTexture& tex);

    void ZoomSize(double alpha);
    void SetMeshPos(Vec3 pos);
    void RotateMesh(Vec3 angle);
    void UpdateMesh();
};

class Triangle {
public:
    Vertex2D v0, v1, v2;
    MoonTexture* ptrTexture = nullptr;
    MoonMaterial* ptrMtl = nullptr;
};


//old codes, dont use it
//转换出来的三角形组
struct TriangleList {
    std::vector <Vertex2D> vertices = {};

    std::vector<MoonTexture*> ptrTexture = {};
    std::vector<MoonMaterial*> ptrMtl = {};

};

struct splitValue {
    double widthBegin = 0.0;
    double widthEnd = 1.0;
    double heightBegin = 0.0;
    double heightEnd = 1.0;
};

class DepthBuffer {
public:
    int Buffer_size[2] = { 0, 0 };
    std::vector<double> Depth;

    void SetBuffer(int width, int height);
    void CleanBuffer();

    void PutDepth(const int x, const int y, double depthBuffersList);
    double GetDepth(const int x, const int y);
    bool CompareDepth_Smaller(const int x, const int y, double depthBuffersList);

};


class BufferCollection {
public:

    int Buffer_size[2] = { 0, 0 };

    std::vector<RGBa> TextureColor;
    std::vector<double> Depth;
    std::vector<Vec3> Cam3DVertex;
    std::vector<Vec3> NorVec;
    std::vector<MoonMaterial*> ptrMaterial;
    std::vector<RGBa> Frame;

    BufferCollection() = default;
    BufferCollection& operator=(const BufferCollection& other) {
        if (this != &other) {
            Buffer_size[0] = other.Buffer_size[0];
            Buffer_size[1] = other.Buffer_size[1];
            TextureColor = other.TextureColor;
            Depth = other.Depth;
            Cam3DVertex = other.Cam3DVertex;
            NorVec = other.NorVec;
            ptrMaterial = other.ptrMaterial;
        }
        return *this;
    }
    BufferCollection(BufferCollection&& other) noexcept :
        Buffer_size{ other.Buffer_size[0], other.Buffer_size[1] },
        TextureColor(std::move(other.TextureColor)),
        Depth(std::move(other.Depth)),
        Cam3DVertex(std::move(other.Cam3DVertex)),
        NorVec(std::move(other.NorVec)),
        ptrMaterial(std::move(other.ptrMaterial))
    {
    }


    void SetBuffer(int width, int height);
    void CleanBuffer();

    void PutPixelData(const int x, const int y, double depthBuffersList, RGBa c, Vec3& Cam3Dvertex, Vec3& NorVec, MoonMaterial* ptrmtl);
    void AddFinalColor(const int x, const int y, RGBa Color);
    void PutFinalColor(const int x, const int y, RGBa Color);

    RGBa GetPixelTextureColor(const int x, const int y);
    RGBa GetFramePixelColor(const int x, const int y);
    MoonMaterial* GetPtrMtl(const int x, const int y);
    Vec3 GetCam3Dvertex(const int x, const int y);
    Vec3 GetNorVec(const int x, const int y);

    bool CompareDepth_Smaller(const int x, const int y, double depthBuffersList);
    double GetDepth(const int x, const int y);

    void merge(const BufferCollection& Buffer_chunk);

};