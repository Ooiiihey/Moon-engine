#include "Moon.h"

BS::thread_pool Shadow_pool(6);

//default
// dont use them
void ShadowCasterBase::SetPosition(Vec3 pos)
{
	VirtualCam.Set_CameraPos(pos);
}
bool ShadowCasterBase::isInShadow(const Vec3& WorldSpaceVertex)
{
	return false;
}






// spot lights' codes
void ShadowCaster_SpotLight::SetCaster(int size_in, Vec3 pos, Vec3 Dir,  double far, double fovAngle)
{
	width = size_in;
	height = size_in;

	Map.SetBuffer(size_in, size_in);

	VirtualCam.Set_CameraPos(pos);

	VirtualCam.Set_VectorDirection(Dir, 0);
	VirtualCam.FarPlane = far;
	VirtualCam.NearPlane = 0.01;

	VirtualCam.FOV = fovAngle;
	VirtualCam.F = 1.0;

}

void ShadowCaster_SpotLight::SetVirtualCam(Vec3 Pos, Vec3 Dir, double fovAngle)
{
	VirtualCam.Set_CameraPos(Pos);
	VirtualCam.Set_VectorDirection(Dir, 0);
	VirtualCam.FOV = fovAngle;
}


void ShadowCaster_SpotLight::CaculateShadow(const std::vector<Model_M>& Models_list)
{
	int screen[2] = { width, height };

	BaseVertexShader TransformFunc;
	Graphics_forShadow GraphicsFunc;

	std::vector <Triangle> TrangleList;
	Map.CleanBuffer();


	for (const Model_M& each_Model : Models_list) {
		TransformFunc.Transform(VirtualCam, screen, each_Model, TrangleList, true);
	}

	std::sort(TrangleList.begin(), TrangleList.end(), [](const Triangle& a, const Triangle& b) {

		const double sum_a = a.v0.v3D.x + a.v1.v3D.x + a.v2.v3D.x;
		const double sum_b = b.v0.v3D.x + b.v1.v3D.x + b.v2.v3D.x;

		return sum_a < sum_b;
	});

	GraphicsFunc.RefreshGraphicContext(VirtualCam, Map, splitValue(0.0, 1.0, 0.0, 1.0));
	for (Triangle& each : TrangleList) {
		GraphicsFunc.DrawTriangle(each.v0, each.v1, each.v2, each.ptrTexture, each.ptrMtl);
	}

}


bool ShadowCaster_SpotLight::isInShadow(const Vec3& WorldSpaceVertex)
{
	BaseVertexShader TransformFunc;
	int screen[2] = { width, height };

	Vertex2D TestVertex = TransformFunc.PerspectiveOneVertex(VirtualCam, screen, WorldSpaceVertex);
	
	if (TestVertex.v3D.x < VirtualCam.NearPlane
		or TestVertex.x > width
		or TestVertex.x < 0.0
		or TestVertex.y > height
		or TestVertex.y < 0.0) return false;
	

	double depth = std::clamp(VirtualCam.FarPlane * (TestVertex.v3D.x - VirtualCam.NearPlane) / (TestVertex.v3D.x * (VirtualCam.FarPlane - VirtualCam.NearPlane)), 0.0, 1.0);

	return not Map.CompareDepth_Smaller((int)TestVertex.x, (int)TestVertex.y, depth);

}




// point lights' codes
void ShadowCaster_PointLight::SetCaster(int size_in, Vec3 pos, double far)
{
	width = size_in;
	height = size_in;

	for (int i = 0; i < 6; ++i) {
		MapsList[i].SetBuffer(size_in, size_in);
	}

	VirtualCam.Set_CameraPos(pos);
	VirtualCam.FarPlane = far;
	VirtualCam.NearPlane = 0.1;

	VirtualCam.FOV = 45 * PI / 180.0;
	VirtualCam.F = 1.0;
}


void ShadowCaster_PointLight::CaculateShadow(const std::vector<Model_M>& Models_list)
{
	int screen[2] = { width, height };

	
	for (int i = 0; i < 6; ++i) {

		Shadow_pool.submit_task([&, i]() {
			BaseVertexShader TransformFunc;
			Graphics_forShadow GraphicsFunc;

			std::vector <Triangle> TrangleList;

			MapsList[i].CleanBuffer();

			Camera tmp = VirtualCam;
			tmp.Set_VectorDirection(directionsVecList[i], 0);

			for (const Model_M& each_Model : Models_list) {
				TransformFunc.Transform(tmp, screen, each_Model, TrangleList, true);
			}

			std::sort(TrangleList.begin(), TrangleList.end(), [](const Triangle& a, const Triangle& b) {
				// 分别计算两个三角形的三顶点深度之和
				const double sum_a = a.v0.v3D.x + a.v1.v3D.x + a.v2.v3D.x;
				const double sum_b = b.v0.v3D.x + b.v1.v3D.x + b.v2.v3D.x;

				return sum_a < sum_b;
				});

			GraphicsFunc.RefreshGraphicContext(tmp, MapsList[i], splitValue(0.0, 1.0, 0.0, 1.0));
			for (Triangle& each : TrangleList) {
				GraphicsFunc.DrawTriangle(each.v0, each.v1, each.v2, each.ptrTexture, each.ptrMtl);
			}

			});

	}
	Shadow_pool.wait();
	
	
	/*
	BaseVertexShader TransformFunc;
	Graphics_forShadow GraphicsFunc;

	std::vector <Triangle> TrangleList;

	Camera tmp = VirtualCam;


	for (int i = 0; i < 6; ++i) {

		MapsList[i].CleanBuffer();
		TrangleList.clear();

		tmp.Set_AngleDirection(directionList[i]);

		for (const Model_M& each_Model : Models_list) {
			TransformFunc.Transform(tmp, screen, each_Model, TrangleList, true);
		}

		GraphicsFunc.RefreshGraphicContext(tmp, MapsList[i], splitValue(0.0, 1.0, 0.0, 1.0));
		for (Triangle& each : TrangleList) {
			GraphicsFunc.DrawTriangle(each.v0, each.v1, each.v2, each.ptrTexture, each.ptrMtl);
		}

	}
	*/

}

bool ShadowCaster_PointLight::isInShadow(const Vec3& WorldSpaceVertex)
{
	BaseVertexShader TransformFunc;
	int screen[2] = { width, height };
	Camera tmp = VirtualCam;

	for (int i = 0; i < 6; ++i) {

		tmp.Set_VectorDirection(directionsVecList[i], 0);

		Vertex2D TestVertex =TransformFunc.PerspectiveOneVertex(tmp, screen, WorldSpaceVertex);


		if (TestVertex.v3D.x < VirtualCam.NearPlane
			or TestVertex.x > width
			or TestVertex.x < 0.0
			or TestVertex.y > height
			or TestVertex.y < 0.0) continue;


		double depth = std::clamp(tmp.FarPlane * (TestVertex.v3D.x - tmp.NearPlane) / (TestVertex.v3D.x * (tmp.FarPlane - tmp.NearPlane)), 0.0, 1.0);

		return not MapsList[i].CompareDepth_Smaller((int)TestVertex.x, (int)TestVertex.y, depth);

	}

	// this will remind me of the difficult I met before
	// I won't delet this
	//return false;
}


