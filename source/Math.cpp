#include "Data.h"

//卡马克之魂快速平方根
double KQsqrt(double number) {
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

    return 1.0 / y;
}


inline double dot(const Vec3& vec1, const Vec3& vec2) {
    return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
};

/*
double dot(const Vec3& vec1, const double vec2[3]) {
    return (vec1.x * vec2[0] + vec1.y * vec2[1] + vec1.z * vec2[2]);
};
*/
inline Vec3 cross(const Vec3& vec1, const Vec3& vec2) {
    return Vec3 ( (vec2.y * vec1.z - vec2.z * vec1.y), (vec2.z * vec1.x - vec2.x * vec1.z), (vec2.x * vec1.y - vec2.y * vec1.x) );
};




//new
//p 绕 k单位向量 旋转
Vec3 rotate(const Vec3& p, const Vec3& k , const double angle) {
    /*
    double len = GetLength(k);
    if (len <= 1e-8) {
        return p; 
    }
    //k_为单位向量
    Vec3 k_ = k / len;
    double dotValue = dot(p, k_);
    Vec3 j;
    j.x = p.x - dotValue * k_.x;
    j.y = p.y - dotValue * k_.y;
    j.z = p.z - dotValue * k_.z;

    return Vec3 ( k_ * dotValue + j * cos(-angle) + cross(j, k_) * sin(-angle) );
    */
    return Vec3(p * cos(angle) + cross(k, p) * sin(angle) + k * dot(k, p) * (1 - cos(angle)));
}



Vec3 rotate_all(const Vec3 &j, Vec3 angle) {

    Vec3 X = { 1, 0, 0 };
    Vec3 Y = { 0, 1, 0 };
    Vec3 Z = { 0, 0, 1 };
    Vec3 tmpj;

    tmpj = rotate(j, X, angle.x);
    tmpj = rotate(tmpj, Y, angle.y);
    tmpj = rotate(tmpj, Z, angle.z);

    return tmpj;

}

