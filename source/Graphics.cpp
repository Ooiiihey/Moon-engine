#include "moon.h"



inline double Graphics::To_unLineDepth(const Camera& Rec_camera, double depth) {
	return std::clamp(Rec_camera.FarPlane * (depth - Rec_camera.NearPlane) / (depth * (Rec_camera.FarPlane - Rec_camera.NearPlane)), 0.0, 1.0);
}


inline void Graphics::PreComputeTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2){

	// 计算三角形面积
	double fixed_term1 = v0.x * v1.y + v1.x * v2.y + v2.x * v0.y;
	double fixed_term2 = v2.x * v1.y + v1.x * v0.y + v0.x * v2.y;
	reciprocal_area = 1.0 / (fixed_term1 - fixed_term2);

	// 预先计算重心坐标系的系数（周期draw same triangle内固定）
	coeffs_[0] = v0.x; coeffs_[1] = v0.y;
	coeffs_[2] = v1.x; coeffs_[3] = v1.y;
	coeffs_[4] = v2.x; coeffs_[5] = v2.y;

	coeffs_[6] = v0.x3d; coeffs_[7] = v1.x3d;
	coeffs_[8] = v2.x3d; 

	coeffs_[9] = v0.u; coeffs_[10] = v0.v;
	coeffs_[11] = v1.u; coeffs_[12] = v1.v;
	coeffs_[13] = v2.u; coeffs_[14] = v2.v;
	
	coeffs_[15] = v0.y3d; coeffs_[16] = v1.y3d;
	coeffs_[17] = v2.y3d;

	coeffs_[18] = v0.z3d; coeffs_[19] = v1.z3d;
	coeffs_[20] = v2.z3d;
}

//优化版本
inline void Graphics::Interporate(double a, double b, Vertex2D& back) {
	// 计算 area0 和 area1 的变项
	double area0 = a * coeffs_[3] + coeffs_[2] * coeffs_[5] + coeffs_[4] * b
							- coeffs_[4] * coeffs_[3] - coeffs_[2] * b - a * coeffs_[5];
	double area1 = coeffs_[0] * b + a * coeffs_[5] + coeffs_[4] * coeffs_[1]
							- coeffs_[4] * b - a * coeffs_[1] - coeffs_[0] * coeffs_[5];

	// 使用预计算的 area
	double alpha = area0 * reciprocal_area;
	double beta = area1 * reciprocal_area;
	double gamma = 1.0 - alpha - beta;

	back.x = a; back.y = b;
	// 返回逆深度插值的结果
	back.x3d = 1.0 / (alpha / coeffs_[6] + beta / coeffs_[7] + gamma / coeffs_[8]);
	back.y3d = (alpha * coeffs_[15] / coeffs_[6] + beta * coeffs_[16] / coeffs_[7] + gamma * coeffs_[17] / coeffs_[8]) / back.x3d;
	back.z3d = (alpha * coeffs_[18] / coeffs_[6] + beta * coeffs_[19] / coeffs_[7] + gamma * coeffs_[20] / coeffs_[8]) / back.x3d;
	//uv插值
	back.u = (alpha * coeffs_[9] / coeffs_[6] + beta  * coeffs_[11] / coeffs_[7]+ gamma  * coeffs_[13] / coeffs_[8]) / back.x3d;
	back.v = (alpha *  coeffs_[10] / coeffs_[6] + beta * coeffs_[12] / coeffs_[7] + gamma  * coeffs_[14] / coeffs_[8]) / back.x3d;

}




void Graphics::DrawFlatTopTriangle(const Camera& Receive_camera, Buffer &buffer, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, const Color& c) {

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
			Vertex2D out ;
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Interporate(x + 0.5f, y + 0.5f, out);
			double d = To_unLineDepth(Receive_camera, out.x3d);
			if (buffer.CompareDepth_Smaller(x, y, d) ) {
				buffer.PutPixel(x, y, d, c);
			}

			

		}
	}
};

void Graphics::DrawFlatBottomTriangle(const Camera& Receive_camera, Buffer& buffer, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, const Color& c) {
	// 斜率倒数
	double m0 = (v1.x - v0.x) / (v1.y - v0.y);
	double m1 = (v2.x - v0.x) / (v2.y - v0.y);

	// 起始扫描线
	int yStart = std::clamp(static_cast<int>(ceil(v0.y - 0.5)), 0, static_cast<int>(buffer.Buffer_size[1]));
	int yEnd = std::clamp(static_cast<int>(ceil(v2.y - 0.5)), 0, static_cast<int>(buffer.Buffer_size[1]));

	if (yStart == yEnd) return;

	PreComputeTriangle(v0, v1, v2);
	for (int y = yStart; y < yEnd; ++y) {
		const double px0 = m0 * (static_cast<double>(y) + 0.5f - v1.y) + v1.x;
		const double px1 = m1 * (static_cast<double>(y) + 0.5f - v2.y) + v2.x;

		//here do not use Buffer_size[0] - 1, this will cause black wire in the side of screen.
		int xStart = std::clamp(static_cast<int>(ceil(px0 - 0.5f)) , 0, static_cast<int>(buffer.Buffer_size[0]));
		int xEnd = std::clamp(static_cast<int>(ceil(px1 - 0.5f)) , 0, static_cast<int>(buffer.Buffer_size[0]));

		if (xStart == xEnd) continue;

		for (int x = xStart ; x < xEnd; ++x) {
			Vertex2D out;
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Interporate(x + 0.5f, y + 0.5f, out);
			double d = To_unLineDepth(Receive_camera, out.x3d);
			if (buffer.CompareDepth_Smaller(x, y, d)) {
				buffer.PutPixel(x, y, d, c);
			}


		}
	}
}

void Graphics::DrawTriangle(const Camera& Receive_camera, Buffer& buffer, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, Color& c) {
	const Vertex2D* pv0 = &v0;
	const Vertex2D* pv1 = &v1;
	const Vertex2D* pv2 = &v2;

	//交换上下顺序
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);
	if (pv2->y < pv1->y)  std::swap(pv2, pv1);
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);

	//自然平顶三角形
	if ((ceil)(pv1->y -0.5f)== (ceil)(pv0->y - 0.5f)) {
		if (pv1->x < pv0->x) std::swap(pv0, pv1);
		DrawFlatTopTriangle(Receive_camera, buffer, *pv0, *pv1, *pv2, c);
	}
	else if ((ceil)(pv1->y -0.5f) == (ceil)(pv2->y -0.5f)) {
		if (pv1->x > pv2->x) std::swap(pv2, pv1);
		DrawFlatBottomTriangle(Receive_camera, buffer, *pv0, *pv1, *pv2, c);
	}
	else {
		const double alphaSplit = (pv1->y - pv0->y) / (pv2->y - pv0->y);
		const double depth = 1.0 / (1 / pv0->x3d + (1/pv2->x3d - 1/pv0->x3d) * alphaSplit);//此处进行的是逆深度的插值
		const Vertex2D vi = {	pv0->x + (pv2->x - pv0->x) * alphaSplit,
										pv1->y,
										depth,
										(pv0->y3d / pv0->x3d + (pv2->y3d / pv2->x3d - pv0->y3d / pv0->x3d) * alphaSplit) * depth,
										(pv0->z3d / pv0->x3d + (pv2->z3d / pv2->x3d - pv0->z3d  / pv0->x3d) * alphaSplit) * depth,
										(pv0->u /pv0->x3d + (pv2->u /pv2->x3d - pv0->u  /pv0->x3d) * alphaSplit) * depth,
										(pv0->v /pv0->x3d + (pv2->v /pv2->x3d - pv0->v  /pv0->x3d) * alphaSplit) * depth
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


