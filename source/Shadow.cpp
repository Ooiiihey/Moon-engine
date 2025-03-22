#include "Moon.h"

void ShadowCasterBase::SetPosition(Vec3 pos)
{
	VirtualCam.Set_CameraPos(pos);
}

bool ShadowCasterBase::isInShadow(const Vec3& WorldSpaceVertex)
{
	return false;
}



void ShadowCaster_PointLight::reSetMap(int size_in, Vec3 pos, int far)
{
	width = size_in;
	height = size_in;

	depthBuffersList.resize(6);
	for (DepthBuffer & each : depthBuffersList) {
		each.SetBuffer(width, height);
	}

	VirtualCam.Set_CameraPos(pos);
	VirtualCam.FarPlane = far;

	//fov多0.5是为了防止处理不到边缘
	VirtualCam.FOV = 45 * PI / 180.0;
	VirtualCam.F = 1.0;
}


void ShadowCaster_PointLight::CaculateShadow(const std::vector<MoonModel>& Models_list)
{
	BaseVertexShader TransformFunc;
	Graphics_forShadow GraphicsFunc;

	std::vector <Triangle> TrangleList;

	int screen[2] = { width, height };


	for (int i = 0; i < 6; ++i) {

		depthBuffersList[i].CleanBuffer();
		TrangleList.clear();

		VirtualCam.Set_Direction(directionList[i]);

		for (const MoonModel& each_Model : Models_list) {
			TransformFunc.Transform(VirtualCam, screen, each_Model, TrangleList);
		}

		GraphicsFunc.RefreshGraphicContext(VirtualCam, depthBuffersList[i], splitValue(0.0, 1.0, 0.0, 1.0));
		for (Triangle& each : TrangleList) {
			GraphicsFunc.DrawTriangleTexture(each.v0, each.v1, each.v2, each.ptrTexture, each.ptrMtl);
		}

	}
	

}

bool ShadowCaster_PointLight::isInShadow(const Vec3& WorldSpaceVertex)
{
	BaseVertexShader TransformFunc;
	int screen[2] = { width, height };
	Camera tmp = VirtualCam;

	for (int i = 0; i < 6; ++i) {

		tmp.Set_Direction(directionList[i]);
		Vertex2D point =TransformFunc.PerspectiveOneVertex(tmp, screen, WorldSpaceVertex);


		if (point.v3D.x < VirtualCam.NearPlane or (point.x > width || point.x < 0 || point.y > height || point.y < 0)) continue;

		if (!depthBuffersList[i].CompareDepth_Smaller((int)point.x, (int)point.y, point.v3D.x - 0.15)) {
			return true;
		}

		
	}

	return false;
}


