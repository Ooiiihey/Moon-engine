#include "moon.h"



inline double Graphics::To_unLineDepth(const Camera_data& Rec_camera, double depth) {
	return std::clamp(Rec_camera.FarPlane * (depth - Rec_camera.NearPlane) / (depth * (Rec_camera.FarPlane - Rec_camera.NearPlane)), 0.0, 1.0);
}


//未优化的函数
inline double Graphics::LocateDepth(const Vec3& v0, const Vec3& v1, const Vec3& v2, double a, double b) {
	//三角形重心插值法
	double area = (v0.x * v1.y + v1.x * v2.y + v2.x * v0.y - v2.x * v1.y - v1.x * v0.y - v0.x * v2.y);
	double area0 = (a * v1.y + v1.x * v2.y + v2.x * b - v2.x * v1.y - v1.x * b - a * v2.y);
	double area1 = (v0.x * b + a * v2.y + v2.x * v0.y - v2.x * b - a * v0.y - v0.x * v2.y);

	double alpha = area0 / area, beta = area1 / area, gama = 1 - (alpha + beta);

	return 1 / (alpha * v0.z + beta * v1.z + gama * v2.z); //此处进行的是逆深度的插值！后面1/获得深度

}


//优化版本
inline void Graphics::PreComputeTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2){

	// 计算三角形面积
	double fixed_term1 = v0.x * v1.y + v1.x * v2.y + v2.x * v0.y;
	double fixed_term2 = v2.x * v1.y + v1.x * v0.y + v0.x * v2.y;
	area_triangle = fixed_term1 - fixed_term2;

	// 预先计算重心坐标系的系数（周期内固定）
	coeffs_[0] = v0.x; coeffs_[1] = v1.y;
	coeffs_[2] = v1.x; coeffs_[3] = v2.y;
	coeffs_[4] = v2.x; coeffs_[5] = v0.y;
	coeffs_[6] = v2.x; coeffs_[7] = v1.y;
	coeffs_[8] = v1.x; coeffs_[9] = v0.y;
	coeffs_[10] = v0.x; coeffs_[11] = v2.y;
	coeffs_[12] = v0.z; coeffs_[13] = v1.z;
	coeffs_[14] = v2.z;
}

//优化版本
inline double Graphics::LocateDepth_v2(double a, double b) {
	// 计算 area0 和 area1 的变项
	double area0 = a * coeffs_[1] + coeffs_[2] * coeffs_[3] + coeffs_[4] * b
		- coeffs_[4] * coeffs_[1] - coeffs_[2] * b - a * coeffs_[3];
	double area1 = coeffs_[0] * b + a * coeffs_[3] + coeffs_[4] * coeffs_[5]
		- coeffs_[4] * b - a * coeffs_[5] - coeffs_[0] * coeffs_[3];

	// 使用预计算的 area_triangle
	double alpha = area0 / area_triangle;
	double beta = area1 / area_triangle;
	double gamma = 1.0 - (alpha + beta);

	// 返回逆深度插值的结果
	return 1.0 / (alpha * coeffs_[12] + beta * coeffs_[13] + gamma * coeffs_[14]);
}




void Graphics::DrawFlatTopTriangle(const Camera_data& Receive_camera, Buffer &buffer, const Vec3& v0, const Vec3& v1, const Vec3& v2, const Color& c) {

	//斜率倒数
	double m0 = (v2.x - v0.x) / (v2.y - v0.y);
	double m1 = (v2.x - v1.x) / (v2.y - v1.y);

	//起始扫描线
	int yStart = std::clamp((int)ceil(v0.y - 0.5), 0, int(buffer.Buffer_size[1]));
	int yEnd = std::clamp((int)ceil(v2.y - 0.5), 0, int(buffer.Buffer_size[1]));

	if (yStart == yEnd) return;

	PreComputeTriangle(v0, v1, v2);
	for (int y = yStart; y < yEnd; ++y) {
		const double px0 = m0 * (double(y) + 0.5f - v0.y) + v0.x;
		const double px1 = m1 * (double(y) + 0.5f - v1.y) + v1.x;

		int xStart = std::clamp((int)ceil(px0 - 0.5f), 0, int(buffer.Buffer_size[0]));
		int xEnd = std::clamp((int)ceil(px1 - 0.5f), 0, int(buffer.Buffer_size[0]));

		if (xStart == xEnd) continue;

		for (int x = xStart; x < xEnd; ++x) {

			double d = To_unLineDepth(Receive_camera, LocateDepth_v2(x + 0.5f, y + 0.5f));
			//double d = To_unLineDepth(Receive_camera, LocateDepth(v0, v1, v2, x, y));
			if (buffer.CompareDepth_Smaller(x, y, d) ) {
				buffer.PutPixel(x, y, d, c);
			}

			

		}
	}
};

void Graphics::DrawFlatBottomTriangle(const Camera_data& Receive_camera, Buffer& buffer, const Vec3& v0, const Vec3& v1, const Vec3& v2, const Color& c) {
	// 斜率倒数
	double m0 = (v1.x - v0.x) / (v1.y - v0.y);
	double m1 = (v2.x - v0.x) / (v2.y - v0.y);

	// 起始扫描线
	int yStart = std::clamp(static_cast<int>(ceil(v0.y - 0.5)), 0, static_cast<int>(buffer.Buffer_size[1]));
	int yEnd = std::clamp(static_cast<int>(ceil(v2.y - 0.5)), 0, static_cast<int>(buffer.Buffer_size[1]));

	if (yStart == yEnd) return;

	PreComputeTriangle(v0, v1, v2);
	for (int y = yStart; y < yEnd; ++y) {
		// 计算当前行的左右 x 坐标
		const double px0 = m0 * (static_cast<double>(y) + 0.5f - v1.y) + v1.x;
		const double px1 = m1 * (static_cast<double>(y) + 0.5f - v2.y) + v2.x;

		// 确定当前行的 x 范围
		int xStart = std::clamp(static_cast<int>(ceil(px0 - 0.5f)) , 0, static_cast<int>(buffer.Buffer_size[0]));
		int xEnd = std::clamp(static_cast<int>(ceil(px1 - 0.5f)) , 0, static_cast<int>(buffer.Buffer_size[0]));

		if (xStart == xEnd) continue;

		for (int x = xStart ; x < xEnd; ++x) {
			// 计算当前像素的深度值
			double d = To_unLineDepth(Receive_camera, LocateDepth_v2(x + 0.5f, y + 0.5f));
			//double d = To_unLineDepth(Receive_camera, LocateDepth(v0, v1, v2, x, y));
			if (buffer.CompareDepth_Smaller(x, y, d)) {
				buffer.PutPixel(x, y, d, c);
			}


		}
	}
}

void Graphics::DrawTriangle(const Camera_data& Receive_camera, Buffer& buffer, const Vec3& v0, const Vec3& v1, const Vec3& v2, Color& c) {
	const Vec3* pv0 = &v0;
	const Vec3* pv1 = &v1;
	const Vec3* pv2 = &v2;

	//交换上下顺序
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);
	if (pv2->y < pv1->y)  std::swap(pv2, pv1);
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);

	//自然平顶三角形
	if (pv1->y == pv0->y) {
		if (pv1->x < pv0->x) std::swap(pv0, pv1);
		DrawFlatTopTriangle(Receive_camera, buffer, *pv0, *pv1, *pv2, c);
	}
	else if (pv1->y == pv2->y) {
		if (pv1->x > pv2->x) std::swap(pv2, pv1);
		DrawFlatBottomTriangle(Receive_camera, buffer, *pv0, *pv1, *pv2, c);
	}
	else {
		const double alphaSplit = (pv1->y - pv0->y) / (pv2->y - pv0->y);
		const Vec3 vi = {	pv0->x + (pv2->x - pv0->x) * alphaSplit,
										pv1->y,
										pv0->z + (pv2->z - pv0->z) * alphaSplit  //此处进行的是逆深度的插值！//得到的仍然是逆深度！！
										};

		if (pv1->x < vi.x) {
			//major right
			DrawFlatBottomTriangle(Receive_camera, buffer, *pv0, *pv1, vi, c);
			DrawFlatTopTriangle(Receive_camera, buffer, *pv1, vi, *pv2, c);
		}
		else {
			//major left
			DrawFlatBottomTriangle(Receive_camera, buffer, *pv0, vi, *pv1, c);
			DrawFlatTopTriangle(Receive_camera, buffer, vi, *pv1, *pv2, c);
		}
	}


};


