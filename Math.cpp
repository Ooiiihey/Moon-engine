#include "moon.h"

#if 0
#include <emmintrin.h> // 包含SSE2指令集头文件
//使用SSE2指令集的快速平方根
double SSE2Qsqrt(double number) {
    __m128d vec;
    vec = _mm_load_sd(&number); // 加载double值到SSE寄存器
    vec = _mm_sqrt_pd(vec);   // 计算平方根
    _mm_store_sd(&number, vec); // 将结果存储回double变量
    return number;
}
#endif

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

    return 1 / y;
}



double vectorMath::GetLength(const vertix& vec) {
    return KQsqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
};
double vectorMath::GetLength(const double vec[3]) {
    return KQsqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
};
double vectorMath::dot(const vertix& vec1, const vertix& vec2) {
    return (vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z);
};
void vectorMath::cross(const vertix& vec1, const vertix& vec2, vertix &vec_cross) {
    vec_cross = { (vec2.y * vec1.z - vec2.z * vec1.y), (vec2.z * vec1.x - vec2.x * vec1.z), (vec2.x * vec1.y - vec2.y * vec1.x) };
};