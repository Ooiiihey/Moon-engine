#include "Moon.h"


void  BaseGraphics::RefreshGraphicContext(const Camera& Receive_camera, BufferCollection& buffer, splitValue& Buffer_spilit) {
	GC.Receive_camera = &Receive_camera;
	GC.buffer = &buffer;
	GC.Buffer_spilit = &Buffer_spilit;

	// 计算渲染范围
	GC.ChunkWidth_Begin = static_cast<int> (Buffer_spilit.widthBegin * buffer.Buffer_size[0]);
	GC.ChunkWidth_End = static_cast<int> (Buffer_spilit.widthEnd * buffer.Buffer_size[0]);
	GC.ChunkHeight_Begin = static_cast<int> (Buffer_spilit.heightBegin * buffer.Buffer_size[1]);
	GC.ChunkHeight_End = static_cast<int> (Buffer_spilit.heightEnd * buffer.Buffer_size[1]);
}




inline double BaseGraphics::To_unLineDepth(double depth) {
	return std::clamp(GC.Receive_camera->FarPlane * (depth - GC.Receive_camera->NearPlane) 
								/ 
								(depth * (GC.Receive_camera->FarPlane - GC.Receive_camera->NearPlane)), 0.0, 1.0);
}

inline void BaseGraphics::PreComputeTriangle(double coeffs_[30], double& reciprocal_area, const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2){

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
inline Vertex2D BaseGraphics::Interporate(double coeffs_[30], double& reciprocal_area, double a, double b) {
	// 计算 area0 和 area1 的变项
	Vertex2D back;
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

	return back;
}



//常规版本的三角绘制
void BaseGraphics::DrawFlatTopTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2) {

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
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Vertex2D out = Interporate(coeffs_, reciprocal_area, x + 0.5f, y + 0.5f);
			
			double d = To_unLineDepth(out.v3D.x);
			if (GC.buffer->CompareDepth_Smaller(x, y, d) && GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v).a != 0) {

				GC.buffer->PutPixelAll(x, y, d, GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v), out.v3D, out.norVector, GC.ptrMateral0);

			}


		}
	}
};

void BaseGraphics::DrawFlatBottomTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2) {
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
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Vertex2D out = Interporate(coeffs_, reciprocal_area, x + 0.5f, y + 0.5f);

			double d = To_unLineDepth(out.v3D.x);
			if (GC.buffer->CompareDepth_Smaller(x, y, d) && GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v).a != 0) {

				GC.buffer->PutPixelAll(x, y, d, GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v), out.v3D, out.norVector, GC.ptrMateral0);

			}


		}
	}
}


//常规的带纹理绘制三角形
//scan line function
void BaseGraphics::DrawTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, Texture_M* ptrTexture0, Material_M* ptrMtl1) {

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
		const double MapsList = 1.0 / (1 / pv0->v3D.x + (1/pv2->v3D.x - 1/pv0->v3D.x) * alphaSplit);//此处进行的是逆深度的插值
		const Vertex2D vi = {	pv0->x + (pv2->x - pv0->x) * alphaSplit,
											pv1->y,
											Vec3(MapsList,
													(pv0->v3D.y / pv0->v3D.x + (pv2->v3D.y / pv2->v3D.x - pv0->v3D.y / pv0->v3D.x) * alphaSplit) * MapsList,
													(pv0->v3D.z / pv0->v3D.x + (pv2->v3D.z / pv2->v3D.x - pv0->v3D.z  / pv0->v3D.x) * alphaSplit) * MapsList),
											Vec2(
											(pv0->UVCoords.u /pv0->v3D.x + (pv2->UVCoords.u /pv2->v3D.x - pv0->UVCoords.u  /pv0->v3D.x) * alphaSplit) * MapsList,
											(pv0->UVCoords.v /pv0->v3D.x + (pv2->UVCoords.v /pv2->v3D.x - pv0->UVCoords.v  /pv0->v3D.x) * alphaSplit) * MapsList),
											Vec3((pv0->norVector.x / pv0->v3D.x + (pv2->norVector.x / pv2->v3D.x - pv0->norVector.x / pv0->v3D.x) * alphaSplit) * MapsList,
													(pv0->norVector.y / pv0->v3D.x + (pv2->norVector.y / pv2->v3D.x - pv0->norVector.y / pv0->v3D.x) * alphaSplit) * MapsList,
													(pv0->norVector.z / pv0->v3D.x + (pv2->norVector.z / pv2->v3D.x - pv0->norVector.z / pv0->v3D.x) * alphaSplit) * MapsList)
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


//In low resolution of screen, this function's speed may be slower than line function
void BaseGraphics::OptimizedDrawTriangle(
	const Vertex2D& v0,
	const Vertex2D& v1,
	const Vertex2D& v2,
	Texture_M* texture,
	Material_M* material
) {

	const Vertex2D* pVertices[3] = { &v0, &v1, &v2 };
	const double cross = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
	if (cross < 0) std::swap(pVertices[1], pVertices[2]);

	// 使用像素中心坐标计算包围盒
	const double minX = std::min({ pVertices[0]->x, pVertices[1]->x, pVertices[2]->x }) - 0.5;
	const double maxX = std::max({ pVertices[0]->x, pVertices[1]->x, pVertices[2]->x }) + 0.5;
	const double minY = std::min({ pVertices[0]->y, pVertices[1]->y, pVertices[2]->y }) - 0.5;
	const double maxY = std::max({ pVertices[0]->y, pVertices[1]->y, pVertices[2]->y }) + 0.5;


	//提前退出
	if (maxX < GC.ChunkWidth_Begin || minX > GC.ChunkWidth_End || maxY < GC.ChunkHeight_Begin || minY > GC.ChunkHeight_End) {
		return;
	}

	// 屏幕裁剪（整数坐标）
	const int clampedMinX = std::clamp(static_cast<int>(std::floor(minX)), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
	const int clampedMaxX = std::clamp(static_cast<int>(std::ceil(maxX)), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
	const int clampedMinY = std::clamp(static_cast<int>(std::floor(minY)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);
	const int clampedMaxY = std::clamp(static_cast<int>(std::ceil(maxY)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);

	// 使用浮点坐标计算边缘方程（避免整数溢出）
	const double edge0_a = (pVertices[1]->y - pVertices[2]->y);
	const double edge0_b = (pVertices[2]->x - pVertices[1]->x);
	const double edge0_c = (pVertices[1]->x * pVertices[2]->y - pVertices[2]->x * pVertices[1]->y);

	const double edge1_a = (pVertices[2]->y - pVertices[0]->y);
	const double edge1_b = (pVertices[0]->x - pVertices[2]->x);
	const double edge1_c = (pVertices[2]->x * pVertices[0]->y - pVertices[0]->x * pVertices[2]->y);

	const double edge2_a = (pVertices[0]->y - pVertices[1]->y);
	const double edge2_b = (pVertices[1]->x - pVertices[0]->x);
	const double edge2_c = (pVertices[0]->x * pVertices[1]->y - pVertices[1]->x * pVertices[0]->y);

	// 预计算三角形参数
	double coeffs[30];
	double reciprocal_area;
	PreComputeTriangle(coeffs, reciprocal_area, *pVertices[0], *pVertices[1], *pVertices[2]);

	constexpr int BLOCK_SIZE = 6;
	for (int blockY = clampedMinY; blockY < clampedMaxY; blockY += BLOCK_SIZE) {
		const int currentBlockYEnd = std::min(blockY + BLOCK_SIZE, clampedMaxY);
		for (int blockX = clampedMinX; blockX < clampedMaxX; blockX += BLOCK_SIZE) {
			const int currentBlockXEnd = std::min(blockX + BLOCK_SIZE, clampedMaxX);

			for (int y = blockY; y < currentBlockYEnd; ++y) {
				const double pixelY = y + 0.5; // 像素中心坐标
				for (int x = blockX; x < currentBlockXEnd; ++x) {
					const double pixelX = x + 0.5;


					const double e0 = edge0_a * pixelX + edge0_b * pixelY + edge0_c;
					const double e1 = edge1_a * pixelX + edge1_b * pixelY + edge1_c;
					const double e2 = edge2_a * pixelX + edge2_b * pixelY + edge2_c;

					if (e0 >= -1e-6 && e1 >= -1e-6 && e2 >= -1e-6) {
						// 插值
						Vertex2D interp = Interporate(coeffs, reciprocal_area, pixelX, pixelY);

						// 深度测试
						const double depth = To_unLineDepth(interp.v3D.x);
						if (GC.buffer->CompareDepth_Smaller(x, y, depth)) {
							// 纹理采样
							const RGBa color = texture->getPixel(
								interp.UVCoords.u,
								interp.UVCoords.v
							);

							// Alpha测试
							if (color.a != 0.0f) {
								GC.buffer->PutPixelAll(
									x, y, depth, color,
									interp.v3D, interp.norVector, material
								);
							}
						}
					}
				}
			}
		}
	}
}


void BaseGraphics::DeferredRender_AmbientLight(AmbientLight& AL) {

	for (int y = GC.ChunkHeight_Begin; y < GC.ChunkHeight_End; ++y) {
		for (int x = GC.ChunkWidth_Begin; x < GC.ChunkWidth_End; ++x) {
			if (!GC.buffer->GetPtrMtl(x, y)) continue;
			RGBa origialPixel = GC.buffer->GetTextureColor(x, y);
			RGBa pixel = origialPixel * AL.Color * GC.buffer->GetPtrMtl(x, y)->ambientColor;

			GC.buffer->AddFrameColor(x, y, pixel);
		}

	}

}



void BaseGraphics::DeferredRender_ParallelLight(ParallelLight& PL) {

	for (int y = GC.ChunkHeight_Begin; y < GC.ChunkHeight_End; ++y) {
		for (int x = GC.ChunkWidth_Begin; x < GC.ChunkWidth_End; ++x) {
			if (!GC.buffer->GetPtrMtl(x, y)) continue;
			RGBa origialPixel = GC.buffer->GetTextureColor(x, y);
			RGBa pixel = PL.Illuminate( origialPixel, *GC.Receive_camera, GC.buffer->GetCam3Dvertex(x, y), GC.buffer->GetNorVec(x, y), GC.buffer->GetPtrMtl(x, y));
			GC.buffer->AddFrameColor(x, y, pixel);
		}

	}

}

void BaseGraphics::DeferredRender_PointLight(PointLight& PL) {
	for (int y = GC.ChunkHeight_Begin; y < GC.ChunkHeight_End; ++y) {
		for (int x = GC.ChunkWidth_Begin; x < GC.ChunkWidth_End; ++x) {
			if (!GC.buffer->GetPtrMtl(x, y)) continue;

			// 计算光照
			RGBa pixel = PL.Illuminate(
				GC.buffer->GetTextureColor(x, y),
				*GC.Receive_camera,
				GC.buffer->GetCam3Dvertex(x, y),
				GC.buffer->GetNorVec(x, y),
				GC.buffer->GetPtrMtl(x, y)
			);

			GC.buffer->AddFrameColor(x, y, pixel);
		}
	}
}


void BaseGraphics::DeferredRender_AllLight(LightsCollection& LC) {
	for (int y = GC.ChunkHeight_Begin; y < GC.ChunkHeight_End; ++y) {
		for (int x = GC.ChunkWidth_Begin; x < GC.ChunkWidth_End; ++x) {
			if (!GC.buffer->GetPtrMtl(x, y) or GC.buffer->GetPtrMtl(x, y)->Unlit_Rendering ) {
				GC.buffer->AddFrameColor(x, y, GC.buffer->GetTextureColor(x, y));
				continue;
			}
			PixelData originalPixel = GC.buffer->GetPixelAll(x, y);

			RGBa FinalColor;

			for (AmbientLight& each : LC.AmbientLights) {
				FinalColor += originalPixel.TextureColor * each.Color * originalPixel.ptrMaterial->ambientColor;
			}
			
			for (ParallelLight& each : LC.ParallelLights) {
				FinalColor += each.Illuminate(
					originalPixel.TextureColor,
					*GC.Receive_camera,
					originalPixel.Cam3DVertex,
					originalPixel.NorVec,
					originalPixel.ptrMaterial
				);
			}

			for (PointLight& each : LC.PointLights) {
				FinalColor += each.Illuminate(
					originalPixel.TextureColor,
					*GC.Receive_camera,
					originalPixel.Cam3DVertex,
					originalPixel.NorVec,
					originalPixel.ptrMaterial
				);
			}

			for (SpotLight& each : LC.SpotLights) {
				FinalColor += each.Illuminate(
					originalPixel.TextureColor,
					*GC.Receive_camera,
					originalPixel.Cam3DVertex,
					originalPixel.NorVec,
					originalPixel.ptrMaterial
				);
			}

			FinalColor = {
				std::clamp(FinalColor.R, 0.0f, 1.0f),
				std::clamp(FinalColor.G, 0.0f, 1.0f),
				std::clamp(FinalColor.B, 0.0f, 1.0f)
			};
			


			GC.buffer->AddFrameColor(x, y, FinalColor);

		}
	}

}




//以下是shadowmapping版本的


inline Vertex2D Graphics_forShadow::Interporate(double coeffs_[30], double& reciprocal_area, double a, double b) {
	// 计算 area0 和 area1 的变项
	Vertex2D back;
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

	//此处省法向量插值计算

	// 透视校正插值 UV
	back.UVCoords.u = (w0 * coeffs_[9] + w1 * coeffs_[11] + w2 * coeffs_[13]) * inv_total_w;
	back.UVCoords.v = (w0 * coeffs_[10] + w1 * coeffs_[12] + w2 * coeffs_[14]) * inv_total_w;

	return back;
}

//专门给shadowmapping用的
void  Graphics_forShadow::RefreshGraphicContext(const Camera& Receive_camera, DepthBuffer& buffer, splitValue Buffer_spilit) {
	GC.Receive_camera = &Receive_camera;
	GC.depthbuffer = &buffer;

	// 计算渲染范围
	GC.ChunkWidth_Begin = static_cast<int> (Buffer_spilit.widthBegin * buffer.Buffer_size[0]);
	GC.ChunkWidth_End = static_cast<int> (Buffer_spilit.widthEnd * buffer.Buffer_size[0]);
	GC.ChunkHeight_Begin = static_cast<int> (Buffer_spilit.heightBegin * buffer.Buffer_size[1]);
	GC.ChunkHeight_End = static_cast<int> (Buffer_spilit.heightEnd * buffer.Buffer_size[1]);
}




void Graphics_forShadow::DrawFlatTopTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2) {

	//斜率倒数
	double m0 = (v2.x - v0.x) / (v2.y - v0.y);
	double m1 = (v2.x - v1.x) / (v2.y - v1.y);

	//起始扫描线
	int yStart = std::clamp(static_cast<int>(ceil(v0.y - 0.5)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);
	int yEnd = std::clamp(static_cast<int>(ceil(v2.y - 0.5)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);

	if (yStart == yEnd) return;

	double coeffs_[30];
	double reciprocal_area = 0;

	PreComputeTriangle(coeffs_, reciprocal_area, v0, v1, v2);
	for (int y = yStart; y < yEnd; ++y) {
		const float px0 = static_cast <float>(m0 * (double(y) + 0.5f - v0.y) + v0.x);
		const float px1 = static_cast <float>(m1 * (double(y) + 0.5f - v1.y) + v1.x);

		//here do not use Buffer_size[0] - 1, this will cause black wire in the side of screen.
		int xStart = std::clamp((int)ceil(px0 - 0.5f), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
		int xEnd = std::clamp((int)ceil(px1 - 0.5f), GC.ChunkWidth_Begin, GC.ChunkWidth_End);

		if (xStart == xEnd) continue;

		for (int x = xStart; x < xEnd; ++x) {
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Vertex2D out = Interporate(coeffs_, reciprocal_area, x + 0.5f, y + 0.5f);
			double d = To_unLineDepth(out.v3D.x);
			if (GC.depthbuffer->CompareDepth_Smaller(x, y, d) && GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v).a != 0) {

				GC.depthbuffer->PutDepth(x, y, d);

			}


		}
	}
};

void Graphics_forShadow::DrawFlatBottomTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2) {
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

		for (int x = xStart; x < xEnd; ++x) {
			//此处坐标+0.5是为了匹配光栅化左顶规则
			Vertex2D out = Interporate(coeffs_, reciprocal_area, x + 0.5f, y + 0.5f);
			double d = To_unLineDepth(out.v3D.x);
			if (GC.depthbuffer->CompareDepth_Smaller(x, y, d) && GC.ptrTexture->getPixel(out.UVCoords.u, out.UVCoords.v).a != 0.0f) {

				GC.depthbuffer->PutDepth(x, y, d);

			}



		}
	}
}


//shadowmapping专属
void Graphics_forShadow::DrawTriangle(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, Texture_M* ptrTexture0, Material_M* ptrMtl1) {

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
		const double MapsList = 1.0 / (1 / pv0->v3D.x + (1 / pv2->v3D.x - 1 / pv0->v3D.x) * alphaSplit);//此处进行的是逆深度的插值
		const Vertex2D vi = { pv0->x + (pv2->x - pv0->x) * alphaSplit,
											pv1->y,
											Vec3(MapsList,
													(pv0->v3D.y / pv0->v3D.x + (pv2->v3D.y / pv2->v3D.x - pv0->v3D.y / pv0->v3D.x) * alphaSplit) * MapsList,
													(pv0->v3D.z / pv0->v3D.x + (pv2->v3D.z / pv2->v3D.x - pv0->v3D.z / pv0->v3D.x) * alphaSplit) * MapsList),
											Vec2(
											(pv0->UVCoords.u / pv0->v3D.x + (pv2->UVCoords.u / pv2->v3D.x - pv0->UVCoords.u / pv0->v3D.x) * alphaSplit) * MapsList,
											(pv0->UVCoords.v / pv0->v3D.x + (pv2->UVCoords.v / pv2->v3D.x - pv0->UVCoords.v / pv0->v3D.x) * alphaSplit) * MapsList),
											// shadow 不用法向量，所以省计算
											Vec3(0, 0, 0)
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

//In low resolution of map, this function's speed may be slower than line function
void Graphics_forShadow::OptimizedDrawTriangle(
	const Vertex2D& v0,
	const Vertex2D& v1,
	const Vertex2D& v2,
	Texture_M* texture,
	Material_M* material
) {

	const Vertex2D* pVertices[3] = { &v0, &v1, &v2 };
	const double cross = (v1.x - v0.x) * (v2.y - v0.y) - (v2.x - v0.x) * (v1.y - v0.y);
	if (cross < 0) std::swap(pVertices[1], pVertices[2]);

	// 使用像素中心坐标计算包围盒
	const double minX = std::min({ pVertices[0]->x, pVertices[1]->x, pVertices[2]->x }) - 0.5;
	const double maxX = std::max({ pVertices[0]->x, pVertices[1]->x, pVertices[2]->x }) + 0.5;
	const double minY = std::min({ pVertices[0]->y, pVertices[1]->y, pVertices[2]->y }) - 0.5;
	const double maxY = std::max({ pVertices[0]->y, pVertices[1]->y, pVertices[2]->y }) + 0.5;

	// 屏幕裁剪（整数坐标）
	const int clampedMinX = std::clamp(static_cast<int>(std::floor(minX)), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
	const int clampedMaxX = std::clamp(static_cast<int>(std::ceil(maxX)), GC.ChunkWidth_Begin, GC.ChunkWidth_End);
	const int clampedMinY = std::clamp(static_cast<int>(std::floor(minY)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);
	const int clampedMaxY = std::clamp(static_cast<int>(std::ceil(maxY)), GC.ChunkHeight_Begin, GC.ChunkHeight_End);

	// 使用浮点坐标计算边缘方程（避免整数溢出）
	const double edge0_a = (pVertices[1]->y - pVertices[2]->y);
	const double edge0_b = (pVertices[2]->x - pVertices[1]->x);
	const double edge0_c = (pVertices[1]->x * pVertices[2]->y - pVertices[2]->x * pVertices[1]->y);

	const double edge1_a = (pVertices[2]->y - pVertices[0]->y);
	const double edge1_b = (pVertices[0]->x - pVertices[2]->x);
	const double edge1_c = (pVertices[2]->x * pVertices[0]->y - pVertices[0]->x * pVertices[2]->y);

	const double edge2_a = (pVertices[0]->y - pVertices[1]->y);
	const double edge2_b = (pVertices[1]->x - pVertices[0]->x);
	const double edge2_c = (pVertices[0]->x * pVertices[1]->y - pVertices[1]->x * pVertices[0]->y);

	// 预计算三角形参数
	double coeffs[30];
	double reciprocal_area;
	PreComputeTriangle(coeffs, reciprocal_area, *pVertices[0], *pVertices[1], *pVertices[2]);

	constexpr int BLOCK_SIZE = 4;
	for (int blockY = clampedMinY; blockY < clampedMaxY; blockY += BLOCK_SIZE) {
		const int currentBlockYEnd = std::min(blockY + BLOCK_SIZE, clampedMaxY);
		for (int blockX = clampedMinX; blockX < clampedMaxX; blockX += BLOCK_SIZE) {
			const int currentBlockXEnd = std::min(blockX + BLOCK_SIZE, clampedMaxX);

			for (int y = blockY; y < currentBlockYEnd; ++y) {
				const double pixelY = y + 0.5; // 像素中心坐标
				for (int x = blockX; x < currentBlockXEnd; ++x) {
					const double pixelX = x + 0.5;


					const double e0 = edge0_a * pixelX + edge0_b * pixelY + edge0_c;
					const double e1 = edge1_a * pixelX + edge1_b * pixelY + edge1_c;
					const double e2 = edge2_a * pixelX + edge2_b * pixelY + edge2_c;

					if (e0 >= -1e-6 && e1 >= -1e-6 && e2 >= -1e-6) {
						// 透视校正插值
						Vertex2D interp = Interporate(coeffs, reciprocal_area, pixelX, pixelY);

						// 深度测试
						const double depth = To_unLineDepth(interp.v3D.x);
						if (GC.depthbuffer->CompareDepth_Smaller(x, y, depth)) {
							// 纹理采样
							const RGBa color = texture->getPixel(
								interp.UVCoords.u,
								interp.UVCoords.v
							);

							// Alpha测试
							if (color.a != 0.0f) {
								GC.depthbuffer->PutDepth(
									x, y, depth);
							}
						}
					}
				}
			}
		}
	}
}