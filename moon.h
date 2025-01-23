#pragma once
#include <vector>
#define PI 3.14159265358979

//数据类型
//顶点
struct vertex {
    double x = 0, y = 0, z = 0;
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
struct Mmesh {
    std::vector <vertex> vertices = {};
    std::vector <Face> faces = {};
    std::vector <vertex> normal_vectors = {};
    std::vector <vertex> textures_uv = {};
    std::vector <Color> color = {};
};

//用来存转换后的mesh
struct mesh_tf {
    std::vector <vertex> vertices_2d = {};
    std::vector <vertex> vertices_3d = {};
    std::vector <vertex> normal_vectors = {};
    std::vector <vertex> textures_uv = {};
    std::vector <Color> color = {};
};


//camera结构数据类型
struct Camera_data {
    //HFOV水平视野  
    double FOV = 40 * PI / 180;
    double Camera[3] = { 0, 0, 0 };  //摄像机位置（实时）
    const double NearPlane = 0.04;   //近平面距离(运行时不可变)
    const double FarPlane = 4000;
    double F = 1;    //f焦距(默认1)
    const double Forward[3] = { NearPlane, 0, 0 },
        y[3] = { NearPlane, -tan(FOV) * NearPlane, 0 },
        z[3] = { NearPlane, 0, tan(FOV) * NearPlane },
        move[3] = { 0, -1, 0 };  //移动辅助点(固定值)
    //以camera为参照的初始相机方向坐标（作为固定值不可更改！）

    //存储实时方向向量
    vertex Forward_vec = { Forward[0], Forward[1], Forward[2] },
               Y_vec = { y[0], y[1], y[2] },
               Z_vec = { z[0], z[1], z[2] },
               move_vec = { move[0], move[1], move[2] };
};

double SSE2Qsqrt(double number);
double KQsqrt(double number);
double GetLength(const vertex &vec);
double GetLength(const double vec[3]);//重载函数
double dot(const vertex &vec1, const vertex &vec2);
void cross(const vertex& vec1, const vertex& vec2, vertex &vec_cross);


class Transform {
private:
    inline void Get_CrossPoint(const Camera_data& Receive_camera, const long screen_in[2], const vertex& origin_1, const vertex& origin_2, vertex& output);
    inline void CameraSpace_to_ScreenSpace(const Camera_data& Receive_camera, const long screen_in[2], const vertex& vertex_origin, vertex& out);
public:
    inline void Get_NormalVector(Mmesh& cMesh);
    void Perspective(const Camera_data& Receive_camera, const long screen_in[2], Mmesh& TargetMesh, mesh_tf& out_mesh);


};




class Graphics {
private:
	inline void DrawFlatTopTriangle(const vertex& v0, const vertex& v1, const vertex& v2, const Color& c);
    inline void DrawFlatBottomTriangle(const vertex& v0, const vertex& v1, const vertex& v2, const Color& c);
public:
    static long Buffer_size[2];
    static std::vector<double> RedBuffer;
    static std::vector<double> GreenBuffer;
    static std::vector<double> BlueBuffer;

    static std::vector<double> DepthBuffer;

    inline double To_unLineDepth(Camera_data& Rec_camera, double& depth);

    void SetBuffer(long width, long height);
    void CleanBuffer();

    void PutPixel(long x, long y, double deepth, Color c);
    Color GetPixelColor(const long x, const long y);


	void DrawTriangle(const vertex& v1, const vertex& v2, const vertex& v3, Color& c);


};

