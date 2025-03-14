#include "Moon.h"


void  Graphics::Refresh_GraphicContext(const Camera& Receive_camera, MoonBuffer& buffer, splitValue& Buffer_spilit, const ParallelLight& PL0) {
	GC.Receive_camera = &Receive_camera;
	GC.buffer = &buffer;
	GC.Buffer_spilit = &Buffer_spilit;
	GC.PL = PL0;

	// 计算渲染范围
	GC.ChunkWidth_Begin = static_cast<int> (Buffer_spilit.widthBegin * buffer.Buffer_size[0]);
	GC.ChunkWidth_End = static_cast<int> (Buffer_spilit.widthEnd * buffer.Buffer_size[0]);
	GC.ChunkHeight_Begin = static_cast<int> (Buffer_spilit.heightBegin * buffer.Buffer_size[1]);
	GC.ChunkHeight_End = static_cast<int> (Buffer_spilit.heightEnd * buffer.Buffer_size[1]);
}

inline double Graphics::To_unLineDepth( double depth) {
	return std::clamp(GC.Receive_camera->FarPlane * (depth - GC.Receive_camera->NearPlane) 
								/ 
								(depth * (GC.Receive_camera->FarPlane - GC.Receive_camera->NearPlane)), 0.0, 1.0);
}

inline void Graphics::PreComputeTriangle(double coeffs_[30], double& reciprocal_area, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2){

	// 计算三角形面积
	double fixed_term1 = v0.x * v1.y + v1.x * v2.y + v2.x * v0.y;
	double fixed_term2 = v2.x * v1.y + v1.x * v0.y + v0.x * v2.y;
	reciprocal_area = 1.0 / (fixed_term1 - fixed_term2);

	// 预先计算重心坐标系的系数（周期draw same triangle内固定）
	coeffs_[0] = v0.x; coeffs_[1] = v0.y;
	coeffs_[2] = v1.x; coeffs_[3] = v1.y;
	coeffs_[4] = v2.x; coeffs_[5] = v2.y;

	coeffs_[6] = v0.v3D.x; coeffs_[7] = v1.v3D.x;
	coeffs_[8] = v2.v3D.x; 

	coeffs_[9] = v0.UVCoords.u; coeffs_[10] = v0.UVCoords.v;
	coeffs_[11] = v1.UVCoords.u; coeffs_[12] = v1.UVCoords.v;
	coeffs_[13] = v2.UVCoords.u; coeffs_[14] = v2.UVCoords.v;
	
	coeffs_[15] = v0.v3D.y; coeffs_[16] = v1.v3D.y;
	coeffs_[17] = v2.v3D.y;

	coeffs_[18] = v0.v3D.z; coeffs_[19] = v1.v3D.z;
	coeffs_[20] = v2.v3D.z;

	coeffs_[21] = v0.norVector.x; coeffs_[22] = v0.norVector.y;
	coeffs_[23] = v0.norVector.z;

	coeffs_[24] = v1.norVector.x; coeffs_[25] = v1.norVector.y;
	coeffs_[26] = v1.norVector.z;

	coeffs_[27] = v2.norVector.x; coeffs_[28] = v2.norVector.y;
	coeffs_[29] = v2.norVector.z;
}

//优化版本
inline void Graphics::Interporate(double coeffs_[30], double& reciprocal_area, double a, double b, Vertex2D& back) {
	// 计算 area0 和 area1 的变项
	double area0 = a * coeffs_[3] + coeffs_[2] * coeffs_[5] + coeffs_[4] * b - coeffs_[4] * coeffs_[3] - coeffs_[2] * b - a * coeffs_[5];
	double area1 = coeffs_[0] * b + a * coeffs_[5] + coeffs_[4] * coeffs_[1] - coeffs_[4] * b - a * coeffs_[1] - coeffs_[0] * coeffs_[5];

	// 使用预计算的 area
	double alpha = area0 * reciprocal_area;
	double beta = area1 * reciprocal_area;
	double gamma = 1.0 - alpha - beta;

	back.x = a; back.y = b;

	// 获取三个顶点的逆深度（ _3D.x 存储的是深度 ）
	double inv_z0 = 1.0 / coeffs_[6];
	double inv_z1 = 1.0 / coeffs_[7];
	double inv_z2 = 1.0 / coeffs_[8];

	// 计算透视校正权重
	double w0 = alpha * inv_z0;
	double w1 = beta * inv_z1;
	double w2 = gamma * inv_z2;
	double inv_total_w = 1.0 / (w0 + w1 + w2);

	// 插值逆深度
	back.v3D.x = 1.0 / (alpha * inv_z0 + beta * inv_z1 + gamma * inv_z2);
	back.v3D.y = (alpha * coeffs_[15] / coeffs_[6] + beta * coeffs_[16] / coeffs_[7] + gamma * coeffs_[17] / coeffs_[8]) * inv_total_w;
	back.v3D.z = (alpha * coeffs_[18] / coeffs_[6] + beta * coeffs_[19] / coeffs_[7] + gamma * coeffs_[20] / coeffs_[8]) * inv_total_w;

	// 透视校正插值法向量
	back.norVector.x = (w0 * coeffs_[21] + w1 * coeffs_[24] + w2 * coeffs_[27]) * inv_total_w;
	back.norVector.y = (w0 * coeffs_[22] + w1 * coeffs_[25] + w2 * coeffs_[28]) * inv_total_w;
	back.norVector.z = (w0 * coeffs_[23] + w1 * coeffs_[26] + w2 * coeffs_[29]) * inv_total_w;

	back.norVector.normalize();


	// 透视校正插值 UV
	back.UVCoords.u = (w0 * coeffs_[9] + w1 * coeffs_[11] + w2 * coeffs_[13]) * inv_total_w;
	back.UVCoords.v = (w0 * coeffs_[10] + w1 * coeffs_[12] + w2 * coeffs_[14]) * inv_total_w;
}


/* old codes
RGBa Graphics::Calculate_ParallelLight(RGBa inputColor, Vertex2D& target) {
	RGBa finalColor;

	Vec3 lightDir = GC.PL.direction;
	RGBa lightColor = GC.PL.LightColor;

	lightDir.normalize();

	Vec3 lightToVertex = { -lightDir.x, -lightDir.y, -lightDir.z };

	// 环境光
	RGBa ambient = inputColor * GC.ptrMateral0->ambientColor * lightColor;

	// 漫反射
	float diffuseFactor = (float)dot(target.norVector, lightToVertex);
	RGBa diffuse;

	if (diffuseFactor > 0) {
		diffuse = inputColor * GC.ptrMateral0->diffuseColor * lightColor;
		diffuse.R *= diffuseFactor;
		diffuse.G *= diffuseFactor;
		diffuse.B *= diffuseFactor;
	}

	// 计算漫反射高光 (Blinn-Phong模型)
	RGBa specular;
	if (diffuseFactor > 0) {
		Vec3 vertexWorldPos = GC.Receive_camera->Forward_vec * target.v3D.x
			+ GC.Receive_camera->Y_vec * target.v3D.y
			+ GC.Receive_camera->Z_vec * target.v3D.z
			+ GC.Receive_camera->Pos;

		Vec3 viewDir = (GC.Receive_camera->Pos - vertexWorldPos).normalize();
		Vec3 halfway = (lightToVertex + viewDir).normalize();

		float specularFactor = (float)dot(target.norVector, halfway);
		if (specularFactor > 0) {
			specularFactor = (float)pow(specularFactor, GC.ptrMateral0->specularExponent); // 高光指数
			specular = GC.ptrMateral0->specularColor * lightColor;
			specular.R *= specularFactor;
			specular.G *= specularFactor;
			specular.B *= specularFactor;
		}
	}

	// 合并所有光照
	finalColor = ambient + diffuse + specular;

	finalColor.R = std::clamp(finalColor.R, 0.0f, 1.0f);
	finalColor.G = std::clamp(finalColor.G, 0.0f, 1.0f);
	finalColor.B = std::clamp(finalColor.B, 0.0f, 1.0f);

	return finalColor;
}
*/

RGBa Graphics::CalculateLight(RGBa inputColor, const Vec3& targetCam3D, const Vec3& norVec, const MoonMaterial* ptrMtl) {
	RGBa finalColor;
	// 添加空指针检查
	if (!ptrMtl) {
		// 返回默认颜色或直接返回输入颜色
		
		return RGBa{0.0f, 0.0f, 0.01f, 1.0f};
	}

	//使用camera space下的方向
	Vec3 lightDir = { dot(GC.PL.direction, GC.Receive_camera->Forward_vec),
								dot(GC.PL.direction, GC.Receive_camera->Y_vec),
								dot(GC.PL.direction, GC.Receive_camera->Z_vec),
							};
	lightDir.normalize();

	RGBa lightColor = GC.PL.LightColor;

	Vec3 lightToVertex = { -lightDir.x, -lightDir.y, -lightDir.z };

	// 环境光
	RGBa ambient = inputColor * lightColor * ptrMtl->ambientColor;

	// 漫反射
	float diffuseFactor = (float)dot(norVec, lightToVertex);
	RGBa diffuse;

	if (diffuseFactor > 0) {
		diffuse = inputColor * lightColor * ptrMtl->diffuseColor;
		diffuse.R *= diffuseFactor;
		diffuse.G *= diffuseFactor;
		diffuse.B *= diffuseFactor;
	}

	// 计算漫反射高光 (Blinn-Phong模型)
	RGBa specular;
	if (diffuseFactor > 0) {

		Vec3 viewDir = (Vec3(0, 0, 0) - targetCam3D).normalize();
		Vec3 halfway = (lightToVertex + viewDir).normalize();

		float specularFactor = (float)dot(norVec, halfway);
		if (specularFactor > 0) {
			specularFactor = (float)pow(specularFactor, ptrMtl->specularExponent); // 高光指数
			specular = ptrMtl->specularColor * lightColor;
			specular.R *= specularFactor;
			specular.G *= specularFactor;
			specular.B *= specularFactor;
		}
	}

	// 合并所有光照
	finalColor = ambient + diffuse + specular;

	finalColor.R = std::clamp(finalColor.R, 0.0f, 1.0f);
	finalColor.G = std::clamp(finalColor.G, 0.0f, 1.0f);
	finalColor.B = std::clamp(finalColor.B, 0.0f, 1.0f);

	return finalColor;
}



void Graphics::DrawFlatTopTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2) {

	//斜率倒数
	double m0 = (v2.x - v0.x) / (v2.y - v0.y);
	double m1 = (v2.x - v1.x) / (v2.y - v1.y);

	//起始扫描线
	int yStart = std::clamp(static_cast<int>(ceil(v0.y - 0.5)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);
	int yEnd = std::clamp(static_cast<int>(ceil(v2.y - 0.5)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);

	if (yStart == yEnd) return;

	double coeffs_[30];
	double reciprocal_area = 0;

	PreComputeTriangle(coeffs_, reciprocal_area,  v0, v1, v2);
	for (int y = yStart; y < yEnd; ++y) {
		const float px0 = static_cast <float>(m0 * (double(y) + 0.5f - v0.y) + v0.x);
		const float px1 = static_cast <float>(m1 * (double(y) + 0.5f - v1.y) + v1.x);

		//here do not use Buffer_size[0] - 1, this will cause black wire in the side of screen.
		int xStart = std::clamp((int)ceil(px0 - 0.5f), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
		int xEnd = std::clamp((int)ceil(px1 - 0.5f), GC.ChunkWidth_Begin, GC.ChunkWidth_End);

		if (xStart == xEnd) continue;

		for (int x = xStart; x < xEnd; ++x) {
			Vertex2D out;
			//此处坐标+0.5是为了匹配光栅化左顶规则

			Interporate(coeffs_, reciprocal_area, x + 0.5f, y + 0.5f, out);
			double d = To_unLineDepth(out.v3D.x);
			if (GC.buffer->CompareDepth_Smaller(x, y, d)) {

				//RGBa Final_color = Calculate_ParallelLight(GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v), out);
				//Final_color = { (float)out.norVector.x , (float)out.norVector.y, (float)out.norVector.z, 1.0f };

				GC.buffer->PutPixelData(x, y, d, GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v), out.v3D, out.norVector, GC.ptrMateral0);
				//GC.buffer->PutPixelData(x, y, d, Final_color, out.v3D, out.norVector, GC.ptrMateral0);
			}

			

		}
	}
};

void Graphics::DrawFlatBottomTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2) {
	// 斜率倒数
	double m0 = (v1.x - v0.x) / (v1.y - v0.y);
	double m1 = (v2.x - v0.x) / (v2.y - v0.y);

	// 起始扫描线
	int yStart = std::clamp(static_cast<int>(ceil(v0.y - 0.5)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);
	int yEnd = std::clamp(static_cast<int>(ceil(v2.y - 0.5)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);

	if (yStart == yEnd) return;

	double coeffs_[30];
	double reciprocal_area = 0;
	PreComputeTriangle(coeffs_, reciprocal_area, v0, v1, v2);

	for (int y = yStart; y < yEnd; ++y) {
		const float px0 = static_cast <float>(m0 * (double(y) + 0.5f - v1.y) + v1.x);
		const float px1 = static_cast <float>(m1 * (double(y) + 0.5f - v2.y) + v2.x);

		//here do not use Buffer_size[0] - 1, this will cause black wire in the side of screen.
		int xStart = std::clamp((int)ceil(px0 - 0.5f), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
		int xEnd = std::clamp((int)ceil(px1 - 0.5f), GC.ChunkWidth_Begin, GC.ChunkWidth_End);

		if (xStart == xEnd) continue;

		for (int x = xStart ; x < xEnd; ++x) {
			Vertex2D out;
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Interporate(coeffs_, reciprocal_area, x + 0.5f, y + 0.5f, out);

			double d = To_unLineDepth(out.v3D.x);
			if (GC.buffer->CompareDepth_Smaller(x, y, d)) {

				//RGBa Final_color = Calculate_ParallelLight(GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v), out);
				//Final_color = { (float)out.norVector.x , (float)out.norVector.y, (float)out.norVector.z, 1.0f };

				GC.buffer->PutPixelData(x, y, d, GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v), out.v3D, out.norVector, GC.ptrMateral0);
				//GC.buffer->PutPixelData(x, y, d, Final_color, out.v3D, out.norVector, GC.ptrMateral0);
			}


		}
	}
}




void Graphics::DrawTriangleTex(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, MoonTexture* ptrTexture0, MoonMaterial* ptrMtl1) {

	const Vertex2D* pv0 = &v0;
	const Vertex2D* pv1 = &v1;
	const Vertex2D* pv2 = &v2;


	// 计算三角形的边界矩形
	double triMinX = std::min({ pv0->x, pv1->x, pv2->x });
	double triMaxX = std::max({ pv0->x, pv1->x, pv2->x });
	double triMinY = std::min({ pv0->y, pv1->y, pv2->y });
	double triMaxY = std::max({ pv0->y, pv1->y, pv2->y });

	

	// 检查三角形是否位于屏幕之外
	if (triMaxX < GC.ChunkWidth_Begin || triMinX > GC.ChunkWidth_End || triMaxY <	GC.ChunkHeight_Begin || triMinY > GC.ChunkHeight_End) {
		return;
	}


	GC.ptrTexture = ptrTexture0;
	GC.ptrMateral0 = ptrMtl1;

	

	//交换上下顺序
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);
	if (pv2->y < pv1->y)  std::swap(pv2, pv1);
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);

	
	//自然平顶三角形
	if (pv1->y == pv0->y) {
		if (pv1->x < pv0->x) std::swap(pv0, pv1);
		DrawFlatTopTriangle(*pv0, *pv1, *pv2);
	}
	else if (pv1->y == pv2->y) {
		if (pv1->x > pv2->x) std::swap(pv2, pv1);
		DrawFlatBottomTriangle(*pv0, *pv1, *pv2);
	}
	else {
		const double alphaSplit = (pv1->y - pv0->y) / (pv2->y - pv0->y);
		const double depth = 1.0 / (1 / pv0->v3D.x + (1/pv2->v3D.x - 1/pv0->v3D.x) * alphaSplit);//此处进行的是逆深度的插值
		const Vertex2D vi = {	pv0->x + (pv2->x - pv0->x) * alphaSplit,
											pv1->y,
											Vec3(depth,
													(pv0->v3D.y / pv0->v3D.x + (pv2->v3D.y / pv2->v3D.x - pv0->v3D.y / pv0->v3D.x) * alphaSplit) * depth,
													(pv0->v3D.z / pv0->v3D.x + (pv2->v3D.z / pv2->v3D.x - pv0->v3D.z  / pv0->v3D.x) * alphaSplit) * depth),
											Vec2(
											(pv0->UVCoords.u /pv0->v3D.x + (pv2->UVCoords.u /pv2->v3D.x - pv0->UVCoords.u  /pv0->v3D.x) * alphaSplit) * depth,
											(pv0->UVCoords.v /pv0->v3D.x + (pv2->UVCoords.v /pv2->v3D.x - pv0->UVCoords.v  /pv0->v3D.x) * alphaSplit) * depth),
											Vec3((pv0->norVector.x / pv0->v3D.x + (pv2->norVector.x / pv2->v3D.x - pv0->norVector.x / pv0->v3D.x) * alphaSplit) * depth,
													(pv0->norVector.y / pv0->v3D.x + (pv2->norVector.y / pv2->v3D.x - pv0->norVector.y / pv0->v3D.x) * alphaSplit) * depth,
													(pv0->norVector.z / pv0->v3D.x + (pv2->norVector.z / pv2->v3D.x - pv0->norVector.z / pv0->v3D.x) * alphaSplit) * depth)
												};

		if (pv1->x < vi.x) {
			//major right
			DrawFlatBottomTriangle(*pv0, *pv1, vi);
			DrawFlatTopTriangle(*pv1, vi, *pv2);
		}
		else {
			//major left
			DrawFlatBottomTriangle(*pv0, vi, *pv1);
			DrawFlatTopTriangle(vi, *pv1, *pv2);
		}
	}



}

void Graphics::Deferred_RenderLight() {

	for (int y = GC.ChunkHeight_Begin; y < GC.ChunkHeight_End; ++y) {
		for (int x = GC.ChunkWidth_Begin; x < GC.ChunkWidth_End; ++x) {
			if (!GC.buffer->GetPtrMtl(x, y)) continue;
			RGBa origialPixel = GC.buffer->GetPixelColor(x, y);
			RGBa pixel = CalculateLight(origialPixel, GC.buffer->GetCam3Dvertex(x, y), GC.buffer->GetNorVec(x, y), GC.buffer->GetPtrMtl(x, y));
			GC.buffer->PutPixelColor_only(x, y, pixel);
		}

	}

}


