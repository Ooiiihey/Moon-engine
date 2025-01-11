#include "moon.h"

#include <iostream>
#include <vector>
#include <algorithm>
#include<graphics.h>  //EasyX图形库
#include <thread>
#include <windows.h>
#include <string>
#include <shared_mutex>
#define PI 3.14159265358979
vectorMath vecMath;
//moon-engine

//分辨率
unsigned int screen_1[] = { 960, 540 };
unsigned int screen_2[] = {1280, 720};
unsigned int* pScreen = screen_2;
//初始化


//camera结构数据类型
struct Camera_data{
                    //HFOV水平视野  
    double FOV = 40 * PI / 180;
    double Camera[3] = { 0, 0, 0 };  //摄像机位置（实时）
    const double nearPlane = 0.01;   //近平面距离(运行时不可变)
    double F = 1;    //f焦距(默认1)
    const double Forward[3] = { nearPlane, 0, 0 },
                y[3] = { nearPlane, -tan(FOV) * nearPlane, 0 },
                z[3] = { nearPlane, 0, tan(FOV) * nearPlane },
                move[3] = { 0, -1, 0 };  //移动辅助点(固定值)
    //以camera为参照的初始相机方向坐标（作为固定值不可更改！）
    
    //存储实时方向向量
    Point Forward_vec = {Forward[0], Forward[1], Forward[2]},
                Y_vec = {y[0], y[1], y[2]},
                Z_vec = {z[0], z[1], z[2]},
                move_vec = { move[0], move[1], move[2]};
};
//创建一个相机
Camera_data Camera_1;

//互斥锁
Camera_data sharedStruct;
std::mutex CameraData_Remain;

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

class PST_function {

private:
//计算线在近平面的交点坐标new                相机                                                     一个点                         另一个点                           输出(屏幕上坐标)
inline void Get_CrossPoint(const Camera_data& Receive_camera,unsigned const int screen_in[2], const Point& origin_1, const Point& origin_2, Point& output) {
    //待处理点和camera的front点的向量
    Point vec1 = { Receive_camera.Forward_vec.x - origin_2.x + Receive_camera.Camera[0],
                             Receive_camera.Forward_vec.y - origin_2.y + Receive_camera.Camera[1],
                             Receive_camera.Forward_vec.z - origin_2.z + Receive_camera.Camera[2] };
    //待处理点和另一点的向量
    Point vec2 = { origin_1.x - origin_2.x,
                             origin_1.y - origin_2.y,
                             origin_1.z - origin_2.z };

    //待处理点到近平面的距离
    double distance = vecMath.dot(Receive_camera.Forward_vec, vec1) / vecMath.GetLength(Receive_camera.Forward_vec);
    //nL2的模
    double vec2_Len = vecMath.GetLength(vec2);
    //vec2 向量和nF向量的cos值
    double cosV = vecMath.dot(vec2, Receive_camera.Forward_vec) / (vecMath.GetLength(Receive_camera.Forward_vec) * vec2_Len);
    Point n_plane;
    if (distance == 0) {
        //恰好距离零时处理
        n_plane.x = origin_2.x - Receive_camera.Forward_vec.x - Receive_camera.Camera[0];
        n_plane.y = origin_2.y - Receive_camera.Forward_vec.y - Receive_camera.Camera[1];
        n_plane.z = origin_2.z - Receive_camera.Forward_vec.z - Receive_camera.Camera[2];
    }else {
        /*
               double Tmp[] = { (vec2.x * distance) / (cosV * vec2_Len) + origin_2.x,
                                           (vec2.y * distance) / (cosV * vec2_Len) + origin_2.y,
                                           (vec2.z * distance) / (cosV * vec2_Len) + origin_2.z };
        */
        n_plane.x = (vec2.x * distance) / (cosV * vec2_Len) + origin_2.x - Receive_camera.Forward_vec.x - Receive_camera.Camera[0];
        n_plane.y = (vec2.y * distance) / (cosV * vec2_Len) + origin_2.y - Receive_camera.Forward_vec.y - Receive_camera.Camera[1];
        n_plane.z = (vec2.z * distance) / (cosV * vec2_Len) + origin_2.z - Receive_camera.Forward_vec.z - Receive_camera.Camera[2];
    };
    output.x = (screen_in[0] / 2) + (Receive_camera.F * (screen_in[0] / 2) * vecMath.dot(Receive_camera.Y_vec, n_plane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Y_vec) ));
    output.y = (screen_in[1] / 2) - (Receive_camera.F * (screen_in[0] / 2) * vecMath.dot(Receive_camera.Z_vec, n_plane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Z_vec) ));
    output.z = 0;
}

public:
// 计算面的法向量New                               mesh
inline void Get_NormalVector(Mmesh &cMesh) {
    Point normal_vectors;
    for (Face &each_face : cMesh.faces) {
        
        double n_a[3] = {cMesh.vertices[each_face.index[1]].x -cMesh.vertices[each_face.index[0]].x ,
                                    cMesh.vertices[each_face.index[1]].y - cMesh.vertices[each_face.index[0]].y,
                                    cMesh.vertices[each_face.index[1]].z - cMesh.vertices[each_face.index[0]].z
         };
        double n_b[3] = { cMesh.vertices[each_face.index[2]].x - cMesh.vertices[each_face.index[1]].x,
                                    cMesh.vertices[each_face.index[2]].y - cMesh.vertices[each_face.index[1]].y,
                                    cMesh.vertices[each_face.index[2]].z - cMesh.vertices[each_face.index[1]].z
        };
        double n_V[3] = {(n_b[1] * n_a[2] - n_b[2] * n_a[1]), (n_b[2] * n_a[0] - n_b[0] * n_a[2]), (n_b[0] * n_a[1] - n_b[1] * n_a[0])};
        double Length = vecMath.GetLength(n_V);//法向量的模
        normal_vectors.x = n_V[0] / Length;
        normal_vectors.y = n_V[1] / Length;
        normal_vectors.z = n_V[2] / Length;
        cMesh.normal_vectors.push_back(normal_vectors);
    }
}

// New                              摄像机结构体                                                                          mesh                     输出数组mesh
void Perspective(const Camera_data& Receive_camera, unsigned const int screen_in[2], Mmesh& TargetMesh, Mmesh& out_mesh) {
    //点的可见性检查数组//面的正面性检查数组
    std::vector <bool> Pinfo, FTinfo;
    //转化到平面坐标轴上的顶点的临时数组
    std::vector <Point> Transformed_vertices;

    //计算法向量
    TargetMesh.normal_vectors.clear();
    TargetMesh.normal_vectors.reserve(TargetMesh.faces.size());
    Get_NormalVector(TargetMesh);

    for (const Point &each_point : TargetMesh.vertices) {
        //临时值
        Point  vec_P, Transformed_P;
        vec_P.x = each_point.x - Receive_camera.Camera[0];
        vec_P.y = each_point.y - Receive_camera.Camera[1];
        vec_P.z = each_point.z - Receive_camera.Camera[2];

        double cosV, Length = vecMath.GetLength(vec_P);
        //double Length = vecMath.GetLength(vec_P);
        //与摄像机重合的情况
        if (Length == 0) {
            Pinfo.push_back(FALSE);
            Transformed_vertices.emplace_back();
            continue;
        } else {//cos 值
            cosV = vecMath.dot(Receive_camera.Forward_vec, vec_P) / (vecMath.GetLength(Receive_camera.Forward_vec) * Length);
        }
        if (cosV > 0 && Length * cosV >= Receive_camera.nearPlane) {   //当顶点在摄像机前方时cos为正时,正常情况
            //添加点可见的信息
            Pinfo.push_back(TRUE);
            /*
            //计算映射到近平面的坐标(以世界坐标轴)
            TmpP.x = (vec_P.x * Receive_camera.nearPlane) / (Length * cosV);
            TmpP.y = (vec_P.y * Receive_camera.nearPlane) / (Length * cosV);
            TmpP.z = (vec_P.z * Receive_camera.nearPlane) / (Length * cosV);
            */
            //映射到近平面的坐标点和方向点的向量
            Point n_plane = { (vec_P.x * Receive_camera.nearPlane) / (Length * cosV) - Receive_camera.Forward_vec.x,
                                              (vec_P.y * Receive_camera.nearPlane) / (Length * cosV) - Receive_camera.Forward_vec.y, 
                                              (vec_P.z * Receive_camera.nearPlane) / (Length * cosV) - Receive_camera.Forward_vec.z };
            Transformed_P.x = (screen_in[0] / 2) + (Receive_camera.F * (screen_in[0] / 2) * vecMath.dot(Receive_camera.Y_vec, n_plane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Y_vec) ));
            Transformed_P.y = (screen_in[1] / 2) - (Receive_camera.F * (screen_in[0] / 2) * vecMath.dot(Receive_camera.Z_vec, n_plane) / (tan(Receive_camera.FOV) * Receive_camera.nearPlane * vecMath.GetLength(Receive_camera.Z_vec) ));
            Transformed_P.z = Length * cosV - Receive_camera.nearPlane;   //z值用来存储Deepth方便后面制作Dbuffer用
            Transformed_vertices.emplace_back(Transformed_P);
            //平面坐标轴映射，                                                                           坐标已适配direcX坐标系
        } else {            //不可见点情况处理(位于近平面之后)
            Pinfo.push_back(FALSE);
            Transformed_vertices.emplace_back(Transformed_P);//即使不可见也要占位
            continue;
        }
    }
    //直接跳过背面不可见的
    if (not std::any_of(Pinfo.begin(), Pinfo.end(), [](bool value) { return value; }) ) return;

    //以面为单位将每个角顶索引，并二次分割处理有不可见点的平面（一个点不可见的情况；两个点不可见的情况；三个点都不可见的情况）
    for (int i = 0; i < TargetMesh.faces.size(); ++i) {
        //测试正面用向量
        double vecTest[3] = { TargetMesh.vertices[ TargetMesh.faces[i].index[0] ].x - Receive_camera.Camera[0], TargetMesh.vertices[ TargetMesh.faces[i].index[0]].y - Receive_camera.Camera[1], TargetMesh.vertices[ TargetMesh.faces[i].index[0] ].z - Receive_camera.Camera[2]};
        //储存面的正面性信息(后面剔除背面)
        (vecTest[0] * TargetMesh.normal_vectors[i].x + vecTest[1] * TargetMesh.normal_vectors[i].y + vecTest[2] * TargetMesh.normal_vectors[i].z) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
        //面全可见的情况（主要)
        if (Pinfo[ TargetMesh.faces[i].index[0] ] && Pinfo[ TargetMesh.faces[i].index[1] ] && Pinfo[ TargetMesh.faces[i].index[2] ] && FTinfo[i]) {
            out_mesh.vertices.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[0]]);
            out_mesh.vertices.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[1]]);
            out_mesh.vertices.emplace_back(Transformed_vertices[TargetMesh.faces[i].index[2]]);
        } else if (FTinfo[i]) {
            //不完整面处理(仍然检查总体面正面可见性)
            int num = static_cast<int>(not Pinfo[ TargetMesh.faces[i].index[0] ]) + static_cast<int>(not Pinfo[ TargetMesh.faces[i].index[1] ]) + static_cast<int>(not Pinfo[ TargetMesh.faces[i].index[2] ]);
            if (num == 3) continue; //排除完全不可见
            bool num_bool = (num == 1) ? TRUE : FALSE;
            //          前顶点        后顶点      中间点   (看情况各自分配是不可见点还是可见点)
            unsigned int previous{}, next{}, medium{};
            //          切点1                  切点2
            Point ClipOut_1, ClipOut_2;
            //for循环分辨情况//这样做是为了最后生成的点的数组仍然是正面顺时针的顺序
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[ TargetMesh.faces[i].index[e] ]) {//获得待处理点            //此处的异或运算可以分辨是1情况还是2情况
                    medium = TargetMesh.faces[i].index[ e ];
                    previous = TargetMesh.faces[i].index[(e + 5) % 3];
                    next = TargetMesh.faces[i].index[(e + 7) % 3];
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[previous], TargetMesh.vertices[medium], ClipOut_1);
                    Get_CrossPoint(Receive_camera, screen_in, TargetMesh.vertices[next], TargetMesh.vertices[medium], ClipOut_2);
                    break;
                }
            }
            if (num_bool) {
                out_mesh.vertices.emplace_back(Transformed_vertices[ previous ]);
                out_mesh.vertices.emplace_back(ClipOut_1);
                out_mesh.vertices.emplace_back(ClipOut_2);
                //此处一个不可见点输出两个三角面
                out_mesh.vertices.emplace_back(ClipOut_2);
                out_mesh.vertices.emplace_back(Transformed_vertices[next]);
                out_mesh.vertices.emplace_back(Transformed_vertices[previous]);
                //后续光栅化加入纹理映射时，记得在这加纹理坐标转换！
            }else{
                out_mesh.vertices.emplace_back(ClipOut_1);
                out_mesh.vertices.emplace_back(Transformed_vertices[medium]);
                out_mesh.vertices.emplace_back(ClipOut_2);
            }
        };

    }
    //developing now

}

};
PST_function PSTfunc;

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

//相机设置                     目标相机                    倾斜角度                    水平角度                      纵向角度             前方向移动量//横方向变化量//纵方向变化量 
void camera_set( Camera_data &Target_camera, double incline_angle, double revolve_hori, double revolve_verti, double moveF,double moveH,double moveZ){
    double front_tmp[] = {Target_camera.Forward[0], Target_camera.Forward[1], Target_camera.Forward[2]};
    double y_tmp[] = { Target_camera.y[0], Target_camera.y[1], Target_camera.y[2] };
    double z_tmp[] = { Target_camera.z[0], Target_camera.z[1], Target_camera.z[2] };

    double ft[3]{0,0,0}, yt[3]{0,0,0}, zt[3]{0,0,0};
    //中间临时缓冲值
    
    //合并过来的移动代码
    if (moveF != 0) {
        double C_front = 1 - (moveF / Target_camera.nearPlane);

        Target_camera.Camera[0] = Target_camera.Camera[0] - Target_camera.Forward_vec.x * C_front + Target_camera.Forward_vec.x;
        Target_camera.Camera[1] = Target_camera.Camera[1] -Target_camera.Forward_vec.y * C_front + Target_camera.Forward_vec.y;
        Target_camera.Camera[2] = Target_camera.Camera[2] -Target_camera.Forward_vec.z * C_front + Target_camera.Forward_vec.z;
    }//前向

    if (moveH != 0) {
        Point n_horizon = { Target_camera.Camera[0] - Target_camera.move_vec.x, Target_camera.Camera[1] - Target_camera.move_vec.y };
        double Lhorizon = vecMath.GetLength(n_horizon);
        double C_horizon = 1 - (moveH / Lhorizon);

        Target_camera.Camera[0] = n_horizon.x * C_horizon + Target_camera.move_vec.x;
        Target_camera.Camera[1] = n_horizon.y * C_horizon + Target_camera.move_vec.y;
        Target_camera.move_vec.x = Target_camera.move_vec.x + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move_vec.y + Target_camera.Camera[1];
    }//横向

    if (moveZ != 0) {
        Target_camera.Camera[2] += moveZ;
    }//纵向

    //相机倾斜
    if (incline_angle != 0){
        double cosV = cos(-incline_angle);
        double sinV = sin(-incline_angle);

        z_tmp[1] = Target_camera.z[2]*sinV + Target_camera.z[1]*cosV;
        z_tmp[2] = Target_camera.z[2]*cosV - Target_camera.z[1]*sinV;

        y_tmp[1] = Target_camera.y[2] * sinV + Target_camera.y[1] * cosV;
        y_tmp[2] = Target_camera.y[2] * cosV - Target_camera.y[1] * sinV;
    }

    //相机纵向旋转
    if (revolve_verti != 0){
        double cosV = cos(revolve_verti);
        double sinV = sin(revolve_verti);

        front_tmp[0] = Target_camera.Forward[0] * cosV - Target_camera.Forward[2] * sinV;
        front_tmp[2] = Target_camera.Forward[2] * cosV + Target_camera.Forward[0] * sinV;

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
        Target_camera.move_vec.y = Target_camera.move[1] * cosV + Target_camera.move[0] * sinV;
        Target_camera.move_vec.x = Target_camera.move[0] * cosV - Target_camera.move[1] * sinV;
        Target_camera.move_vec.x = Target_camera.move_vec.x + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move_vec.y + Target_camera.Camera[1];
    }else if (revolve_hori == 0) {
        Target_camera.move_vec.x = Target_camera.move[0] + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move[1] + Target_camera.Camera[1];
        //防止移动辅助方向点不更新
    }


    Target_camera.Forward_vec.x = front_tmp[0];
    Target_camera.Forward_vec.y = front_tmp[1];
    Target_camera.Forward_vec.z = front_tmp[2];

    Target_camera.Y_vec.x = y_tmp[0] - Target_camera.Forward_vec.x;
    Target_camera.Y_vec.y = y_tmp[1] - Target_camera.Forward_vec.y;
    Target_camera.Y_vec.z = y_tmp[2] - Target_camera.Forward_vec.z;

    Target_camera.Z_vec.x = z_tmp[0] - Target_camera.Forward_vec.x;
    Target_camera.Z_vec.y = z_tmp[1] - Target_camera.Forward_vec.y;
    Target_camera.Z_vec.z = z_tmp[2] - Target_camera.Forward_vec.z;
    return;
    
    
}



struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3,1,0},{4,1,0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };
};
cube cubeData;



void Render_thread() {
    initgraph(pScreen[0], pScreen[1]);
    setbkcolor(BLACK); // 背景色
    settextcolor(GREEN);//文字颜色
    setlinecolor(RGB(255, 255, 255)); // 线框颜色
    setfillcolor(RGB(128, 128, 128));

    std::vector <Mmesh> mesh_list;
    //cube test压力测试
    for (int h = -2; h < 0;h +=2) {
    for (int t = -20; t < 20; t += 2) {
        for (int w = -20; w < 20;w +=2) {
            Mmesh cube_mesh;
            for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
                Point& each_point = cube_mesh.vertices.emplace_back();
                each_point.x = cubeData.Cpoint[i][0] + t;
                each_point.y = cubeData.Cpoint[i][1] + w;
                each_point.z = cubeData.Cpoint[i][2] + h ;
            }
            for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
                Face& each_face = cube_mesh.faces.emplace_back();
                each_face.index[0] = cubeData.Ctriangle[i][0];
                each_face.index[1] = cubeData.Ctriangle[i][1];
                each_face.index[2] = cubeData.Ctriangle[i][2];
            }

            mesh_list.emplace_back(cube_mesh);
        }
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

        auto PSTcosttime_0 = std::chrono::high_resolution_clock::now();
        Mmesh out;
        CameraData_Remain.lock();
        for (Mmesh& each_mesh : mesh_list) {
            PSTfunc.Perspective(Camera_1, pScreen, each_mesh, out);
            //此时的out_mesh中vertices按顺序以三个点为一个面，不使用成员faces
        }
        CameraData_Remain.unlock();
        auto PSTcosttime_1 = std::chrono::high_resolution_clock::now();

        //临时实现光栅化
        for (unsigned int k = 0; k < out.vertices.size(); k += 3) {
            POINT triangle[] = { static_cast<long>(out.vertices[k].x) , static_cast<long>(out.vertices[k].y) ,static_cast<long>(out.vertices[k + 1].x) , static_cast<long>(out.vertices[k + 1].y) , static_cast<long>(out.vertices[k + 2].x) , static_cast<long>(out.vertices[k + 2].y) };
            fillpolygon(triangle, 3);
        }
        
        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start;
        Frames = 1/elapsed_seconds.count();
        faces_num = out.vertices.size() / 3;

        std::chrono::duration<double> PSTcosttime = PSTcosttime_1 - PSTcosttime_0;
        double PSTcosttime_process = 1/PSTcosttime.count();


        std::string title_str = "MOON_Engine_Build_version_0.4.2  Compilation_Date:";
        title_str.append(__DATE__);
        std::string info = "FPS: " + std::to_string(Frames);
        std::string info2 = ("  PST_only_FPS:" + std::to_string(PSTcosttime_process));
        std::string Faces_number = "  Faces: " + std::to_string(faces_num);
        info.append(info2);
        info.append(Faces_number);

        const char* title = title_str.c_str();
        const char* Frames_char = info.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) + textheight(Frames_char) + 4, "By_H  press ESC to close this program!");
        fillcircle(pScreen[0] / 2, pScreen[1] / 2, 2);

        EndBatchDraw();
    }

}


//camera控制线程
void control_thread()
{
    POINT currentPos, centerPos = { GetSystemMetrics(SM_CXSCREEN)/ 2, GetSystemMetrics(SM_CYSCREEN)/2 };
    BOOL firstTime = TRUE;

    double angleCurrent[] = { 0,0 };
    double Control[] = {0,0,0,0,0,0};
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
            }if (angleCurrent[0] < -360) {
                angleCurrent[0] = angleCurrent[0] + 360;
            }
            if (angleCurrent[1] > 90) {
                angleCurrent[1] = 90;
            }if(angleCurrent[1] < -90) {
                angleCurrent[1] = -90;
            }

            //处理camera镜头方向
            Control[1] = angleCurrent[0] * PI / 180;
            Control[2] = angleCurrent[1] * PI / 180;
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
            Control[3] = speed;
        }
        else if (GetAsyncKeyState('S') & 0x8000) {
            Control[3] = -speed;
        }else{
            Control[3] = 0;
        }
        if (GetAsyncKeyState('A') & 0x8000){
            Control[4] = -speed;
            //Control[0] = -1 * PI / 180;
        }else if (GetAsyncKeyState('D') & 0x8000){
            Control[4] = speed;
            //Control[0] = 1 * PI / 180;
        }else{
            Control[4] = 0;
            //Control[0] = 0;
        }
        if (GetKeyState(VK_LSHIFT) & 0x8000){
            Control[5] = -speed;
        }else if (GetAsyncKeyState(VK_SPACE) & 0x8000){
            Control[5] = speed;
        }else{
            Control[5] = 0;
        }

        CameraData_Remain.lock();
        //变焦试验
        if (GetKeyState('Z') & 0x8000 && Camera_1.F < 10) {
            Camera_1.F = Camera_1.F+0.1;
        }
        if (GetAsyncKeyState('C') & 0x8000 && Camera_1.F > 0.5) {
            Camera_1.F = Camera_1.F - 0.1;
        }
        if (GetAsyncKeyState('X') & 0x8000) {
            Camera_1.F = 1;
        }
        camera_set(Camera_1,Control[0], Control[1], Control[2], Control[3], Control[4], Control[5]);
        CameraData_Remain.unlock();

        

    }
}


int main() {
    Rasterization_function RT_func;
    RT_func.SetFrameBuffer(pScreen[0], pScreen[1]);

    std::thread render(Render_thread);
    std::thread control(control_thread);

    render.join();
    control.join();
    return 0;
}
