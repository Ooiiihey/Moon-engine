#include "Moon.h"

void Mesh::VectorMovePos(Vec3 pos) {
	for (Vec3& each : vertices) {
		each = pos + each;
	}

}

void MoonModel::LinkMesh(Mesh& m)
{
	ptrOriginalMesh = &m;
	UpdateMesh();
}

void MoonModel::linkMaterial(MoonMaterial& mtl)
{
	ptrMaterial = &mtl;
}

void MoonModel::linkTexture(MoonTexture& tex)
{
	ptrTexture = &tex;
}

void MoonModel::ZoomSize(double alpha)
{
	size = alpha;
}

void MoonModel::SetMeshPos(Vec3 pos) {
	Positon = pos;
}

void MoonModel::RotateMesh(Vec3 angle)
{
	X = rotate_all(Vec3(1, 0, 0), angle);
	Y = rotate_all(Vec3(0, 1, 0), angle);
	Z = rotate_all(Vec3(0, 0, 1), angle);
}

void MoonModel::UpdateMesh()
{
	if (ptrOriginalMesh) {
		WorldSpaceMesh = *ptrOriginalMesh;
		for (Vec3& each : WorldSpaceMesh.vertices) {
			each = each * size;
			each = Vec3( dot(each, X), dot(each, Y), dot(each, Z) );
			each = Positon + each;
		}

	}
	else return;


}


void MoonTexture::SetTexture(unsigned int width_in, unsigned int height_in){
	width = width_in;
	height = height_in;
	Pixels.reserve(width * height);
}


RGBa MoonTexture::getPixel(double u, double v) {
	// 将 u/v 限制在 [0,1) 范围内（避免 v=1.0 时的越界）
	u = std::clamp(u, 0.0, 0.999999);
	v = std::clamp(v, 0.0, 0.999999);
	unsigned int x = static_cast<unsigned int>(width * u);
	unsigned int y = static_cast<unsigned int>(height * v);
	return Pixels[y * width + x]; // 行优先访问
}



void VertexBufferObject::put3DIndex(Vec3& in, unsigned int index){
	vertex2d[index].v3D = in;
}

Vec3 VertexBufferObject::get3D(unsigned int index){
	return vertex2d[index].v3D;
}

