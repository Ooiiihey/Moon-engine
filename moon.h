#pragma once
#include <vector>


//数据类型
struct Point {
    double x = 0, y = 0, z = 0;
};

struct Face {
    unsigned int index[3] = { 0,0,0 };
};

struct Mmesh {
    std::vector <Point> vertices = {};
    std::vector <Face> faces = {};
    std::vector <Point> normal_vectors = {};
    std::vector <Point> textures_uv = {};
};

//RGB 
struct Color {
    float R = 0, G = 0, B = 0 ,a = 0;
};

//光栅化会使用的
struct Pixel {
    //用来存深度
    double deepth;
    Color color;
};

double SSE2Qsqrt(double number);
double KQsqrt(double number);

class vectorMath {
public:
    double GetLength(const Point &vec);
    double GetLength(const double vec[3]);//重载函数
    double dot(const Point &vec1, const Point &vec2);
    void cross(const Point& vec1, const Point& vec2, Point &vec_cross);
};










class Rasterization_function {
private:
    static std::vector <std::vector <Pixel>> Frame_Buffer;
public:
    void SetFrameBuffer(unsigned int width, unsigned int height);
    void PutPixel(double x, double y, double deepth, Color color);
	void DrawFlatTopTriangle();

	void DrawTriangle(double* pv1, double* pv2, double* pv3, Color& c);


};

