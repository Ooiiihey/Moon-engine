#pragma once
#include "Data.h"
#define PI 3.14159265358979

//camera
class Camera {
public:
    double NearPlane = 0.04;
    double FarPlane = 4000;
    double FOV = 45.0 * PI / 180.0;
    double F = 1;    //f����
    Vec3 Pos = { 0, 0, 0 };

    // ��ʼ����������ΪĬ������ϵ
    const Vec3 Forward = { 1, 0, 0 },
        Y = { 0, 1, 0 },
        Z = { 0, 0, 1 },
        move = { 0, -1, 0 };  //�ƶ�������(�̶�ֵ)
    //��cameraΪ���յĳ�ʼ����������꣨���ɸ��ģ���

    Vec3 Forward_vec = { 1, 0, 0 },
        Y_vec = { 0, 1, 0 },
        Z_vec = { 0, 0, 1 },
        move_vec = { 0, -1, 0 };

    void Set_CameraPos(const double x, const double y, const double z);
    void Set_CameraPos(Vec3 pos_in);
    void Move_ForwardBack(const double step);
    void Move_LeftRight(const double step);
    void Move_UpDown(const double step);

    void Set_Direction(const double a, const double b, const double c);
    void Set_Direction(Vec3 direction);

};