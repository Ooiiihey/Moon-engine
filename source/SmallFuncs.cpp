#include "Moon.h"

void Mesh::VectorMovePos(Vec3 pos) {
	for (Vec3& each : vertices) {
		each = pos + each;
	}

}

void Model_M::LinkMesh(Mesh& m)
{
	ptrOriginalMesh = &m;
	UpdateMesh();
}

void Model_M::linkMaterial(Material_M& mtl)
{
	ptrMaterial = &mtl;
}

void Model_M::linkTexture(Texture_M& tex)
{
	ptrTexture = &tex;
}

void Model_M::ZoomSize(double alpha)
{
	size = alpha;
}

void Model_M::SetMeshPos(Vec3 pos) {
	Positon = pos;
}

void Model_M::RotateMesh(Vec3 angle)
{
	X = rotate_all(Vec3(1, 0, 0), angle);
	Y = rotate_all(Vec3(0, 1, 0), angle);
	Z = rotate_all(Vec3(0, 0, 1), angle);
}

void Model_M::UpdateMesh()
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


void Texture_M::SetTexture(unsigned int width_in, unsigned int height_in){
	width = width_in;
	height = height_in;

}


RGBa Texture_M::getPixel(double u, double v) {
	// 将 u/v 限制在 [0,1) 范围内（避免 v=1.0 时的越界）
	u = std::clamp(u, 0.0, 0.9999999);
	v = std::clamp(v, 0.0, 0.9999999);
	unsigned int x = static_cast<unsigned int>(width * u);
	unsigned int y = static_cast<unsigned int>(height * v);
	return Pixels[y * width + x]; // 行优先访问
}


/*
void VertexBufferObject::put3DIndex(Vec3& in, unsigned int index){
	vertex2d[index].v3D = in;
}

Vec3 VertexBufferObject::get3D(unsigned int index){
	return vertex2d[index].v3D;
}
*/
