#pragma once
#include <iostream>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include "BS_thread_pool.h"
#include <future> 

#include <SDL.h>
#include <SDL_ttf.h>
//use SDL 2

#define ENGINE_NAME "Moon_engine_Alpha_v0.5.8"
#define PI 3.14159265358979



//数据类型

struct Vec3 {
    double x = 0, y = 0, z = 0;

    Vec3() = default;
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& _vec_) const {
        return Vec3(x + _vec_.x, y + _vec_.y, z + _vec_.z);
    }

    Vec3 operator-(const Vec3& _vec_) const {
        return Vec3(x - _vec_.x, y - _vec_.y, z - _vec_.z);
    }

    Vec3 operator*(double num) const {
        return Vec3(x * num, y * num, z * num);
    }

    Vec3 operator/(double num) const {
        return Vec3(x / num, y / num, z / num);
    }

    Vec3& operator+=(const Vec3& _vec_) {
        x += _vec_.x;
        y += _vec_.y;
        z += _vec_.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& _vec_) {
        x -= _vec_.x;
        y -= _vec_.y;
        z -= _vec_.z;
        return *this;
    }

    Vec3& operator*=(double num) {
        x *= num;
        y *= num;
        z *= num;
        return *this;
    }

    Vec3& operator/=(double num) {
        x /= num;
        y /= num;
        z /= num;
        return *this;
    }
};

struct Vec2 {
    double u = 0, v = 0;
};

//2D顶点
struct  Vertex2D{
    double x = 0, y = 0, x3d = 0, y3d = 0, z3d = 0, u = 0, v = 0;
};

class VerticesData {
public:
    std::vector <Vertex2D> vertex2d;
    std::vector <Vec3> norVec;

    void put3DIndex(Vec3& in, unsigned int index);
    Vec3 get3D(unsigned int index);
};

//面
struct Face {
    unsigned int index[3] = { 0,0,0 };
};

//RGB 
struct Color {
    double R = 0, G = 0, B = 0, a = 0;
};

//原始mesh
struct Mesh {
    std::vector <Vec3> vertices = {};
    std::vector <Face> faces = {};

    std::vector <Color> color = {};
};

//用来存转换后的mesh
struct Mesh_R {
    std::vector <Vertex2D> vertices = {};
    std::vector <Vec3> normal_vectors = {};

    std::vector <Color> color = {};
};




/*old codes
struct Camera {
    //HFOV水平视野  
    double FOV = 40 * PI / 180;
    double F = 1;    //f焦距(默认1)
    const double NearPlane = 0.04;   //近平面距离(运行时不可变)
    const double FarPlane = 4000;

    double Pos[3] = { 0, 0, 0 };  //摄像机位置（实时）

    const double Forward[3] = { NearPlane, 0, 0 },
        y[3] = { NearPlane, 1, 0 },
        z[3] = { NearPlane, 0, 1 },
        move[3] = { 0, -1, 0 };  //移动辅助点(固定值)
    //以camera为参照的初始相机方向坐标（固定值不可更改！）

    //存储实时方向向量
    Vec3 Forward_vec = { Forward[0], Forward[1], Forward[2] },
        Y_vec = { y[0], y[1], y[2] },
        Z_vec = { z[0], z[1], z[2] },
        move_vec = { move[0], move[1], move[2] };
};
*/

double KQsqrt(double number);
double dot(const Vec3& vec1, const Vec3& vec2);
double dot(const Vec3& vec1, const double vec2[3]);
Vec3 cross(const Vec3& vec1, const Vec3& vec2);
double GetLength(const Vec3& vec);
double GetLength(const double vec[3]);//重载函数
Vec3 rotate(const Vec3& p, const Vec3& k, const double angle);


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

    void Set_Direction(const double a, const double b, const double c);

};


class Buffer {
public:
    std::mutex mtx;
    long Buffer_size[2] = {0, 0};
    std::vector<float> Red;
    std::vector<float> Green;
    std::vector<float> Blue;
    std::vector<float> Alapha;
    std::vector<float> Depth;

    // 默认构造函数
    Buffer() = default;

    // 显式定义拷贝构造函数
    Buffer(const Buffer& other): 
        Buffer_size{ other.Buffer_size[0], other.Buffer_size[1] },
        Red(other.Red),
        Green(other.Green),
        Blue(other.Blue),
        Depth(other.Depth) {
    }

    // 显式定义拷贝赋值运算符
    Buffer& operator=(const Buffer& other) {
        if (this != &other) {
            Buffer_size[0] = other.Buffer_size[0];
            Buffer_size[1] = other.Buffer_size[1];
            Red = other.Red;
            Green = other.Green;
            Blue = other.Blue;
            Depth = other.Depth;
        }
        return *this;
    }


    // 显式定义移动赋值运算符
    Buffer(Buffer&& other) noexcept : 
        Buffer_size{ other.Buffer_size[0], other.Buffer_size[1] },
        Red(std::move(other.Red)),
        Green(std::move(other.Green)),
        Blue(std::move(other.Blue)),
        Depth(std::move(other.Depth)) {
    }


    void SetBuffer(long width, long height);
    void CleanBuffer();

    void PutPixel(long x, long y, double deepth, Color c);
    Color GetPixelColor(const long x, const long y);

    bool CompareDepth_Smaller(const long x, const long y, double depth);

    double GetDepth(const long x, const long y);

    void merge(const Buffer& Buffer_chunk);
};



class Transform {
private:
    inline Vec3 To_CameraSpace(const Camera& Receive_camera, const Vec3& Vertex_WorldSpace);
    inline Vec3 Get_CrossPoint(const Camera& Receive_camera, const Vec3& origin_1, const Vec3& origin_2);
    inline Vertex2D CameraSpace_to_ScreenSpace(const Camera& Receive_camera, const long screen_in[2], const Vec3& vertex_origin);
    inline void Get_NormalVector(Mesh& cMesh, VerticesData& list);
public:
    
    void Perspective(const Camera& Receive_camera, const long screen_in[2], Mesh& TargetMesh, Mesh_R& out_mesh);


};



class Graphics {
private:
    //临时项
    double coeffs_[21];
    double reciprocal_area;

    inline double To_unLineDepth(const Camera& Rec_camera, double depth);

    inline void PreComputeTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2);
    inline void Interporate(double a, double b, Vertex2D& back);

	inline void DrawFlatTopTriangle(const Camera& Receive_camera_2, Buffer& buffer, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, const Color& c);
    inline void DrawFlatBottomTriangle(const Camera& Receive_camera_2, Buffer& buffer, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, const Color& c);
public:
    void DrawTriangle(const Camera &Receive_camera_2, Buffer& buffer, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, Color& c);


};

void Render(const Camera Receive_camera, Buffer& FrameBuffer, const long screen_in[2], const std::vector <Mesh>& mesh_list);
