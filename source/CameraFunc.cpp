#include "Moon.h"


void Camera::Set_CameraPos(double x, double y, double z) {
	Pos.x = x;
	Pos.y = y;
	Pos.z = z;
};

void Camera::Set_CameraPos(Vec3 pos_in) {
    Pos = pos_in;
};


void Camera::Move_ForwardBack(const double step) {
	if (step == NULL) return;
	double a_front = 1 - step;
	Pos = Pos - Forward_vec * a_front + Forward_vec;

};


void Camera::Move_LeftRight(const double step) {
	if (step == NULL) return;
    Vec3 horizon_vec = { Pos.x - move_vec.x, Pos.y - move_vec.y, 0 };
    double a_horizon = 1 + step;

    Pos.x = horizon_vec.x * a_horizon + move_vec.x;
    Pos.y = horizon_vec.y * a_horizon + move_vec.y;
    move_vec.x = move_vec.x + Pos.x;
    move_vec.y = move_vec.y + Pos.y;
};


void Camera::Move_UpDown(const double step) {
	if (step == NULL) return;
	Pos.z += step;
};


//											    		横向				纵向		    	倾斜					
void Camera::Set_AngleDirection(double yaw, double pitch, double roll) {

    Vec3 new_forward = rotate(Forward, Z, yaw);
    Vec3 new_left = rotate(Y, Z, yaw);

    move_vec.y = move.y * cos(-yaw) + move.x * sin(-yaw);
    move_vec.x = move.x * cos(-yaw) - move.y * sin(-yaw);
    move_vec.x = move_vec.x + Pos.x;
    move_vec.y = move_vec.y + Pos.y;

    new_forward = rotate(new_forward, new_left, pitch);
    Vec3 new_up = rotate(Z, new_left, pitch);


    new_left = rotate(new_left, new_forward, -roll);
    new_up = rotate(new_up, new_forward, -roll);


    //updata
    Forward_vec = new_forward;
    Y_vec = new_left;
    Z_vec = new_up;
}

void Camera::Set_VectorDirection(Vec3 new_forward, const double c) {
    new_forward.normalize();
    if (new_forward == Forward_vec) return;

    // 计算旋转轴（输入方向向量与当前Forward向量的叉乘）
    Vec3 rotation_axis = cross(Forward, new_forward).normalize();
    double rotation_angle = acos(dot(Forward, new_forward));

    Y_vec = rotate(Y, rotation_axis, rotation_angle);
    Z_vec = rotate(Z, rotation_axis, rotation_angle);
    Forward_vec = new_forward;

    Y_vec = rotate(Y_vec, Forward_vec, c);
    Z_vec = rotate(Z_vec, Forward_vec, c);
};

void Camera::Set_AngleDirection(Vec3 Direction) {

    Vec3 new_forward = rotate(Forward, Z, Direction.x);
    Vec3 new_left = rotate(Y, Z, Direction.x);

    move_vec.y = move.y * cos(-Direction.x) + move.x * sin(-Direction.x);
    move_vec.x = move.x * cos(-Direction.x) - move.y * sin(-Direction.x);
    move_vec.x = move_vec.x + Pos.x;
    move_vec.y = move_vec.y + Pos.y;

    new_forward = rotate(new_forward, new_left, Direction.y);
    Vec3 new_up = rotate(Z, new_left, Direction.y);


    new_left = rotate(new_left, new_forward, -Direction.z);
    new_up = rotate(new_up, new_forward, -Direction.z);


    //updata
    Forward_vec = new_forward;
    Y_vec = new_left;
    Z_vec = new_up;
}