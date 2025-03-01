#include "moon.h"

void Mesh::VectorMovePos(Vec3 pos) {
	for (Vec3& each : vertices) {
		each = pos + each;
	}

}

void Model::LinkMesh(Mesh& m)
{
	ptrOriginalMesh = &m;
	UpdateMesh();
}

void Model::LinkMaterial(Material& mtl)
{
	ptrMaterial = &mtl;
}

void Model::ZoomSize(double alpha)
{
	size = alpha;
}

void Model::SetMeshPos(Vec3 pos) {
	Positon = pos;
}

void Model::RotateMesh(Vec3 angle)
{
	X = rotate_all(Vec3(1, 0, 0), angle);
	Y = rotate_all(Vec3(0, 1, 0), angle);
	Z = rotate_all(Vec3(0, 0, 1), angle);
}

void Model::UpdateMesh()
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



void VertexBufferObject::put3DIndex(Vec3& in, unsigned int index){
	vertex2d[index]._3D = in;
}

Vec3 VertexBufferObject::get3D(unsigned int index){

	return vertex2d[index]._3D;
}

