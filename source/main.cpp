#include "moon.h"

#include <string>
#include <windows.h>
#include <shared_mutex>

//moon-engine

//分辨率
long screen_1[] = { 960, 540 };
long screen_2[] = {1280, 720};
long screen_3[] = { 640, 480 };
long screen_4[] = { 2560, 1600 };
long screen_5[] = { 192, 108 };
long screen_6[] = { 1920, 1080};

long* ptrScreen = screen_1;
//long* ptrScreenTEST = screen_5;
//初始化

//创建相机
Camera Camera_1;
//Camera Camera_2;

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

//old codes
#if 0
//相机设置                     目标相机                       倾斜角度                    水平角度                      纵向角度          前方向移动量//横方向变化量//纵方向变化量 
void camera_set( Camera &Target_Camera, double revolve_incline, double revolve_hori, double revolve_verti, double moveF,double moveH,double moveZ){
    double front_tmp[] = {Target_Camera.Forward.x, Target_Camera.Forward.y, Target_Camera.Forward.z};
    double y_tmp[] = { Target_Camera.Y.x, Target_Camera.Y.y, Target_Camera.Y.z };
    double z_tmp[] = { Target_Camera.Z.x, Target_Camera.Z.y, Target_Camera.Z.z };

    double ft[3]{0,0,0}, yt[3]{0,0,0}, zt[3]{0,0,0};
    //中间临时缓冲值
    
    //合并过来的移动代码
    if (moveF != 0) {
        double a_front = 1 - (moveF / Target_Camera.NearPlane);

        Target_Camera.Pos.x = Target_Camera.Pos.x - Target_Camera.Forward_vec.x * a_front + Target_Camera.Forward_vec.x;
        Target_Camera.Pos.y = Target_Camera.Pos.y -Target_Camera.Forward_vec.y * a_front + Target_Camera.Forward_vec.y;
        Target_Camera.Pos.z = Target_Camera.Pos.z -Target_Camera.Forward_vec.z * a_front + Target_Camera.Forward_vec.z;
    }//前向

    if (moveH != 0) {
        Vec3 horizon_vec = { Target_Camera.Pos.x - Target_Camera.move_vec.x, Target_Camera.Pos.y - Target_Camera.move_vec.y, 0 };
        //Vec3 horizon_vec = Target_Camera.Pos - Target_Camera.move_vec;
        double a_horizon = 1 - (moveH / GetLength(horizon_vec));

        Target_Camera.Pos.x = horizon_vec.x * a_horizon + Target_Camera.move_vec.x;
        Target_Camera.Pos.y = horizon_vec.y * a_horizon + Target_Camera.move_vec.y;
        Target_Camera.move_vec.x = Target_Camera.move_vec.x + Target_Camera.Pos.x;
        Target_Camera.move_vec.y = Target_Camera.move_vec.y + Target_Camera.Pos.y;
    }//横向

    if (moveZ != 0) {
        Target_Camera.Pos.z += moveZ;
    }//纵向

    //相机倾斜
    if (revolve_incline != 0){
        double cosV = cos(-revolve_incline);
        double sinV = sin(-revolve_incline);

        z_tmp[1] = Target_Camera.Z.z*sinV + Target_Camera.Z.y*cosV;
        z_tmp[2] = Target_Camera.Z.z*cosV - Target_Camera.Z.y*sinV;

        y_tmp[1] = Target_Camera.Y.z * sinV + Target_Camera.Y.y * cosV;
        y_tmp[2] = Target_Camera.Y.z * cosV - Target_Camera.Y.y * sinV;
    }

    //相机纵向旋转
    if (revolve_verti != 0){
        double cosV = cos(revolve_verti);
        double sinV = sin(revolve_verti);

        front_tmp[0] = Target_Camera.Forward.x * cosV - Target_Camera.Forward.z * sinV;
        front_tmp[2] = Target_Camera.Forward.z * cosV + Target_Camera.Forward.x * sinV;

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
        Target_Camera.move_vec.y = Target_Camera.move.y * cosV + Target_Camera.move.x * sinV;
        Target_Camera.move_vec.x = Target_Camera.move.x * cosV - Target_Camera.move.y * sinV;
        Target_Camera.move_vec.x = Target_Camera.move_vec.x + Target_Camera.Pos.x;
        Target_Camera.move_vec.y = Target_Camera.move_vec.y + Target_Camera.Pos.y;
    } else {
        Target_Camera.move_vec.x = Target_Camera.move.x + Target_Camera.Pos.x;
        Target_Camera.move_vec.y = Target_Camera.move.y + Target_Camera.Pos.y;
    }


    Target_Camera.Forward_vec.x = front_tmp[0];
    Target_Camera.Forward_vec.y = front_tmp[1];
    Target_Camera.Forward_vec.z = front_tmp[2];

    
    Target_Camera.Y_vec.x = y_tmp[0] - Target_Camera.Forward_vec.x;
    Target_Camera.Y_vec.y = y_tmp[1] - Target_Camera.Forward_vec.y;
    Target_Camera.Y_vec.z = y_tmp[2] - Target_Camera.Forward_vec.z;

    Target_Camera.Z_vec.x = z_tmp[0] - Target_Camera.Forward_vec.x;
    Target_Camera.Z_vec.y = z_tmp[1] - Target_Camera.Forward_vec.y;
    Target_Camera.Z_vec.z = z_tmp[2] - Target_Camera.Forward_vec.z;
    
    return;
    
    
}
#endif

//old
struct cube {
    //                                       0        1        2          3        4        5         6        7    
    double Cpoint[8][3] = { {3,0,0},{4,0,0},{3, 1, 0},{4, 1, 0},{3,0,1},{4,0,1},{3,0.5,1},{4,0.5,1} };

    int Ctriangle[12][3] = {
        {0,5,1},{4,5,0},{7,2,3} ,{2,7,6} ,{7,5,6}, {5,4,6} ,{3,2,1} ,{2,0,1} ,{7,3,1},{5,7,1},{0,6,4},{0,2,6} };

    double Ccolor[12][3] = { {0.15, 0.15, 0.15},  {0.15, 0.15, 0.15},  {0.7, 0.7, 0.7},   {0.7, 0.7, 0.7}, {0.5, 0.5, 0.5},  {0.5, 0.5, 0.5},  {0.1, 0.1, 0.1},  {0.1, 0.1, 0.1}, {0.3, 0.3, 0.3}, {0.3, 0.3, 0.3},  {0.15, 0.15, 0.15},  {0.15, 0.15, 0.15}};
};
cube cubeData;



void EasyRenderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color textColor, int x, int y) {
    SDL_Color shadow_color = { 50, 50, 50, 255 };
    SDL_Surface* textSurface_2 = TTF_RenderUTF8_Solid(font, text, shadow_color);
    SDL_Rect textDestination_2 = { x + 1, y + 1, textSurface_2->w, textSurface_2->h };

    SDL_Texture* textTexture_2 = SDL_CreateTextureFromSurface(renderer, textSurface_2);
    SDL_RenderCopy(renderer, textTexture_2, nullptr, &textDestination_2);

    SDL_FreeSurface(textSurface_2);
    SDL_DestroyTexture(textTexture_2);

    SDL_Surface* textSurface_3 = TTF_RenderUTF8_Solid(font, text, textColor);
    SDL_Texture* textTexture_3 = SDL_CreateTextureFromSurface(renderer, textSurface_3);
    SDL_Rect shadowDestination = { x, y , textSurface_3->w, textSurface_3->h };
    SDL_RenderCopy(renderer, textTexture_3, nullptr, &shadowDestination);


    SDL_FreeSurface(textSurface_3);
    SDL_DestroyTexture(textTexture_3);
}


Buffer Render_thread(const Camera& c0, long Buffer_size[2], const long screen_in[2], const std::vector <Mesh>& mesh_list) {
    Buffer frame_chunk;
    frame_chunk.SetBuffer(Buffer_size[0], Buffer_size[1]);
    Render(c0, frame_chunk, screen_in, mesh_list);
    return std::move(frame_chunk);
}



//under development

void Display_thread() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    if (TTF_Init() == -1) {
        ExitProcess(21);
    }

    SDL_Window* window = SDL_CreateWindow(ENGINE_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, ptrScreen[0], ptrScreen[1], SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, ptrScreen[0], ptrScreen[1]);
    
    //test
     TTF_Font* font = TTF_OpenFont("resources/minecraft.ttf", 12);
     SDL_Color textColor = { 0, 192, 0, 255 };
    if (font == nullptr) ExitProcess(10);
    
    BS::thread_pool pool(8);

    std::vector <Mesh> mesh_list;
    //坐标轴模型
    Mesh testModel;
    //                                            0                 1                   2                   3                       4                       5                       6                   7                           z8              y9                  x10
    testModel.vertices = { {0.1, -0.1 ,-0.1}, {-0.1, -0.1, -0.1},{-0.1, 0.1, -0.1}, { 0.1, 0.1 , -0.1 },   
                                        {0.1, -0.1 ,0.1}, {-0.1, -0.1, 0.1},{-0.1, 0.1, 0.1}, { 0.1, 0.1 ,0.1 },   
                                        {0, 0, 1024}, {0, 1024,0.05}, {1024, 0, 0.05} };

    testModel.faces = { {7, 6, 9}, {2, 9, 6}, {3, 9, 2}, { 3, 7, 9 },    
                                    {3, 2, 1},  {3, 1, 0},  
                                    {2, 5, 1}, {2, 6, 5},    
                                    {5, 4, 1}, {1, 4, 0},
                                    {5, 8, 4}, {6, 8, 5}, {7, 8, 6}, {4, 8, 7},
                                    {7, 10, 4}, {3, 10, 7}, {0, 10, 3}, {0, 4, 10} };

    testModel.color = { {0.3, 0.9, 0.3}, {0.5, 0.8, 0.5}, {0.1, 0.4, 0.1},  {0.1, 0.9, 0.1},
                                    {0.2, 0.2, 0.2},  {0.2, 0.2, 0.2},
                                    {0.8, 0.8, 0.8},  {0.8, 0.8, 0.8},
                                    {0.6, 0.6, 0.6}, {0.6, 0.6, 0.6},
                                    {0.3, 0.3, 0.9},  {0.3, 0.3, 0.8} ,{0.1, 0.1, 0.6},  {0.1, 0.1, 0.9},
                                    {0.9, 0.3, 0.3},   {0.8, 0.5, 0.5}, {0.4, 0.1, 0.1},  {0.9, 0.1, 0.1} };
    mesh_list.push_back(testModel);

    //cube
    for (int h = -2; h < 0; h += 2) {
        for (int t = -20; t < 20; t += 2) {
            for (int w = -20; w < 20; w += 2) {
                Mesh cube_mesh;
                for (int i = 0; i < sizeof(cubeData.Cpoint) / sizeof(cubeData.Cpoint[0]); i += 1) {
                    Vec3& each_point = cube_mesh.vertices.emplace_back();
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



    //帧缓存设置
    Buffer Framebuffer_1;
    Framebuffer_1.SetBuffer(ptrScreen[0], ptrScreen[1]);

    //double camera test
    /*
    Buffer Framebuffer_2;
    Framebuffer_2.SetBuffer(ptrScreenTEST[0], ptrScreenTEST[1]);
    */

    while (true) {
        SDL_Event event{};
        SDL_PollEvent(&event);
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
            }
            else if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                ExitProcess(0);
                return;
            }
            break;

        }

        auto start = std::chrono::high_resolution_clock::now();   //测帧

        
        //Framebuffer_2.CleanBuffer();

        Camera ctmp = Camera_1;

        
#if 0
        // 计算总任务数量和每个线程处理的任务大小
        const int total_meshes = static_cast<int>(mesh_list.size());
        const int step = max(1, total_meshes / 4);

        const int num_tasks = (total_meshes + step - 1) / step; // 向上取整

        std::vector<std::future<Buffer>> bufferFutures;

        for (int i = 0; i < num_tasks; ++i) {
            const int start = i * step;
            const int end = min(start + step, total_meshes);
            const int count = end - start;

            // 使用 std::vector 的构造函数直接初始化，避免多次 push_back
            std::vector<Mesh> mesh_list_little(mesh_list.begin() + start, mesh_list.begin() + end);

             bufferFutures.emplace_back( pool.submit_task([&, mesh = std::move(mesh_list_little)]() {
                return Render_thread(ctmp, Framebuffer_1.Buffer_size, ptrScreen, mesh);
                }) 
             );
        }
        /*
        pool.submit_task([&]() {
            return Render_thread(ctmp, Framebuffer_1, ptrScreen, mesh_list);
            });
        */

        //pool.wait();

        Framebuffer_1.CleanBuffer();

        for (int d = 0; d < bufferFutures.size(); d++) {
            Buffer chunk = bufferFutures[d].get();
            Framebuffer_1.merge(chunk);
        }

#endif
        Framebuffer_1.CleanBuffer();
        Render(ctmp, Framebuffer_1, ptrScreen, mesh_list);

        //new
        //double camera test
        //Render(Camera_2, Framebuffer_2, ptrScreenTEST, mesh_list);
        


        auto tech_end = std::chrono::high_resolution_clock::now();


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
                //double D = 1- Framebuffer_1.GetDepth(x, y);
                
                //if (p.R == 0 && p.G == 0 && p.B == 0) continue;

                uint8_t r = static_cast<uint8_t>(p.R * 255.0);
                uint8_t g = static_cast<uint8_t>(p.G * 255.0);
                uint8_t b = static_cast<uint8_t>(p.B * 255.0);
                /*
                uint8_t r = static_cast<uint8_t>(std::clamp( p.R * 255.0 *D*100, 0.0, 255.0));
                uint8_t g = static_cast<uint8_t>(std::clamp(p.G * 255.0 * D * 100, 0.0, 255.0));
                uint8_t b = static_cast<uint8_t>(std::clamp(p.B * 255.0 * D * 100, 0.0, 255.0));
                */

                //test codes
                //test render depth picture
                /*
                r = static_cast<uint8_t>(D * 255.0 );
                g = static_cast<uint8_t>(D * 255.0);
                b = static_cast<uint8_t>(D * 255.0 );
                */

                uint32_t pixel = (255 << 24) | (r << 16) | (g << 8) | b;
                int index = y * (pitch / sizeof(uint32_t)) + x ;
                pixelData[index] = pixel;
            }
        }
        
        //new
        //double camera test
        //model's depth test
        /*
        
        for (int y = 0; y < ptrScreenTEST[1]; ++y) {
            for (int x = 0; x < ptrScreenTEST[0]; ++x) {
                Color p = Framebuffer_2.GetPixelColor(x, y);
                double D = Framebuffer_2.GetDepth(x, y);

                uint8_t r = static_cast<uint8_t>(D * 255.0);
                uint8_t g = static_cast<uint8_t>(D * 255.0);
                uint8_t b = static_cast<uint8_t>(D * 255.0);

                uint32_t pixel = (255 << 24) | (r << 16) | (g << 8) | b;
                int index = y * (pitch / sizeof(uint32_t)) + x + ptrScreen[0] - ptrScreenTEST[0];
                pixelData[index] = pixel;
            }
        }
        */

        SDL_UnlockTexture(texture);


        // 将纹理渲染到屏幕上
        
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        


        auto end = std::chrono::high_resolution_clock::now();    //测帧
        std::chrono::duration<double> elapsed_seconds = end - start; 

        std::chrono::duration<double> Theoretical__ = tech_end - start;

        std::string title_str = ENGINE_NAME;
        title_str.append("                      Compilation_Date:");
        title_str.append(__DATE__);

        std::string info = std::to_string( 1 / elapsed_seconds.count() ) + " FPS      " + std::to_string(1 / Theoretical__.count()) + " FPS(theoretical)";

        std::string pos = "Pos  " + std::to_string(Camera_1.Pos.x) + ", " + std::to_string(Camera_1.Pos.y) + ", " + std::to_string(Camera_1.Pos.z);
        const char* pos_char = pos.c_str();

        const char* Sign = "Press  ESC  to  pause  this  program  or  continue  play  it.";
        const char* Sign_2 = "You  can  close  this  window  when  it  is  paused.";
        const char* title = title_str.c_str();
        const char* Frames_char = info.c_str();


        EasyRenderText(renderer, font, title, textColor, 4, 4);
        EasyRenderText(renderer, font, Frames_char, textColor, 4, 20);
        EasyRenderText(renderer, font, pos_char, textColor, 4, 36);
        EasyRenderText(renderer, font, Sign, textColor, 4, 68);
        EasyRenderText(renderer, font, Sign_2, textColor, 4, 84);

        // 刷新渲染器
        SDL_RenderPresent(renderer);

        

        
    }

}


//camera控制线程
void control_thread()
{
    POINT currentPos, centerPos = { GetSystemMetrics(SM_CXSCREEN)/ 2, GetSystemMetrics(SM_CYSCREEN)/2 };

    double angleCurrent[] = { 0,0 };
    double Control[] = {0,0,0,0,0,0};
    double speed = 0.03;

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

        angleCurrent[0] = std::fmod(angleCurrent[0] + deltaX * 0.4, 360.0);
        if (angleCurrent[0] < 0) {
            angleCurrent[0] += 360.0;
        }
        angleCurrent[1] = std::clamp(angleCurrent[1] - deltaY * 0.4, -90.0, 90.0);
        //标准化数据



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
            Control[4] = speed;
        }else if (GetAsyncKeyState('D') & 0x8000){
            Control[4] = -speed;
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

        //camera_set(Camera_1,Control[0], Control[1], Control[2], Control[3], Control[4], Control[5]);
        Camera_1.Move_ForwardBack(Control[3]);
        Camera_1.Move_LeftRight(Control[4]);
        Camera_1.Move_UpDown(Control[5]);
        Camera_1.Set_Direction(Control[1], Control[2], Control[0]);
        //double camera test




    }
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance, LPSTR cmdstr, int code) {

    std::thread render(Display_thread);
    std::thread control(control_thread);

    render.join();
    control.join();

    ExitProcess(0);

}
