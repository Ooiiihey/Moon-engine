#pragma once

#include "Moon.h"


//base class, dont use it directly
class ShadowCasterBase {
protected:
Camera VirtualCam;

public:

	int width, height = 256;

	virtual void SetPosition(Vec3 pos);
	virtual void SetCaster(int size, Vec3 pos, double f) {};
	
	virtual void CaculateShadow(const std::vector <Model_M>& Models_list) {};
	virtual bool isInShadow(const Vec3& WorldSpaceVertex);
};


class ShadowCaster_PointLight : public ShadowCasterBase{
protected:

	Vec3 directionList[6] = {
	//front, back
	Vec3(0, 0, 0),
	Vec3(PI, 0, 0),
	//left, right
	Vec3(PI / 2.0, 0, 0),
	Vec3(-PI / 2.0, 0, 0),
	//up, down
	Vec3(0, PI / 2.0, 0),
	Vec3(0, -PI / 2.0, 0),
	};

	DepthBuffer MapsList[6];

public:

	void SetCaster(int size, Vec3 pos, double f) override;
	void CaculateShadow(const std::vector <Model_M>& Models_list) override;
	bool isInShadow(const Vec3& WorldSpaceVertex) override;
};



class ShadowCaster_SpotLight : public ShadowCasterBase {
protected:

DepthBuffer Map;

public:

	void SetCaster(int size_in, Vec3 pos, Vec3 Dir, double far, double fovAngle);
	void SetVirtualCam(Vec3 Pos, Vec3 Dir, double fovAngle);
	void CaculateShadow(const std::vector <Model_M>& Models_list) override;
	bool isInShadow(const Vec3& WorldSpaceVertex) override;

};



/*
class ShadowCaster_ParalleLight : ShadowCaster {
protected:
	DepthBuffer Map;

public:

	void reSetMap(int size, Vec3 pos, int n, int f);
	void CaseShadow(const std::vector <MoonModel>& Models_list);
};

*/