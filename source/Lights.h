#pragma once

#include "Moon.h"


//under development
class Light {
public:
    RGBa Color = {0.5f, 0.5f, 0.5f};
    
};

class AmbientLight : public Light {
public:
    void SetLight(RGBa color_in);
};


class ParallelLight : public Light {
public:
    Vec3 Direction = { 0.1, 0.1, 0.1 };
    void SetLight(Vec3 Dir, RGBa color_in);
    RGBa Illuminate(RGBa inputColor, const Camera& Receive_camera, const Vec3& targetCam3D, const Vec3& norVec, const Material_M* ptrMtl);

};


class PointLight : public Light {
public:
    Vec3 Positon;
    double Constant = 1.0;    // 衰减常数项
    double Linear = 0.09;     // 线性衰减项
    double Quadratic = 0.032; // 二次衰减项
    double Radius = 10.0; //照明半径限制
    ShadowCaster_PointLight ShadowCaster;

    void SetLight(Vec3 Pos, RGBa color_in,
                            double Constant_in, double Linear_in,
                            double Quadratic_in, double Radius_in,
                            int Map_size);
    void reFresh_LightAndShadow(Vec3 Pos,const std::vector<Model_M>& Models_list);
    RGBa Illuminate(RGBa inputColor, const Camera& Receive_camera, const Vec3& targetCam3D, const Vec3& norVec, const Material_M* ptrMtl);

};



class SpotLight : public Light {
public:
    Vec3 Position;
    Vec3 Direction;

    double OuterAngle = 0.7854; // 45度，外锥角
    double InnerAngle = 0.5236; // 30度，内锥角
    
    double Constant = 1.0;
    double Linear = 0.09;
    double Quadratic = 0.032;
    double Radius = 10.0;
    ShadowCaster_SpotLight ShadowCaster;

    void SetLight(Vec3 Pos, Vec3 Dir, RGBa color_in,
                            double OuterAngle_in, double InnerAngle_in,
                            double Constant_in, double Linear_in, double Quadratic_in,
                            double Radius_in, int Map_size);
    void reFresh_LightAndShadow(Vec3 Pos, Vec3 Dir, double OuterAngle, double innerAngle, const std::vector<Model_M>& Models_list);
    RGBa Illuminate(RGBa inputColor, const Camera& Receive_camera,
                                const Vec3& targetCam3D, const Vec3& norVec,
                                const Material_M* ptrMtl);

};

class LightsCollection {
public:
    std::vector <AmbientLight> AmbientLights;
    std::vector <ParallelLight> ParallelLights;
    std::vector <PointLight> PointLights;
    std::vector <SpotLight> SpotLights;
};