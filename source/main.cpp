#include "moon.h"

//#include<graphics.h>  //EasyX图形库

#include <string>
#include <thread>
#include <iostream>
#include <windows.h>
#include <shared_mutex>

//moon-engine

//分辨率
long screen_1[] = { 960, 540 };
long screen_2[] = {1280, 720};
long screen_3[] = { 640, 480 };
long screen_4[] = { 2560, 1600 };
long* ptrScreen = screen_1;
//初始化

//创建一个相机
Camera_data Camera_1;

Camera_data sharedStruct;
std::mutex CameraData_Remain;
//互斥锁


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



// 函数用于将鼠标移动到屏幕中央
void MoveMouseToCenter() {
    // 获取屏幕的宽度和高度
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 计算屏幕中央的坐标
    int centerX = screenWidth >> 1;
    int centerY = screenHeight >> 1 ;
    // 将鼠标移动到屏幕中央
    SetCursorPos(centerX, centerY);
}

//相机设置                     目标相机                       倾斜角度                    水平角度                      纵向角度          前方向移动量//横方向变化量//纵方向变化量 
void camera_set( Camera_data &Target_camera, double revolve_incline, double revolve_hori, double revolve_verti, double moveF,double moveH,double moveZ){
    double front_tmp[] = {Target_camera.Forward[0], Target_camera.Forward[1], Target_camera.Forward[2]};
    double y_tmp[] = { Target_camera.y[0], Target_camera.y[1], Target_camera.y[2] };
    double z_tmp[] = { Target_camera.z[0], Target_camera.z[1], Target_camera.z[2] };

    double ft[3]{0,0,0}, yt[3]{0,0,0}, zt[3]{0,0,0};
    //中间临时缓冲值
    
    //合并过来的移动代码
    if (moveF != 0) {
        double a_front = 1 - (moveF / Target_camera.NearPlane);

        Target_camera.Camera[0] = Target_camera.Camera[0] - Target_camera.Forward_vec.x * a_front + Target_camera.Forward_vec.x;
        Target_camera.Camera[1] = Target_camera.Camera[1] -Target_camera.Forward_vec.y * a_front + Target_camera.Forward_vec.y;
        Target_camera.Camera[2] = Target_camera.Camera[2] -Target_camera.Forward_vec.z * a_front + Target_camera.Forward_vec.z;
    }//前向

    if (moveH != 0) {
        Vertex horizon_vec = { Target_camera.Camera[0] - Target_camera.move_vec.x, Target_camera.Camera[1] - Target_camera.move_vec.y };
        double a_horizon = 1 - (moveH / GetLength(horizon_vec));

        Target_camera.Camera[0] = horizon_vec.x * a_horizon + Target_camera.move_vec.x;
        Target_camera.Camera[1] = horizon_vec.y * a_horizon + Target_camera.move_vec.y;
        Target_camera.move_vec.x = Target_camera.move_vec.x + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move_vec.y + Target_camera.Camera[1];
    }//横向

    if (moveZ != 0) {
        Target_camera.Camera[2] += moveZ;
    }//纵向

    //相机倾斜
    if (revolve_incline != 0){
        double cosV = cos(-revolve_incline);
        double sinV = sin(-revolve_incline);

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
    } else {
        Target_camera.move_vec.x = Target_camera.move[0] + Target_camera.Camera[0];
        Target_camera.move_vec.y = Target_camera.move[1] + Target_camera.Camera[1];
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


//old
struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3, 1, 0},{4, 1, 0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };

    double Ccolor[12][3] = { {0.15, 0.15, 0.15},  {0.15, 0.15, 0.15},  {0.7, 0.7, 0.7},   {0.7, 0.7, 0.7}, {0.5, 0.5, 0.5},  {0.5, 0.5, 0.5},  {0.1, 0.1, 0.1},  {0.1, 0.1, 0.1}, {0.3, 0.3, 0.3}, {0.3, 0.3, 0.3},  {0.15, 0.15, 0.15},  {0.15, 0.15, 0.15}};
};
cube cubeData;



//old
#if 0 
void Render_thread() {
    initgraph(ptrScreen[0], ptrScreen[1]);
    setbkcolor(BLACK);
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
                vertex& each_point = cube_mesh.vertices.emplace_back();
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

            for (int c = 0; c < sizeof(cubeData.Ccolor) / sizeof(cubeData.Ccolor[0]); ++c) {
                Color& each_color = cube_mesh.color.emplace_back();
                each_color.R = cubeData.Ccolor[c][0] ;
                each_color.G = cubeData.Ccolor[c][1];
                each_color.B = cubeData.Ccolor[c][2];
            }
            mesh_list.emplace_back(cube_mesh);
        }
    }
    }

    //帧缓存设置
    Graphics_func.SetBuffer(ptrScreen[0], ptrScreen[1]);

    while (TRUE) {
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            return;
        }
        int faces_num = 0;
        double Frames;
        auto start = std::chrono::high_resolution_clock::now();   //测帧

        BeginBatchDraw();
        cleardevice(); // 清空屏幕

        auto tech_start = std::chrono::high_resolution_clock::now();

        mesh_tf out;
        CameraData_Remain.lock();
        for (Mmesh& each_mesh : mesh_list) {
            Tf_func.Perspective(Camera_1, ptrScreen, each_mesh, out);
        }
        CameraData_Remain.unlock();


        Graphics_func.CleanBuffer();
        for (unsigned int k = 0; k < out.vertices_2d.size(); k+=3) {
            Color c = out.color[k / 3];
            Graphics_func.DrawTriangle(out.vertices_2d[k], out.vertices_2d[k+1], out.vertices_2d[k+2], c);
        }

        auto tech_end = std::chrono::high_resolution_clock::now();


        //此处io性能瓶颈
        for (int y = 0; y < ptrScreen[1]; ++y) {
            for (int x = 0; x < ptrScreen[0]; ++x) {
               Color p = Graphics_func.GetPixelColor(x,y);
               if (p.R == 0 && p.G == 0 && p.B == 0) continue;
               COLORREF color = RGB(p.R * 255, p.G * 255, p.B * 255);
               putpixel(x,y, color);
               
               //plz重写此处读取帧缓存
            }
        }
        
          
        /*
        //临时实现光栅化
        for (unsigned int k = 0; k < out.vertices_2d.size(); k += 3) {
            POINT triangle[] = { static_cast<long>(out.vertices_2d[k].x) , static_cast<long>(out.vertices_2d[k].y) ,static_cast<long>(out.vertices_2d[k + 1].x) , static_cast<long>(out.vertices_2d[k + 1].y) , static_cast<long>(out.vertices_2d[k + 2].x) , static_cast<long>(out.vertices_2d[k + 2].y) };
            fillpolygon(triangle, 3);
        }
        */


        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start;
        Frames = 1/elapsed_seconds.count();
        faces_num = out.vertices_2d.size() / 3;

        std::chrono::duration<double> PSTcosttime = tech_end - tech_start;
        double PSTcosttime_process = 1/PSTcosttime.count();


        std::string title_str = "MOON_Engine_Alpha_0.4.5  Compilation_Date:";
        title_str.append(__DATE__);
        std::string info = "FPS: " + std::to_string(Frames);
        std::string info2 = ("  Theoretical_FPS:" + std::to_string(PSTcosttime_process));
        std::string Faces_number = "  Faces: " + std::to_string(faces_num);
        info.append(info2);
        info.append(Faces_number);

        const char* title = title_str.c_str();
        const char* Frames_char = info.c_str();
        outtextxy(4, 4, title);
        outtextxy(4, textheight(title) + 4, Frames_char);
        outtextxy(4, textheight(title) + textheight(Frames_char) + 4, "By_H  press ESC to close this program!");

        fillcircle(ptrScreen[0] / 2, ptrScreen[1] / 2, 2);

        FlushBatchDraw();
    }

}
#endif


//new codes
//under development
void Render_thread_V2() {

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);

    SDL_Window* window = SDL_CreateWindow("Moon_engine_Alpha_0.5.1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ptrScreen[0], ptrScreen[1], SDL_WINDOW_RESIZABLE);


    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);


    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ptrScreen[0], ptrScreen[1]);

    std::vector <Mmesh> mesh_list;

    //cube test压力测试
    for (int h = -2; h < 0; h += 2) {
        for (int t = -20; t < 20; t += 2) {
            for (int w = -20; w < 20; w += 2) {
                Mmesh cube_mesh;
                for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
                    Vertex& each_point = cube_mesh.vertices.emplace_back();
                    each_point.x = cubeData.Cpoint[i][0] + t;
                    each_point.y = cubeData.Cpoint[i][1] + w;
                    each_point.z = cubeData.Cpoint[i][2] + h;
                }
                for (int i = 0; i < sizeof(cubeData.Ctriangle) / sizeof(cubeData.Ctriangle[0]); i += 1) {
                    Face& each_face = cube_mesh.faces.emplace_back();
                    each_face.index[0] = cubeData.Ctriangle[i][0];
                    each_face.index[1] = cubeData.Ctriangle[i][1];
                    each_face.index[2] = cubeData.Ctriangle[i][2];
                }

                for (int c = 0; c < sizeof(cubeData.Ccolor) / sizeof(cubeData.Ccolor[0]); ++c) {
                    Color& each_color = cube_mesh.color.emplace_back();
                    each_color.R = cubeData.Ccolor[c][0];
                    each_color.G = cubeData.Ccolor[c][1];
                    each_color.B = cubeData.Ccolor[c][2];
                }
                mesh_list.emplace_back(cube_mesh);
            }
        }
    }

    Mmesh testModel;
    //                                            0                 1                   2                   3                       4                       5                       6                   7                           z8              y9                  x10
    testModel.vertices = { {0.1, -0.1 ,-0.1}, {-0.1, -0.1, -0.1},{-0.1, 0.1, -0.1}, { 0.1, 0.1 , -0.1 },     {0.1, -0.1 ,0.1}, {-0.1, -0.1, 0.1},{-0.1, 0.1, 0.1}, { 0.1, 0.1 ,0.1 },    {0.1, 0.1, 2}, {0.1, 2 ,0.1}, {2, 0.1, 0.1} };
    testModel.faces = { {7, 6, 9}, {2, 9, 6}, {3, 9, 2}, { 3, 7, 9 },            {3, 2, 1},  {3, 1, 0},         {2, 5, 1}, {2, 6, 5},        {5, 4, 1}, {1, 4, 0},       {5, 8, 4}, {6, 8, 5}, {7, 8, 6}, {4, 8, 7},        {7, 10, 4}, {3, 10, 7}, {0, 10, 3}, {0, 4, 10} };
    testModel.color = {   {0.3, 0.9, 0.3}, {0.5, 0.8, 0.5}, {0.1, 0.4, 0.1},  {0.1, 0.9, 0.1},       {0.2, 0.2, 0.2},  {0.2, 0.2, 0.2},        {0.8, 0.8, 0.8},  {0.8, 0.8, 0.8},       {0.6, 0.6, 0.6}, {0.6, 0.6, 0.6},      {0.3, 0.3, 0.9},  {0.5, 0.5, 0.8} ,{0.1, 0.1, 0.4},  {0.1, 0.1, 0.9},        {0.9, 0.3, 0.3},   {0.8, 0.5, 0.5}, {0.4, 0.1, 0.1},  {0.9, 0.1, 0.1}};
    mesh_list.push_back(testModel);


    //帧缓存设置
    Buffer Framebuffer_1;
    Framebuffer_1.SetBuffer(ptrScreen[0], ptrScreen[1]);

    SDL_Event event{};
    while (true) {
        SDL_PollEvent(&event);
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                std::cout << "lost focus" << std::endl;
            }
            else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return;
            }
            break;

        case SDL_QUIT:
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            return;
        }

        double Frames;
        auto start = std::chrono::high_resolution_clock::now();   //测帧

        Framebuffer_1.CleanBuffer();

        //代替互斥锁的作用
        Camera_data tmpCamera_1 = Camera_1;
        
        for (Mmesh & each_mesh : mesh_list) {
            Render(tmpCamera_1, Framebuffer_1, ptrScreen, each_mesh);
        }
        
        auto tech_end = std::chrono::high_resolution_clock::now();

        /*old
        SDL_RenderClear(renderer);

        //此处io性能瓶颈
        for (int y = 0; y < ptrScreen[1]; ++y) {
            for (int x = 0; x < ptrScreen[0]; ++x) {
                Color p = Graphics_func.GetPixelColor(x, y);
                if (p.R == 0 && p.G == 0 && p.B == 0) continue; 
                SDL_PutPixel(renderer, x, y, p.R, p.G, p.B, 1);
                
                //plz重写此处读取帧缓存
            }
        }
        
        SDL_RenderPresent(renderer);
        
        */
        

        // 获取纹理的像素数据
        // 将帧缓存数据更新到纹理
        int pitch;
        void* pixels;
        SDL_LockTexture(texture, nullptr, &pixels, &pitch);
        uint32_t* pixelData = static_cast<uint32_t*>(pixels);
        for (int y = 0; y < ptrScreen[1]; ++y) {
            for (int x = 0; x < ptrScreen[0]; ++x) {
                Color p = Framebuffer_1.GetPixelColor(x, y);

                //test codes
                double D = Framebuffer_1.GetDepth(x, y);

                
                uint8_t r = static_cast<uint8_t>(p.R * 255.0);
                uint8_t g = static_cast<uint8_t>(p.G * 255.0);
                uint8_t b = static_cast<uint8_t>(p.B * 255.0);
                


                //test codes
                //test render depth picture
                /*
                r = static_cast<uint8_t>(D * 255.0 );
                g = static_cast<uint8_t>(D * 255.0);
                b = static_cast<uint8_t>(D * 255.0 );
                */

                uint32_t pixel = (255 << 24) | (r << 16) | (g << 8) | b;
                int index = y * (pitch / sizeof(uint32_t)) + x;
                pixelData[index] = pixel;
            }
        }
        
        SDL_UnlockTexture(texture);


        // 将纹理渲染到屏幕上
        
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);


        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start;
        Frames = 1 / elapsed_seconds.count();

        std::chrono::duration<double> PSTcosttime = tech_end - start;
        double PSTcosttime_process = 1 / PSTcosttime.count();

        
        // 刷新渲染器
        SDL_RenderPresent(renderer);

        std::cout << Frames <<", "<< PSTcosttime_process << std::endl;

        /*old
        
        
        std::string title_str = "MOON_Engine_Alpha_0.4.5  Compilation_Date:";
        title_str.append(__DATE__);
        std::string info = "FPS: " + std::to_string(Frames);
        std::string info2 = ("  Theoretical_FPS:" + std::to_string(PSTcosttime_process));
        std::string Faces_number = "  Faces: " + std::to_string(faces_num);
        info.append(info2);
        info.append(Faces_number);

        const char* title = title_str.c_str();
        const char* Frames_char = info.c_str();
        */
    }

}


//camera控制线程
void control_thread()
{
    POINT currentPos, centerPos = { GetSystemMetrics(SM_CXSCREEN)/ 2, GetSystemMetrics(SM_CYSCREEN)/2 };

    double angleCurrent[] = { 0,0 };
    double Control[] = {0,0,0,0,0,0};
    double speed = 0.03;

    Camera_1.Camera[2] = 1;//更改camera初始位置示范

    bool Puase = false;
    MoveMouseToCenter();
    while (true) {
        while (true) {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                Puase = Puase ? false : true;
                Sleep(200);
            }
            if (Puase) { 
                Sleep(2);
                continue;
            }else {
                MoveMouseToCenter();
                break;
            } 

        }

        Sleep(1); // 休眠一段时间
        GetCursorPos(&currentPos);
        

        int deltaX = currentPos.x - centerPos.x;
        int deltaY = currentPos.y - centerPos.y;

        angleCurrent[0] = angleCurrent[0] + deltaX * 0.4;
        angleCurrent[1] = std::clamp(angleCurrent[1] - deltaY * 0.4, -90.0, 90.0);
        //标准化数据

        if (angleCurrent[0] > 360) {
            angleCurrent[0] = angleCurrent[0] - 360;
        }if (angleCurrent[0] < -360) {
            angleCurrent[0] = angleCurrent[0] + 360;
        }


        //处理camera镜头方向
        Control[1] = angleCurrent[0] * PI / 180;
        Control[2] = angleCurrent[1] * PI / 180;
        
        MoveMouseToCenter();

        //检测操作

        if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
            speed =0.001;
        }else{
            speed = 0.02;
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
        }else if (GetAsyncKeyState('D') & 0x8000){
            Control[4] = speed;
        }else{
            Control[4] = 0;
        }

        if (GetKeyState(VK_LSHIFT) & 0x8000){
            Control[5] = -speed;
        }else if (GetAsyncKeyState(VK_SPACE) & 0x8000){
            Control[5] = speed;
        }else{
            Control[5] = 0;
        }

        //变焦test
        if (GetKeyState('Z') & 0x8000 && Camera_1.F < 10) {
            Camera_1.F = Camera_1.F+0.1;
        }
        if (GetAsyncKeyState('C') & 0x8000 && Camera_1.F > 0.5) {
            Camera_1.F = Camera_1.F - 0.1;
        }
        if (GetAsyncKeyState('X') & 0x8000) {
            Camera_1.F = 1;
        }

        if (GetAsyncKeyState('J') & 0x8000) {
            Control[0] = Control[0] + 1 * PI / 180;
        }
        else if (GetAsyncKeyState('K') & 0x8000) {
            Control[0] = Control[0] - 1 * PI / 180;
        }
        else if (GetAsyncKeyState('L') & 0x8000) {
            Control[0] = 0;
        }

        camera_set(Camera_1,Control[0], Control[1], Control[2], Control[3], Control[4], Control[5]);



    }
}



int main() {
    
    std::thread render(Render_thread_V2);
    std::thread control(control_thread);

    render.join();
    control.join();
    


    return 0 ;

}
