#pragma once
//#include "Data.h"
//#include "Shadow.h"
#include "Moon.h"


//under development
class Light {
public:
    RGBa Color = {0.5f, 0.5f, 0.5f};
    //double Intensity = 1.0;
};

class AmbientLight : public Light {
    
};

class ParallelLight : public Light {
public:
    Vec3 direction = { -0.2, -0.5, -0.3 };
    RGBa Illuminate(RGBa inputColor, const Camera& Receive_camera, const Vec3& targetCam3D, const Vec3& norVec, const MoonMaterial* ptrMtl);

};

class PointLight : public Light {
public:
    Vec3 Positon;
    double Constant = 1.0;    // 衰减常数项
    double Linear = 0.09;     // 线性衰减项
    double Quadratic = 0.032; // 二次衰减项
    double Radius = 10;
    ShadowCaster_PointLight ShadowCaster;
    RGBa Illuminate(RGBa inputColor, const Camera& Receive_camera, const Vec3& targetCam3D, const Vec3& norVec, const MoonMaterial* ptrMtl);

};

class LightsCollection {
public:
    std::vector <AmbientLight> AmbientLights;
    std::vector <ParallelLight> ParallelLights;
    std::vector <PointLight> PointLights;
};