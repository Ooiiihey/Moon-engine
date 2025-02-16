#include "moon.h"

void DataCollection::put3DIndex(Vec3& in, unsigned int index){
	vertex2d[index] = { 0, 0, in.x, in.y, in.z, 0, 0 };
}
Vec3 DataCollection::get3D(unsigned int index){

	return Vec3(vertex2d[index].x3d, vertex2d[index].y3d, vertex2d[index].z3d);
}

