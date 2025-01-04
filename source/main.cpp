#include <iostream>
#include<graphics.h>  //EasyX图形库
#include <thread>
#include <windows.h>
#include <vector>
#include <string>
#include <shared_mutex>
#include <emmintrin.h> // 包含SSE2指令集的头文件

#define PI 3.14159265358979

//moon-engine

//分辨率
int screen_1[] = { 960, 540 };
int screen_2[] = {1280, 720};
int* screen = screen_1;
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
    std::vector <Mpoint> normal_vector = {};
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

//old codes
#if 0
//实时变量
std::vector<double> pointRuntime;//实时顶点数组
std::vector<int> triangleRuntime;//实时三角面数据数组
std::vector <double> NormalVectorRuntime;//面的法向量
//三角面数组内，整数数字代表顶点数组内一个点的位置
std::vector<int> TmpBuffer;
//实时中间暂时缓存数组（render函数执行完的结果缓存，接下来这个会交给光栅化处理）
#endif


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

    return 1/y;
}
//选择使用的平方根函数
//使用哪种平方根
double (*Qsqrt)(double) = KQsqrt;

//old codes
#if 0
//old

//计算线在近平面的交点坐标(屏幕坐标)   相机          另一个点                 待处理点           输出(屏幕上坐标)
inline void Get_Cross_Point(Camera_data &Rc, double orig[3], double pointWP[3], int output[2]) {
    //待处理点和camera的front方向点的向量
    double nL1[] = { Rc.Forward_vec[0] - pointWP[0] +Rc.Camera[0],
                             Rc.Forward_vec[1] - pointWP[1] + Rc.Camera[1],
                             Rc.Forward_vec[2] - pointWP[2] + Rc.Camera[2] };
    //待处理点和另一点的向量
    double nL2[] = { orig[0] - pointWP[0],
                             orig[1] - pointWP[1],
                             orig[2] - pointWP[2] };

    //待处理点到近平面的距离
    double distance = (Rc.Forward_vec[0] * nL1[0] + Rc.Forward_vec[1] * nL1[1] + Rc.Forward_vec[2] * nL1[2]) / Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]);
    //nL2的模
    double Length = Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]);
    //nL2 向量和nF向量的cos值
    double cosV = (Rc.Forward_vec[0] * nL2[0] + Rc.Forward_vec[1] * nL2[1] + Rc.Forward_vec[2] * nL2[2]) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * Qsqrt(nL2[0] * nL2[0] + nL2[1] * nL2[1] + nL2[2] * nL2[2]));
    double n_plane[3];
    if (distance == 0) {
        //恰好距离零时处理
        n_plane[0] = pointWP[0] - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = pointWP[1] - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = pointWP[2] - Rc.Forward_vec[2] - Rc.Camera[2];
    }else {
        double Tmp[] = { (nL2[0] * (distance / cosV) / Length) + pointWP[0], (nL2[1] * (distance / cosV) / Length) + pointWP[1], ((nL2[2] * distance) / (cosV * Length)) + pointWP[2] };
        n_plane[0] = Tmp[0] - Rc.Forward_vec[0] - Rc.Camera[0];
        n_plane[1] = Tmp[1] - Rc.Forward_vec[1] - Rc.Camera[1];
        n_plane[2] = Tmp[2] - Rc.Forward_vec[2] - Rc.Camera[2];
    };
    output[0] = static_cast<int> ((screen[0] / 2) + (Rc.F * (screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2]))));
    output[1] = static_cast<int> ((screen[1] / 2) - (Rc.F * (screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2]))));

}

// 计算面的法向量                                            点数组                    三角面数组                        输出法向量数组
inline void Get_NormalVector(std::vector <double> Point, std::vector <int> Triangle, std::vector<double>& outVector) {
    for (int i = 0; i < Triangle.size(); i += 3) {
        double na[3] = { Point.at(Triangle.at(i + 1) * 3) - Point.at(Triangle.at(i) * 3), 
            Point.at(Triangle.at(i + 1) * 3 + 1) - Point.at(Triangle.at(i) * 3 + 1), 
            Point.at(Triangle.at(i + 1) * 3 + 2) - Point.at(Triangle.at(i) * 3 + 2) 
        };
        double nb[3] = { Point[Triangle.at(i + 2) * 3] - Point[Triangle.at(i + 1) * 3], 
            Point[Triangle.at(i + 2) * 3 + 1] - Point[Triangle.at(i + 1) * 3 + 1], 
            Point[Triangle.at(i + 2) * 3 + 2] - Point[Triangle.at(i + 1) * 3 + 2] 
        };
        double nV[3] = { (nb[1] * na[2] - nb[2] * na[1]), (nb[2] * na[0] - nb[0] * na[2]), (nb[0] * na[1] - nb[1] * na[0]) };
        double Length = Qsqrt(nV[0]* nV[0] + nV[1] * nV[1] + nV[2] * nV[2]);
        outVector.push_back(nV[0] / Length);
        outVector.push_back(nV[1] / Length);
        outVector.push_back(nV[2] / Length);
    }
}

// render               摄像机结构体                                 点数组                    三角面数组                                 法向量数组                                  输出数组
void Render_V(Camera_data &Rc, std::vector <double> Point, std::vector <int> Triangle, std::vector <double> NormalVector, std::vector<int>& out) {
    //点的可见性检查数组
    std::vector <bool> Pinfo;
    //面的正面性检查数组
    std::vector <bool> FTinfo;
    //转化到平面坐标轴上的顶点的临时数组
    std::vector <int> TransformTriangle;
    
    double outTmp[3] = { 0,0,0 };

    //在以点为单位先获取顶点的可渲染信息和渲染好可见点(预处理)
    for (int i = 0; i < Point.size(); i += 3) {
        //获得顶点
        double P[3] = { Point[i], Point[i + 1], Point[i + 2] };
        double n_P[] = { P[0] - Rc.Camera[0],
                                  P[1] - Rc.Camera[1],
                                  P[2] - Rc.Camera[2] };
        double LO = Qsqrt((n_P[0]) * (n_P[0]) + (n_P[1]) * (n_P[1]) + (n_P[2]) * (n_P[2]));
        double cosV;//cos 值
        //点与camera重合情况处理
        if (LO == 0) {
            Pinfo.push_back(FALSE);
            TransformTriangle.push_back(0);
            TransformTriangle.push_back(0);
            continue;
        } else {
            cosV = (Rc.Forward_vec[0] * n_P[0] + Rc.Forward_vec[1] * n_P[1] + Rc.Forward_vec[2] * n_P[2]) / (Qsqrt(Rc.Forward_vec[0] * Rc.Forward_vec[0] + Rc.Forward_vec[1] * Rc.Forward_vec[1] + Rc.Forward_vec[2] * Rc.Forward_vec[2]) * LO);
            //计算cos
        }

        if (cosV > 0 && LO*cosV >= Rc.R) {   //当顶点在摄像机前方时cos为正时 
            //添加点可见的信息
            Pinfo.push_back(TRUE);
            //计算映射到近平面的坐标(以世界坐标轴)
            outTmp[0] = (n_P[0] * Rc.R) / (LO * cosV);
            outTmp[1] = (n_P[1] * Rc.R) / (LO * cosV);
            outTmp[2] = (n_P[2] * Rc.R) / (LO * cosV);
            
            //映射到近平面的坐标点和方向点的向量
            double n_plane[] = { outTmp[0] - Rc.Forward_vec[0], outTmp[1] - Rc.Forward_vec[1], outTmp[2] - Rc.Forward_vec[2] };
            
            TransformTriangle.push_back(static_cast<int> ( (screen[0] / 2) + (Rc.F* ( screen[0] / 2) * (Rc.Y_vec[0] * n_plane[0] + Rc.Y_vec[1] * n_plane[1] + Rc.Y_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Y_vec[0] * Rc.Y_vec[0] + Rc.Y_vec[1] * Rc.Y_vec[1] + Rc.Y_vec[2] * Rc.Y_vec[2]) ) )) );
            TransformTriangle.push_back(static_cast<int> ( (screen[1] / 2) - (Rc.F* ( screen[0] / 2) * (Rc.Z_vec[0] * n_plane[0] + Rc.Z_vec[1] * n_plane[1] + Rc.Z_vec[2] * n_plane[2]) / (tan(Rc.Horizen) * Rc.R * Qsqrt(Rc.Z_vec[0] * Rc.Z_vec[0] + Rc.Z_vec[1] * Rc.Z_vec[1] + Rc.Z_vec[2] * Rc.Z_vec[2]) ) )) );
            //平面坐标轴映射，                                                                           坐标已适配direcX坐标系

        } else  {            //不可见点情况处理(位于近平面之后)
            Pinfo.push_back(FALSE);
            TransformTriangle.push_back(0);
            TransformTriangle.push_back(0);
            continue;

        }
    };
    //以面为单位将每个面的坐标信息提取，并二次分割处理有不可见点的平面（一个点不可见的情况；两个点不可见的情况*(unfinish)；三个点都不可见的情况）
    for (int i = 0; i < Triangle.size(); i += 3) {
        //测试正面用向量
        double nTest[3] = { Point[Triangle[i] * 3] - Rc.Camera[0], Point[Triangle[i] * 3 + 1] - Rc.Camera[1], Point[Triangle[i] * 3 + 2] - Rc.Camera[2] };
        //储存面的正面性信息(后面剔除背面)
        (nTest[0] * NormalVector[i] + nTest[1] * NormalVector[i + 1] + nTest[2] * NormalVector[i + 2]) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
        //面全可见的情况（主要)
        if (Pinfo[Triangle[i]] && Pinfo[Triangle[i + 1]] && Pinfo[Triangle[i + 2]] && FTinfo[i / 3]) {
            out.push_back(TransformTriangle[Triangle[i] * 2]);
            out.push_back(TransformTriangle[Triangle[i] * 2 + 1]);

            out.push_back(TransformTriangle[Triangle[i + 1] * 2]);
            out.push_back(TransformTriangle[Triangle[i + 1] * 2 + 1]);

            out.push_back(TransformTriangle[Triangle[i + 2] * 2]);
            out.push_back(TransformTriangle[Triangle[i + 2] * 2 + 1]);
        }else if ( FTinfo[i / 3] ) {
            //不完整面处理(仍然检查总体面正面可见性)
            int num = static_cast<int>(not Pinfo[Triangle[i]]) + static_cast<int>(not Pinfo[Triangle[i + 1]]) + static_cast<int>(not Pinfo[Triangle[i + 2]]);
            if (num == 3) continue; //排除完全不可见
            //          前顶点        后顶点      中间点   (看情况各自分配是不可见点还是可见点)
            double previous[4], next[4], medium[4];
            //          切点1                  切点2
            int TmpClipOut_1[2], TmpClipOut_2[2];
            bool num_bool = (num == 1) ? TRUE : FALSE;

            //for循环分辨情况//这样做是为了最后生成的点的数组仍然是正面顺时针的顺序
            for (int e = 0; e < 3; e++) {
                if (num_bool ^ Pinfo[Triangle[i + e]]) {//获得待处理点            //此处的异或运算可以分辨是1情况还是2情况
                    medium[0] = Point[Triangle[i + e] * 3];
                    medium[1] = Point[Triangle[i + e] * 3 + 1];
                    medium[2] = Point[Triangle[i + e] * 3 + 2];
                    medium[3] = i + e;
                    previous[0] = Point[Triangle[i + (e + 5) % 3] * 3];
                    previous[1] = Point[Triangle[i + (e + 5) % 3] * 3 + 1];
                    previous[2] = Point[Triangle[i + (e + 5) % 3] * 3 + 2];
                    previous[3] = i + (e + 5) % 3;
                    next[0] = Point[Triangle[i + (e + 7) % 3] * 3];
                    next[1] = Point[Triangle[i + (e + 7) % 3] * 3 + 1];
                    next[2] = Point[Triangle[i + (e + 7) % 3] * 3 + 2];
                    next[3] = i + (e + 7) % 3;
                    break;
                }
            }
            Get_Cross_Point(Rc, previous, medium, TmpClipOut_1);
            Get_Cross_Point(Rc, next, medium, TmpClipOut_2);
            if (num_bool) {
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2 + 1]);
                out.push_back(TmpClipOut_1[0]);
                out.push_back(TmpClipOut_1[1]);
                out.push_back(TmpClipOut_2[0]);
                out.push_back(TmpClipOut_2[1]);
                //此处一个不可见点输出两个三角面
                out.push_back(TmpClipOut_2[0]);
                out.push_back(TmpClipOut_2[1]);
                out.push_back(TransformTriangle[(Triangle[static_cast<int>(next[3])] * 2)]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(next[3])] * 2 + 1]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(previous[3])] * 2 + 1]);
                //后续光栅化加入纹理映射时，记得在这加纹理坐标转换！！!!!!
            }else if(not num_bool){
                out.push_back(TmpClipOut_1[0]);
                out.push_back(TmpClipOut_1[1]);
                out.push_back(TransformTriangle[(Triangle[static_cast<int>(medium[3])] * 2)]);
                out.push_back(TransformTriangle[Triangle[static_cast<int>(medium[3])] * 2 + 1]);
                out.push_back(TmpClipOut_2[0]);
                out.push_back(TmpClipOut_2[1]);
                //wait for finish
            }


            


        };
    }
}

//old

#endif

//计算线在近平面的交点坐标new                相机                            一个点                         另一个点                           输出(屏幕上坐标)
inline void Get_CrossPoint_New(const Camera_data& Rc, const Mpoint &origin, const Mpoint &process_point, Mpoint &output) {
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
inline void Get_NormalVector_New(Mmesh &cMesh) {
    Mpoint normal_vector;
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
        normal_vector.x = n_V[0] / Length;
        normal_vector.y = n_V[1] / Length;
        normal_vector.z = n_V[2] / Length;
        cMesh.normal_vector.push_back(normal_vector);
    }
}


// New render       摄像机结构体                          mesh                    输出数组mesh
void Render_New(const Camera_data& Rc, Mmesh& cMesh, Mmesh& out_mesh) {
    //点的可见性检查数组//面的正面性检查数组
    std::vector <bool> Pinfo, FTinfo;
    //转化到平面坐标轴上的顶点的临时数组
    std::vector <Mpoint> Transform_vertices;
    //计算法向量
    cMesh.normal_vector.clear();
    Get_NormalVector_New(cMesh);
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
        (nTest[0] * cMesh.normal_vector[i].x + nTest[1] * cMesh.normal_vector[i].y + nTest[2] * cMesh.normal_vector[i].z) < 0 ? FTinfo.push_back(TRUE) : FTinfo.push_back(FALSE);
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
                    Get_CrossPoint_New(Rc, cMesh.vertices[previous], cMesh.vertices[medium], ClipOut_1);
                    Get_CrossPoint_New(Rc, cMesh.vertices[next], cMesh.vertices[medium], ClipOut_2);
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



struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3,1,0},{4,1,0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };
};

cube cubeData;

//old codes
#if 0
//old
//绘图线程
void draw_thread(){

    initgraph(screen[0], screen[1]);
    setbkcolor(BLACK); // 背景色
    settextcolor(GREEN);//文字颜色
    setlinecolor(RGB(255,255,255)); // 线框颜色
    setfillcolor(RGB(128, 128, 128)); 

    while (TRUE){
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }

        auto start = std::chrono::high_resolution_clock::now();   //测帧
        //实时数组复位
        pointRuntime.clear();
        triangleRuntime.clear();
        NormalVectorRuntime.clear();
        //实时全局顶点与面的数组变量更新
        for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
            pointRuntime.push_back(cubeData.Cpoint[i][0]);
            pointRuntime.push_back(cubeData.Cpoint[i][1]);
            pointRuntime.push_back(cubeData.Cpoint[i][2]);
        }
        for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
            triangleRuntime.push_back(cubeData.Ctriangle[i][0]);
            triangleRuntime.push_back(cubeData.Ctriangle[i][1]);
            triangleRuntime.push_back(cubeData.Ctriangle[i][2]);
        }
        
        //互斥锁
        camera_Remain.lock();
        //render
        Get_NormalVector(pointRuntime, triangleRuntime, NormalVectorRuntime);
        Render_V(Camera_1,pointRuntime, triangleRuntime, NormalVectorRuntime, TmpBuffer);
        //释放互斥锁
        camera_Remain.unlock();

        BeginBatchDraw();
        cleardevice(); // 清空屏幕
        //临时光栅化
        //draw triangle
        for (int i = 0; i < TmpBuffer.size(); i += 6) {
            POINT pts[] = { {TmpBuffer.at(i), TmpBuffer.at(i + 1)},{TmpBuffer.at(i + 2), TmpBuffer.at(i + 3)},{TmpBuffer.at(i + 4), TmpBuffer.at(i + 5)} };
            fillpolygon(pts, 3);
        }

        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start;
        double Frames = 1 / elapsed_seconds.count();

        std::string title_str = "MOON_Engine_Build_version_0.3.0  Compilation_Date:";
        title_str.append(__DATE__);
        std::string Frames_str = "Frames:"+std::to_string(Frames);

        const char* title = title_str.c_str();
        const char* Frames_char = Frames_str.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) +textheight(Frames_char) + 4, "By_H  Type ESC to close this program!");

        EndBatchDraw();

        TmpBuffer.clear();
        //重置缓存

    }

}
#endif


void draw_thread_New(){
    initgraph(screen[0], screen[1]);
    setbkcolor(BLACK); // 背景色
    settextcolor(GREEN);//文字颜色
    setlinecolor(RGB(255, 255, 255)); // 线框颜色
    setfillcolor(RGB(128, 128, 128));
    std::vector <Mmesh> mesh_list;
    Mmesh cube_mesh;
    for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
        Mpoint& each_point = cube_mesh.vertices.emplace_back();
        each_point.x = cubeData.Cpoint[i][0];
        each_point.y = cubeData.Cpoint[i][1];
        each_point.z = cubeData.Cpoint[i][2];
    }
    for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
        Mface& each_face = cube_mesh.faces.emplace_back();
        each_face.sequnce[0] = cubeData.Ctriangle[i][0];
        each_face.sequnce[1] = cubeData.Ctriangle[i][1];
        each_face.sequnce[2] = cubeData.Ctriangle[i][2];
    }

    mesh_list.push_back(cube_mesh);

    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }
        auto start = std::chrono::high_resolution_clock::now();   //测帧
        BeginBatchDraw();
        cleardevice(); // 清空屏幕
        for (Mmesh &each_mesh : mesh_list) {
            Mmesh out;
            camera_Remain.lock();
            Render_New(Camera_1, each_mesh, out);
            camera_Remain.unlock();
            //此时的out_mesh中vertices按顺序以三个点为一个面，不使用成员faces
            for (unsigned int k = 0; k < out.vertices.size(); k +=3) {
                POINT p[] = { static_cast<long>(out.vertices[k].x) , static_cast<long>(out.vertices[k].y) ,static_cast<long>(out.vertices[k+1].x) , static_cast<long>(out.vertices[k+1].y) , static_cast<long>(out.vertices[k+2].x) , static_cast<long>(out.vertices[k+2].y) };
                fillpolygon(p, 3);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start;
        double Frames = 1 / elapsed_seconds.count();

        std::string title_str = "MOON_Engine_Build_version_0.4.0  Compilation_Date:";
        title_str.append(__DATE__);
        std::string Frames_str = "Frames:" + std::to_string(Frames);

        const char* title = title_str.c_str();
        const char* Frames_char = Frames_str.c_str();
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

    double angleNOW[] = { 0,0 };
    double CS[] = {0,0,0,0,0,0};
    double speed = 0.02;

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

            angleNOW[0] = angleNOW[0] + deltaX * 0.4;
            angleNOW[1] = angleNOW[1] - deltaY * 0.4;
            //标准限制化处理数据

            if (angleNOW[0] > 360) {
                angleNOW[0] = angleNOW[0] - 360;
            }else if (angleNOW[0] < -360) {
                angleNOW[0] = angleNOW[0] + 360;
            }
            if (angleNOW[1] > 90) {
                angleNOW[1] = 90;
            }else if(angleNOW[1] < -90) {
                angleNOW[1] = -90;
            }

            //处理camera镜头方向
            CS[1] = angleNOW[0] * PI / 180;
            CS[2] = angleNOW[1] * PI / 180;
        }
        //把鼠标移到中心
        MoveMouseToCenter();

        //检测操作
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
        camera_set(Camera_1,CS[0], CS[1], CS[2], CS[3], CS[4], CS[5]);
        camera_Remain.unlock();

        

    }
}


int main() {
    
    std::thread draw(draw_thread_New);
    std::thread control(control_thread);

    draw.join();
    control.join();
    return 0;
}
