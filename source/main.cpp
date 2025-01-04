#include <iostream>
#include<graphics.h>  //EasyX图形库
#include <thread>
#include <windows.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include <emmintrin.h> // 包含SSE2指令集的头文件
#include <random>

#include "all.h"
#define PI 3.14159265358979

//moon-engine

//分辨率
unsigned int screen_1[] = { 960, 540 };
unsigned int screen_2[] = {1280, 720};
unsigned int* pScreen = screen_2;
//初始化

//计算用数据类型
struct Mpoint {
    double x = 0, y = 0, z = 0;
};
struct Mface {
    unsigned int sequnce[3] = { 0,0,0 };
};
struct Mmesh {
    std::vector <Mpoint> vertices = {};
    std::vector <Mface> faces = {};
    std::vector <Mpoint> normal_vectors = {};
    std::vector <Mpoint> textures = {};
};

//camera结构数据类型
struct Camera_data{
                    //HFOV水平视野           VFOV垂直视野（好像没什么实际作用）
    double Horizen = 44 * PI / 180, angle_verti = 30 * PI / 180;  
    double Camera[3] = { 0, 0, 0 };  //摄像机位置（实时）
    const double R = 0.01;   //近平面到camera点距离(运行时不可变)
    double F = 1;    //f焦距(默认1)
    const double front[3] = { R, 0, 0 },
                y[3] = { R, -tan(Horizen) * R, 0 },
                z[3] = { R, 0, tan(angle_verti) * R },
                move[2] = { 0, -1 };  //移动辅助点(标准值)
    //以camera为参照的初始相机方向坐标（作为标准值不可更改！）
    
    //存储实时方向向量
    double Forward_vec[3] = {front[0], front[1], front[2]},
                Y_vec[3] = {y[0], y[1], y[2]},
                Z_vec[3] = {z[0], z[1], z[2]},
                move_vec[2] = { move[0], move[1] };
};
//创建一个相机
Camera_data Camera_1;

//互斥锁
Camera_data sharedStruct;
std::mutex camera_Remain;

#if 0
//输出数组
template<typename T, size_t n>
void printA(T const(&arr)[n])
{
    std::cout.precision(16);
    for (size_t i = 0; i < n; i++) {
        std::cout << arr[i] << ' ';
    }
    std::cout << std::endl;
}
#endif

//使用SSE2指令集的快速平方根
inline double SSE2Qsqrt(double number) {
    __m128d vec;
    vec = _mm_load_sd(&number); // 加载double值到SSE寄存器
    vec = _mm_sqrt_pd(vec);   // 计算平方根
    _mm_store_sd(&number, vec); // 将结果存储回double变量
    return number;
}

//卡马克之魂快速平方根
inline double KQsqrt(double number) {
    long long i;
    double x2, y;
    const double threehalfs = 1.5;

    x2 = number * 0.5;
    y = number;
    i = *reinterpret_cast<long long*>(&y); // 将 double 的位模式解释为 long long
    i = 0x5fe6eb50c7b537aaLL - (i >> 1);   // what the fuck?
    y = *reinterpret_cast<double*>(&i);    // 将 long long 的位模式解释回 double
    y = y * (threehalfs - (x2 * y * y));   // 1st iteration
    y = y * (threehalfs - (x2 * y * y));   // 2nd iteration

    return 1 / y;
}

//选择哪种平方根函数
double (*Qsqrt)(double) = KQsqrt;


// 函数用于将鼠标移动到屏幕中央
void MoveMouseToCenter() {
    // 获取屏幕的宽度和高度
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 计算屏幕中央的坐标
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;
    // 将鼠标移动到屏幕中央
    SetCursorPos(centerX, centerY);
}


class Main_function {
public:


//计算线在近平面的交点坐标new                相机                            一个点                         另一个点                           输出(屏幕上坐标)
inline void Get_CrossPoint(const Camera_data& Rc,unsigned const int screen[2], const Mpoint& origin, const Mpoint& process_point, Mpoint& output) {
    //待处理点和camera的front点的向量
    double nL1[] = { Rc.Forward_vec[0] - process_point.x + Rc.Camera[0],
                             Rc.Forward_vec[1] - process_point.y + Rc.Camera[1],
                             Rc.Forward_vec[2] - process_point.z + Rc.Camera[2] };
    //待处理点和另一点的向量
    double nL2[] = { origin.x - process_point.x,
                             origin.y - process_point.y,
                             origin.z - process_point.z };

    //待处理点到近平面的距离
    double distance = (Rc.Forward_vec[0] * nL1[0] + Rc.Forward_vec[1] * nL1[1] + Rc.Forward_vec[2] * nL1[2]) / Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]);
    //nL2的模
    double Length = Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]);
    //nL2 向量和nF向量的cos值
    double cosV = (Rc.Forward_vec[0] * nL2[0] + Rc.Forward_vec[1] * nL2[1] + Rc.Forward_vec[2] * nL2[2]) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]));
    double n_plane[3];
    if (distance == 0) {
        //恰好距离零时处理
        n_plane[0] = process_point.x - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = process_point.y - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = process_point.z - Rc.Forward_vec[2] - Rc.Camera[2];
    }else {
        double Tmp[] = { (nL2[0] * distance) / (cosV * Length) + process_point.x,
                                   (nL2[1] * distance) / (cosV * Length) + process_point.y,
                                  (nL2[2] * distance) / (cosV * Length) + process_point.z };
        n_plane[0] = Tmp[0] - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = Tmp[1] - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = Tmp[2] - Rc.Forward_vec[2] - Rc.Camera[2];
    };
    output.x = (screen[0] / 2) + (Rc.F * (screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2])));
    output.y = (screen[1] / 2) - (Rc.F * (screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2])));
    output.z = 0;
}

// 计算面的法向量New                                 mesh
inline void Get_NormalVector(Mmesh &cMesh) {
    Mpoint normal_vectors;
    for (Mface &each_face : cMesh.faces) {
        double n_a[3] = {cMesh.vertices[each_face.sequnce[1]].x -cMesh.vertices[each_face.sequnce[0]].x ,
                                    cMesh.vertices[each_face.sequnce[1]].y - cMesh.vertices[each_face.sequnce[0]].y,
                                    cMesh.vertices[each_face.sequnce[1]].z - cMesh.vertices[each_face.sequnce[0]].z
         };
        double n_b[3] = { cMesh.vertices[each_face.sequnce[2]].x - cMesh.vertices[each_face.sequnce[1]].x,
                                    cMesh.vertices[each_face.sequnce[2]].y - cMesh.vertices[each_face.sequnce[1]].y,
                                    cMesh.vertices[each_face.sequnce[2]].z - cMesh.vertices[each_face.sequnce[1]].z
        };
        double n_V[3] = { (n_b[1] * n_a[2] - n_b[2] * n_a[1]), (n_b[2] * n_a[0] - n_b[0] * n_a[2]), (n_b[0] * n_a[1] - n_b[1] * n_a[0]) };
        double Length = Qsqrt(n_V[0] * n_V[0] + n_V[1] * n_V[1] + n_V[2] * n_V[2]);//法向量的模
        normal_vectors.x = n_V[0] / Length;
        normal_vectors.y = n_V[1] / Length;
        normal_vectors.z = n_V[2] / Length;
        cMesh.normal_vectors.push_back(normal_vectors);
    }
}

// New render       摄像机结构体                          mesh                    输出数组mesh
void Render(const Camera_data& Rc, unsigned const int screen[2], Mmesh& cMesh, Mmesh& out_mesh) {
    //点的可见性检查数组//面的正面性检查数组
    std::vector <bool> Pinfo, FTinfo;
    //转化到平面坐标轴上的顶点的临时数组
    std::vector <Mpoint> Transform_vertices;
    //计算法向量
    cMesh.normal_vectors.clear();
    Get_NormalVector(cMesh);
    for (const Mpoint &each_point : cMesh.vertices) {
        //临时值
        Mpoint TmpP, n_P, outP;
        n_P.x = each_point.x - Rc.Camera[0];
        n_P.y = each_point.y - Rc.Camera[1];
        n_P.z = each_point.z - Rc.Camera[2];

        double Length = Qsqrt(n_P.x * n_P.x + n_P.y * n_P.y + n_P.z * n_P.z), cosV;
        //与摄像机重合的情况
        if (Length == 0) {
            Pinfo.push_back(FALSE);
            Transform_vertices.push_back(TmpP);
            continue;
        } else {//cos 值
            cosV = (Rc.Forward_vec[0] * n_P.x + Rc.Forward_vec[1] * n_P.y + Rc.Forward_vec[2] * n_P.z) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * Length);
        }
        if (cosV > 0 && Length * cosV >= Rc.R) {   //当顶点在摄像机前方时cos为正时,正常情况
            //添加点可见的信息
            Pinfo.push_back(TRUE);
            //计算映射到近平面的坐标(以世界坐标轴)
            TmpP.x = (n_P.x * Rc.R) / (Length * cosV);
            TmpP.y = (n_P.y * Rc.R) / (Length * cosV);
            TmpP.z = (n_P.z * Rc.R) / (Length * cosV);
            //映射到近平面的坐标点和方向点的向量
            double n_plane[] = { TmpP.x - Rc.Forward_vec[0], TmpP.y - Rc.Forward_vec[1], TmpP.z - Rc.Forward_vec[2] };
            outP.x = (screen[0] / 2) + (Rc.F * (screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2])));
            outP.y = (screen[1] / 2) - (Rc.F * (screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2])));
            outP.z = Length * cosV - Rc.R;   //z值用来存储Deepth方便后面制作Dbuffer用
            Transform_vertices.push_back(outP);
            //平面坐标轴映射，                                                                           坐标已适配direcX坐标系
        } else {            //不可见点情况处理(位于近平面之后)
            Pinfo.push_back(FALSE);
            Transform_vertices.push_back(outP);//即使不可见也要占位
            continue;
        }
    }
    //以面为单位将每个面的坐标信息提取，并二次分割处理有不可见点的平面（一个点不可见的情况；两个点不可见的情况*(unfinish)；三个点都不可见的情况）
    for (int i = 0; i < cMesh.faces.size(); ++i) {
        //测试正面用向量
        double nTest[3] = { cMesh.vertices[ cMesh.faces[i].sequnce[0] ].x - Rc.Camera[0], cMesh.vertices[ cMesh.faces[i].sequnce[0]].y - Rc.Camera[1], cMesh.vertices[ cMesh.faces[i].sequnce[0] ].z - Rc.Camera[2]};
        //储存面的正面性信息(后面剔除背面)
        (nTest[0] * cMesh.normal_vectors[i].x + nTest[1] * cMesh.normal_vectors[i].y + nTest[2] * cMesh.normal_vectors[i].z) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
        //面全可见的情况（主要)
        if (Pinfo[ cMesh.faces[i].sequnce[0] ] && Pinfo[ cMesh.faces[i].sequnce[1] ] && Pinfo[ cMesh.faces[i].sequnce[2] ] && FTinfo[i]) {
            out_mesh.vertices.push_back(Transform_vertices[cMesh.faces[i].sequnce[0]]);
            out_mesh.vertices.push_back(Transform_vertices[cMesh.faces[i].sequnce[1]]);
            out_mesh.vertices.push_back(Transform_vertices[cMesh.faces[i].sequnce[2]]);
        }
        else if (FTinfo[i]) {
            //不完整面处理(仍然检查总体面正面可见性)
            int num = static_cast<int>(not Pinfo[ cMesh.faces[i].sequnce[0] ]) + static_cast<int>(not Pinfo[ cMesh.faces[i].sequnce[1] ]) + static_cast<int>(not Pinfo[ cMesh.faces[i].sequnce[2] ]);
            if (num == 3) continue; //排除完全不可见
            bool num_bool = (num == 1) ? TRUE : FALSE;
            //          前顶点        后顶点      中间点   (看情况各自分配是不可见点还是可见点)
            unsigned int previous{}, next{}, medium{};
            //          切点1                  切点2
            Mpoint ClipOut_1, ClipOut_2;
            //for循环分辨情况//这样做是为了最后生成的点的数组仍然是正面顺时针的顺序
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[ cMesh.faces[i].sequnce[e] ]) {//获得待处理点            //此处的异或运算可以分辨是1情况还是2情况
                    medium = cMesh.faces[i].sequnce[ e ];
                    previous = cMesh.faces[i].sequnce[(e + 5) % 3];
                    next = cMesh.faces[i].sequnce[(e + 7) % 3];
                    Get_CrossPoint(Rc, screen, cMesh.vertices[previous], cMesh.vertices[medium], ClipOut_1);
                    Get_CrossPoint(Rc, screen, cMesh.vertices[next], cMesh.vertices[medium], ClipOut_2);
                    break;
                }
            }
            if (num_bool) {
                out_mesh.vertices.push_back(Transform_vertices[ previous ]);
                out_mesh.vertices.push_back(ClipOut_1);
                out_mesh.vertices.push_back(ClipOut_2);
                //此处一个不可见点输出两个三角面
                out_mesh.vertices.push_back(ClipOut_2);
                out_mesh.vertices.push_back(Transform_vertices[next]);
                out_mesh.vertices.push_back(Transform_vertices[previous]);
                //后续光栅化加入纹理映射时，记得在这加纹理坐标转换！
            }else{
                out_mesh.vertices.push_back(ClipOut_1);
                out_mesh.vertices.push_back(Transform_vertices[medium]);
                out_mesh.vertices.push_back(ClipOut_2);
            }
        };

    }
    //developing now

}

//相机设置                     目标相机                    倾斜角度                    水平角度                      纵向角度             前方向移动量//横方向变化量//纵方向变化量 
void camera_set( Camera_data &Tc, double incline_angle, double revolve_hori, double revolve_verti, double moveF,double moveH,double moveZ){
    double front_tmp[] = {Tc.front[0], Tc.front[1], Tc.front[2]};
    double y_tmp[] = { Tc.y[0], Tc.y[1], Tc.y[2] };
    double z_tmp[] = { Tc.z[0], Tc.z[1], Tc.z[2] };

    double ft[3]{0,0,0}, yt[3]{0,0,0}, zt[3]{0,0,0};
    //中间临时缓冲值
    
    //合并过来的移动代码
    if (moveF != 0) {
        double n_front[] = { -Tc.Forward_vec[0],  -Tc.Forward_vec[1], -Tc.Forward_vec[2] };
        double C_front = 1 - (moveF / Tc.R);

        Tc.Camera[0] = Tc.Camera[0] - Tc.Forward_vec[0] * C_front + Tc.Forward_vec[0];
        Tc.Camera[1] = Tc.Camera[1] -Tc.Forward_vec[1] * C_front + Tc.Forward_vec[1];
        Tc.Camera[2] = Tc.Camera[2] -Tc.Forward_vec[2] * C_front + Tc.Forward_vec[2];
    }//前向

    if (moveH != 0) {
        double n_horizon[] = { Tc.Camera[0] - Tc.move_vec[0], Tc.Camera[1] - Tc.move_vec[1] };
        double Lhorizon = Qsqrt((n_horizon[0])*(n_horizon[0]) + (n_horizon[1]) * (n_horizon[1]));
        double C_horizon = 1 - (moveH / Lhorizon);

        Tc.Camera[0] = n_horizon[0] * C_horizon + Tc.move_vec[0];
        Tc.Camera[1] = n_horizon[1] * C_horizon + Tc.move_vec[1];
        Tc.move_vec[0] = Tc.move_vec[0] + Tc.Camera[0];
        Tc.move_vec[1] = Tc.move_vec[1] + Tc.Camera[1];
    }//横向

    if (moveZ != 0) {
        Tc.Camera[2] += moveZ;
    }//纵向

    //相机倾斜
    if (incline_angle != 0){
        double cosV = cos(-incline_angle);
        double sinV = sin(-incline_angle);

        z_tmp[1] = Tc.z[2]*sinV + Tc.z[1]*cosV;
        z_tmp[2] = Tc.z[2]*cosV - Tc.z[1]*sinV;

        y_tmp[1] = Tc.y[2] * sinV + Tc.y[1] * cosV;
        y_tmp[2] = Tc.y[2] * cosV - Tc.y[1] * sinV;
    }

    //相机纵向旋转
    if (revolve_verti != 0){
        double cosV = cos(revolve_verti);
        double sinV = sin(revolve_verti);

        front_tmp[0] = Tc.front[0] * cosV - Tc.front[2] * sinV;
        front_tmp[2] = Tc.front[2] * cosV + Tc.front[0] * sinV;

        yt[2] = y_tmp[2] * cosV + y_tmp[0] * sinV;
        yt[0] = y_tmp[0] * cosV - y_tmp[2] * sinV;
        y_tmp[2] = yt[2];
        y_tmp[0] = yt[0];

        zt[2] = z_tmp[2] * cosV + z_tmp[0] * sinV;
        zt[0] = z_tmp[0] * cosV - z_tmp[2] * sinV;
        z_tmp[2] = zt[2];
        z_tmp[0] = zt[0];

    }

    //相机水平旋转
    if (revolve_hori != 0){
        double cosV = cos(-revolve_hori);
        double sinV = sin(-revolve_hori);

        ft[1] = front_tmp[1] * cosV + front_tmp[0] * sinV;
        ft[0] = front_tmp[0] * cosV - front_tmp[1] * sinV;
        front_tmp[1] = ft[1];
        front_tmp[0] = ft[0];


        yt[1] = y_tmp[1] * cosV + y_tmp[0] * sinV;
        yt[0] = y_tmp[0] * cosV - y_tmp[1] * sinV;
        y_tmp[1] = yt[1];
        y_tmp[0] = yt[0];

        zt[1] = z_tmp[1] * cosV + z_tmp[0] * sinV;
        zt[0] = z_tmp[0] * cosV - z_tmp[1] * sinV;
        z_tmp[1] = zt[1];
        z_tmp[0] = zt[0];
     

        //移动用的辅助点旋转
        Tc.move_vec[1] = Tc.move[1] * cosV + Tc.move[0] * sinV;
        Tc.move_vec[0] = Tc.move[0] * cosV - Tc.move[1] * sinV;
        Tc.move_vec[0] = Tc.move_vec[0] + Tc.Camera[0];
        Tc.move_vec[1] = Tc.move_vec[1] + Tc.Camera[1];
    }else if (revolve_hori == 0) {
        Tc.move_vec[0] = Tc.move[0] + Tc.Camera[0];
        Tc.move_vec[1] = Tc.move[1] + Tc.Camera[1];
        //防止移动辅助方向点不更新
    }


    Tc.Forward_vec[0] = front_tmp[0];
    Tc.Forward_vec[1] = front_tmp[1];
    Tc.Forward_vec[2] = front_tmp[2];

    Tc.Y_vec[0] = y_tmp[0] - Tc.Forward_vec[0];
    Tc.Y_vec[1] = y_tmp[1] - Tc.Forward_vec[1];
    Tc.Y_vec[2] = y_tmp[2] - Tc.Forward_vec[2];

    Tc.Z_vec[0] = z_tmp[0] - Tc.Forward_vec[0];
    Tc.Z_vec[1] = z_tmp[1] - Tc.Forward_vec[1];
    Tc.Z_vec[2] = z_tmp[2] - Tc.Forward_vec[2];
    return;
    
    
}



};
Main_function Mainfunc;




struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3,1,0},{4,1,0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };
};

cube cubeData;


void draw_thread_New() {
    initgraph(pScreen[0], pScreen[1]);
    setbkcolor(BLACK); // 背景色
    settextcolor(GREEN);//文字颜色
    setlinecolor(RGB(255, 255, 255)); // 线框颜色
    setfillcolor(RGB(128, 128, 128));

    std::vector <Mmesh> mesh_list;
    
        // 创建随机数生成器
    std::random_device rd;  // 非确定性随机数生成器，用于初始化
    std::mt19937 gen(rd()); // Mersenne Twister 19937 生成器

    // 创建一个均匀分布的随机数生成器
    std::uniform_int_distribution<> dis(-10, 10);
    //cube test压力测试
    for (int t = -20; t < 20; t += 2) {
        for (int w = -20; w < 20;w +=2) {
            Mmesh cube_mesh;
            for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
                Mpoint& each_point = cube_mesh.vertices.emplace_back();
                const int random_number = dis(gen);
                each_point.x = cubeData.Cpoint[i][0] + t;
                each_point.y = cubeData.Cpoint[i][1] + w;
                each_point.z = cubeData.Cpoint[i][2]  ;
            }
            for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
                Mface& each_face = cube_mesh.faces.emplace_back();
                each_face.sequnce[0] = cubeData.Ctriangle[i][0];
                each_face.sequnce[1] = cubeData.Ctriangle[i][1];
                each_face.sequnce[2] = cubeData.Ctriangle[i][2];
            }

            mesh_list.push_back(cube_mesh);
        }
    }
    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }
        unsigned int faces_num = 0;

        double Frames;
        auto start = std::chrono::high_resolution_clock::now();   //测帧
        BeginBatchDraw();
        cleardevice(); // 清空屏幕
        for (Mmesh &each_mesh : mesh_list) {
            

            Mmesh out;
            camera_Remain.lock();
            Mainfunc.Render(Camera_1,pScreen ,each_mesh, out);
            camera_Remain.unlock();

            //此时的out_mesh中vertices按顺序以三个点为一个面，不使用成员faces
            for (unsigned int k = 0; k < out.vertices.size(); k +=3) {
                POINT p[] = { static_cast<long>(out.vertices[k].x) , static_cast<long>(out.vertices[k].y) ,static_cast<long>(out.vertices[k+1].x) , static_cast<long>(out.vertices[k+1].y) , static_cast<long>(out.vertices[k+2].x) , static_cast<long>(out.vertices[k+2].y) };
                fillpolygon(p, 3);
                ++faces_num;
            }
            
        }


        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start;
        Frames = 1 / elapsed_seconds.count();


        std::string title_str = "MOON_Engine_Build_version_0.4.1  Compilation_Date:";
        title_str.append(__DATE__);
        std::string info = "FPS: " + std::to_string(Frames);
        std::string Faces_number = "  Faces: " + std::to_string(faces_num);
        info.append(Faces_number);

        const char* title = title_str.c_str();
        const char* Frames_char = info.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) + textheight(Frames_char) + 4, "By_H  press ESC to close this program!");

        EndBatchDraw();
    }

}


//camera控制线程
void control_thread()
{
    POINT currentPos, centerPos = { GetSystemMetrics(SM_CXSCREEN)/ 2, GetSystemMetrics(SM_CYSCREEN)/2 };
    BOOL firstTime = TRUE;

    double angleCurrent[] = { 0,0 };
    double CS[] = {0,0,0,0,0,0};
    double speed = 0.03;

    Camera_1.Camera[2] = 1;//更改camera初始位置示范
    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            break;
        }

        Sleep(1); // 休眠一段时间
        GetCursorPos(&currentPos);
        
        if (firstTime) {
            firstTime = FALSE;
        }else{
            int deltaX = currentPos.x - centerPos.x;
            int deltaY = currentPos.y - centerPos.y;

            angleCurrent[0] = angleCurrent[0] + deltaX * 0.4;
            angleCurrent[1] = angleCurrent[1] - deltaY * 0.4;
            //标准限制化处理数据

            if (angleCurrent[0] > 360) {
                angleCurrent[0] = angleCurrent[0] - 360;
            }else if (angleCurrent[0] < -360) {
                angleCurrent[0] = angleCurrent[0] + 360;
            }
            if (angleCurrent[1] > 90) {
                angleCurrent[1] = 90;
            }else if(angleCurrent[1] < -90) {
                angleCurrent[1] = -90;
            }

            //处理camera镜头方向
            CS[1] = angleCurrent[0] * PI / 180;
            CS[2] = angleCurrent[1] * PI / 180;
        }
        //把鼠标移到中心
        MoveMouseToCenter();

        //检测操作

        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
            speed =0.1;
        }else{
            speed = 0.03;
        }

        if (GetAsyncKeyState('W') & 0x8000){
            CS[3] = speed;
        }
        else if (GetAsyncKeyState('S') & 0x8000) {
            CS[3] = -speed;
        }else{
            CS[3] = 0;
        }
        if (GetAsyncKeyState('A') & 0x8000){
            CS[4] = -speed;
            //CS[0] = -1 * PI / 180;
        }else if (GetAsyncKeyState('D') & 0x8000){
            CS[4] = speed;
            //CS[0] = 1 * PI / 180;
        }else{
            CS[4] = 0;
            //CS[0] = 0;
        }
        if (GetKeyState(VK_LSHIFT) & 0x8000){
            CS[5] = -speed;
        }else if (GetAsyncKeyState(VK_SPACE) & 0x8000){
            CS[5] = speed;
        }else{
            CS[5] = 0;
        }

        camera_Remain.lock();
        //变焦试验
        if (GetKeyState('Z') & 0x8000 && Camera_1.F < 10) {
            Camera_1.F = Camera_1.F+0.1;
        }
        if (GetAsyncKeyState('C') & 0x8000 && Camera_1.F > 0.7) {
            Camera_1.F = Camera_1.F - 0.1;
        }
        if (GetAsyncKeyState('X') & 0x8000) {
            Camera_1.F = 1;
        }
        Mainfunc.camera_set(Camera_1,CS[0], CS[1], CS[2], CS[3], CS[4], CS[5]);
        camera_Remain.unlock();

        

    }
}


int main() {

    Rasterization_function test;
    test.Rasterizer();

    std::thread draw(draw_thread_New);
    std::thread control(control_thread);

    draw.join();
    control.join();
    return 0;
}
