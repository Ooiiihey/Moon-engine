#pragma once

#include "Moon.h"

class ShadowCasterBase {
protected:

Camera VirtualCam;

public:

	int width, height = 256;

	virtual void reSetMap(int size, Vec3 pos, int f) {};
	virtual void SetPosition(Vec3 pos);
	virtual void CaculateShadow(const std::vector <MoonModel>& Models_list) {};
	virtual bool isInShadow(const Vec3& WorldSpaceVertex);
};


class ShadowCaster_PointLight : public ShadowCasterBase{
protected:

	Vec3 directionList[6] = {
	Vec3(0, 0, 0),
	Vec3(PI, 0, 0),

	Vec3(PI / 2.0, 0, 0),
	Vec3(-PI / 2.0, 0, 0),

	Vec3(0, PI / 2.0, 0),
	Vec3(0, -PI / 2.0, 0),
	};

	std::vector<DepthBuffer> depthBuffersList;

public:

	void reSetMap(int size, Vec3 pos, int f) override;
	void CaculateShadow(const std::vector <MoonModel>& Models_list) override;
	bool isInShadow(const Vec3& WorldSpaceVertex) override;
};



/*
class ShadowCasterParalleLight : ShadowCaster {
protected:
	DepthBuffer forward;

public:

	void reSetMap(int size, Vec3 pos, int n, int f);
	void CaseShadow(const std::vector <MoonModel>& Models_list);
};

*/