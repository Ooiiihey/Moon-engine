#include "Moon.h"



void AmbientLight::SetLight(RGBa color_in)
{
    Color = color_in;
}

void ParallelLight::SetLight(Vec3 Dir, RGBa color_in)
{
    Color = color_in;
    Direction = Dir.normalize();
}

RGBa ParallelLight::Illuminate(
    RGBa inputColor,
    const Camera& Receive_camera,
    const Vec3& targetCam3D,
    const Vec3& norVec,
    const Material_M* ptrMtl)
{
	RGBa finalColor;
	// 空指针检查
	if (!ptrMtl) {
		return RGBa{ 0.0f, 1.0f, 0.0f, 1.0f };
	}

	//使用camera space下光照的方向
	Vec3 lightDir = { dot(Direction, Receive_camera.Forward_vec),
								dot(Direction, Receive_camera.Y_vec),
								dot(Direction, Receive_camera.Z_vec),
	};
	lightDir.normalize();


    // 漫反射
	float diffuseFactor = static_cast<float>(dot(norVec, lightDir));
	RGBa diffuse;

	if (diffuseFactor > 0) {
		diffuse = inputColor * Color * ptrMtl->diffuseColor;
		diffuse.R *= diffuseFactor;
		diffuse.G *= diffuseFactor;
		diffuse.B *= diffuseFactor;
	}

	// 计算漫反射高光 (Blinn-Phong模型)
	RGBa specular;
	if (diffuseFactor > 0) {

		Vec3 viewDir = (Vec3(0, 0, 0) - targetCam3D).normalize();
		Vec3 halfway = (lightDir + viewDir).normalize();

		float specularFactor = static_cast<float>(dot(norVec, halfway));
		if (specularFactor > 0) {
			specularFactor = powf(specularFactor, ptrMtl->specularExponent); // 高光指数
			specular = ptrMtl->specularColor * Color;
			specular.R *= specularFactor;
			specular.G *= specularFactor;
			specular.B *= specularFactor;
		}
	}

	// 合并所有光照
	finalColor = diffuse + specular;


	return finalColor;
}







void PointLight::SetLight(Vec3 Pos, RGBa color_in, double Constant_in, double Linear_in, double Quadratic_in, double Radius_in, int Map_size)
{
    Positon = Pos;
    Color = color_in;
    
    Constant = Constant_in;
    Linear = Linear_in;
    Quadratic = Quadratic_in;
    Radius = Radius_in;

    ShadowCaster.SetCaster(Map_size, Pos, Radius_in);

}

void PointLight::reFresh_LightAndShadow(Vec3 Pos, const std::vector<Model_M>& Models_list)
{
    Positon = Pos;
    ShadowCaster.SetPosition(Pos);
    ShadowCaster.CaculateShadow(Models_list);
}


RGBa PointLight::Illuminate(
    RGBa inputColor,
    const Camera& camera,
    const Vec3& targetCam3D,
    const Vec3& norCam,
    const Material_M* ptrMtl
) {
    RGBa finalColor;
    if (!ptrMtl) return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };

    Vec3 WorldVertex = camera.Forward_vec * targetCam3D.x +
                                      camera.Y_vec * targetCam3D.y +
                                      camera.Z_vec * targetCam3D.z + camera.Pos;

    if (ShadowCaster.isInShadow(WorldVertex)) {
        return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    // 将光源位置转换到相机空间
    Vec3 lightPosCam = { dot(Positon - camera.Pos, camera.Forward_vec),
                                    dot(Positon - camera.Pos, camera.Y_vec),
                                    dot(Positon - camera.Pos, camera.Z_vec),
    };

    // 超出半径范围限制处理
    if ((lightPosCam - targetCam3D).length() > Radius) return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };

    // 计算光源方向（从顶点到光源，相机空间下）
    Vec3 lightDirCam = lightPosCam - targetCam3D;
    float distance = (float)lightDirCam.length();
    lightDirCam.normalize();

    // 计算衰减
    float attenuation =(float)(1.0f / (Constant + Linear * distance + Quadratic * (distance * distance)));

    // 漫反射计算
    float diffuseFactor = (float)dot(norCam, lightDirCam);
    RGBa diffuse;
    if (diffuseFactor > 0) {
        diffuse = inputColor * Color * ptrMtl->diffuseColor;
        diffuse.R *= diffuseFactor * attenuation;
        diffuse.G *= diffuseFactor * attenuation;
        diffuse.B *= diffuseFactor * attenuation;
    }

    // 高光计算（Blinn-Phong）
    RGBa specular;
    if (diffuseFactor > 0) {
        // 视线方向（从顶点指向相机原点，相机空间中相机原点为 (0,0,0)）
        Vec3 viewDirCam = (Vec3(0, 0, 0) - targetCam3D);
        viewDirCam.normalize();

        // 计算半角向量
        Vec3 halfwayCam = (lightDirCam + viewDirCam).normalize();
        float specularFactor = (float)dot(norCam, halfwayCam);

        if (specularFactor > 0) {
            specularFactor = powf(specularFactor, ptrMtl->specularExponent);
            specular = ptrMtl->specularColor * Color;
            specular.R *= specularFactor * attenuation;
            specular.G *= specularFactor * attenuation;
            specular.B *= specularFactor * attenuation;
        }
    }

    // 合并光照
    finalColor = diffuse + specular;


    return finalColor;
}



void SpotLight::SetLight(Vec3 Pos, Vec3 Dir, RGBa color_in,
    double OuterAngle_in, double InnerAngle_in,
    double Constant_in, double Linear_in, double Quadratic_in,
    double Radius_in, int Map_size) {

    Position = Pos;
    Direction = Dir.normalize();
    Color = color_in;

    OuterAngle = OuterAngle_in;
    InnerAngle = InnerAngle_in;
    
    Constant = Constant_in;
    Linear = Linear_in;
    Quadratic = Quadratic_in;
    Radius = Radius_in;

    ShadowCaster.SetCaster(Map_size, Pos, Direction, Radius_in, OuterAngle_in);
}


void SpotLight::reFresh_LightAndShadow(Vec3 Pos, Vec3 Dir, double OuterAngle_in, double innerAngle_in, const std::vector<Model_M>& Models_list)
{
    Position = Pos;
    Direction = Dir;
    OuterAngle = OuterAngle_in;
    InnerAngle = innerAngle_in;

    ShadowCaster.SetVirtualCam(Pos, Dir, OuterAngle_in);
    ShadowCaster.CaculateShadow(Models_list);

}



RGBa SpotLight::Illuminate(
    RGBa inputColor,
    const Camera& camera,
    const Vec3& targetCam3D,
    const Vec3& norCam,
    const Material_M* ptrMtl
) {
    RGBa finalColor;
    if (!ptrMtl) {
        return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    Vec3 WorldVertex = camera.Forward_vec * targetCam3D.x +
        camera.Y_vec * targetCam3D.y +
        camera.Z_vec * targetCam3D.z + camera.Pos;

    if (ShadowCaster.isInShadow(WorldVertex)) {
        return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };
    }


    // 光源位置转换到相机空间
    Vec3 lightPosCam = {
        dot(Position - camera.Pos, camera.Forward_vec),
        dot(Position - camera.Pos, camera.Y_vec),
        dot(Position - camera.Pos, camera.Z_vec)
        };

    Vec3 lightDirCam = (lightPosCam - targetCam3D).normalize();


    // 计算距离衰减
    double distance = (lightPosCam - targetCam3D).length();
    if (distance > Radius) return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };

    float attenuation = (float)(1.0f / (Constant + Linear * distance + Quadratic * (distance * distance)));

    // 计算聚光灯方向
    Vec3 spotDirCam = {
        -dot(Direction, camera.Forward_vec),
        -dot(Direction, camera.Y_vec),
        -dot(Direction, camera.Z_vec)
    };
    spotDirCam.normalize();

    // 计算光线方向与聚光灯方向的夹角
    float cosTheta = (float)dot(lightDirCam, spotDirCam);
    float cosInner = cosf((float)InnerAngle);
    float cosOuter = cosf((float)OuterAngle);

    

    // 角度衰减计算
    float angleAttenuation = 0.0f;
    if (cosTheta > cosInner) {
        angleAttenuation = 1.0f;
    }
    else if (cosTheta > cosOuter) {
        // 平滑过渡
        angleAttenuation = (cosTheta - cosOuter) / (cosInner - cosOuter);
        angleAttenuation = angleAttenuation * angleAttenuation; // 二次曲线过渡
    }
    

    // 综合衰减
    attenuation *= angleAttenuation;
    if (attenuation <= 0.0f) return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };

    // 漫反射计算
    float diffuseFactor = static_cast<float>(dot(norCam, lightDirCam));
    RGBa diffuse;
    if (diffuseFactor > 0) {
        diffuse = inputColor * Color * ptrMtl->diffuseColor;
        diffuse.R *= diffuseFactor * attenuation;
        diffuse.G *= diffuseFactor * attenuation;
        diffuse.B *= diffuseFactor * attenuation;
    }

    // 高光计算（Blinn-Phong）
    RGBa specular;
    if (diffuseFactor > 0) {
        Vec3 viewDirCam = (Vec3(0, 0, 0) - targetCam3D).normalize();
        Vec3 halfway = (lightDirCam + viewDirCam).normalize();
        float specularFactor = static_cast<float>(dot(norCam, halfway));

        if (specularFactor > 0) {
            specularFactor = powf(specularFactor, ptrMtl->specularExponent);
            specular = ptrMtl->specularColor * Color;
            specular.R *= specularFactor * attenuation;
            specular.G *= specularFactor * attenuation;
            specular.B *= specularFactor * attenuation;
        }
    }

    finalColor = diffuse + specular;
    return finalColor;
}


