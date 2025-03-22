#include "Moon.h"


RGBa ParallelLight::Illuminate(
    RGBa inputColor,
    const Camera& Receive_camera,
    const Vec3& targetCam3D,
    const Vec3& norVec,
    const MoonMaterial* ptrMtl)
{
	RGBa finalColor;
	// ��ָ����
	if (!ptrMtl) {
		return RGBa{ 0.0f, 0.0f, 0.01f, 1.0f };
	}

	//ʹ��camera space�¹��յķ���
	Vec3 lightDir = { -dot(direction, Receive_camera.Forward_vec),
								-dot(direction, Receive_camera.Y_vec),
								-dot(direction, Receive_camera.Z_vec),
	};
	lightDir.normalize();


	/*

	// ������
	RGBa ambient = inputColor * ptrMtl->ambientColor;
*/

// ������
	double diffuseFactor = dot(norVec, lightDir);
	RGBa diffuse;

	if (diffuseFactor > 0) {
		diffuse = inputColor * Color * ptrMtl->diffuseColor;
		diffuse.R *= static_cast<float> (diffuseFactor);
		diffuse.G *= static_cast<float> (diffuseFactor);
		diffuse.B *= static_cast<float> (diffuseFactor);
	}

	// ����������߹� (Blinn-Phongģ��)
	RGBa specular;
	if (diffuseFactor > 0) {

		Vec3 viewDir = (Vec3(0, 0, 0) - targetCam3D).normalize();
		Vec3 halfway = (lightDir + viewDir).normalize();

		double specularFactor = dot(norVec, halfway);
		if (specularFactor > 0) {
			specularFactor = pow(specularFactor, ptrMtl->specularExponent); // �߹�ָ��
			specular = ptrMtl->specularColor * Color;
			specular.R *= static_cast<float> (specularFactor);
			specular.G *= static_cast<float> (specularFactor);
			specular.B *= static_cast<float> (specularFactor);
		}
	}

	// �ϲ����й���
	finalColor = diffuse + specular;


	finalColor.R = std::clamp(finalColor.R, 0.0f, 1.0f);
	finalColor.G = std::clamp(finalColor.G, 0.0f, 1.0f);
	finalColor.B = std::clamp(finalColor.B, 0.0f, 1.0f);
	finalColor.a = std::clamp(finalColor.a, 0.0f, 1.0f);

	return finalColor;
}





// Lights.cpp
RGBa PointLight::Illuminate(
    RGBa inputColor,
    const Camera& camera,
    const Vec3& targetCam3D,
    const Vec3& norCam,
    const MoonMaterial* ptrMtl
) {
    RGBa finalColor;
    if (!ptrMtl) {
        return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };
    }

    // ����Դλ��ת��������ռ�
// �����������ͼ��������ȷʵ���������� -> ��������ת��
    Vec3 tmp = Positon - camera.Pos;

    Vec3 lightPosCam = { dot(tmp, camera.Forward_vec),
                                    dot(tmp, camera.Y_vec),
                                    dot(tmp, camera.Z_vec),
    };
    if ((lightPosCam - targetCam3D).length() > Radius) return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };

    Vec3 WorldVertex =  camera.Forward_vec * targetCam3D.x +
                                       camera.Y_vec * targetCam3D.y +
                                       camera.Z_vec * targetCam3D.z + camera.Pos;

    if (ShadowCaster.isInShadow(WorldVertex)) {
        return RGBa{ 0.0f, 0.0f, 0.0f, 0.0f };
    }




    // �����Դ���򣨴Ӷ��㵽��Դ������ռ��£�
    Vec3 lightDirCam = lightPosCam - targetCam3D;
    double distance = lightDirCam.length();
    lightDirCam.normalize();

    // ����˥��
    double attenuation =1.0 / (Constant + Linear * distance + Quadratic * (distance * distance));

    // ���������
    double diffuseFactor = dot(norCam, lightDirCam);
    RGBa diffuse;
    if (diffuseFactor > 0) {
        diffuse = inputColor * Color * ptrMtl->diffuseColor;
        diffuse.R *= static_cast<float> (diffuseFactor * attenuation);
        diffuse.G *= static_cast<float> (diffuseFactor * attenuation);
        diffuse.B *= static_cast<float> (diffuseFactor * attenuation);
    }

    // �߹���㣨Blinn-Phong��
    RGBa specular;
    if (diffuseFactor > 0) {
        // ���߷��򣨴Ӷ���ָ�����ԭ�㣬����ռ������ԭ��Ϊ (0,0,0)��
        Vec3 viewDirCam = (Vec3(0, 0, 0) - targetCam3D);
        viewDirCam.normalize();

        // ����������
        Vec3 halfwayCam = (lightDirCam + viewDirCam).normalize();
        double specularFactor = dot(norCam, halfwayCam);

        if (specularFactor > 0) {
            specularFactor = pow(specularFactor, ptrMtl->specularExponent);
            specular = ptrMtl->specularColor * Color;
            specular.R *= static_cast<float> (specularFactor * attenuation);
            specular.G *= static_cast<float> (specularFactor * attenuation);
            specular.B *= static_cast<float> (specularFactor * attenuation);
        }
    }

    // �ϲ�����
    finalColor = diffuse + specular;

    // ������ɫ��Χ
    finalColor.R = std::clamp(finalColor.R, 0.0f, 1.0f);
    finalColor.G = std::clamp(finalColor.G, 0.0f, 1.0f);
    finalColor.B = std::clamp(finalColor.B, 0.0f, 1.0f);
    finalColor.a = std::clamp(finalColor.a, 0.0f, 1.0f);

    return finalColor;
}