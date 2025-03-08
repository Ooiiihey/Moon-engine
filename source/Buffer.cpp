#include "Moon.h"

void MoonBuffer::SetBuffer(int width, int height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	PixelColor.clear();
	Depth.clear();
	PixelCam3DVertex.clear();
	PixelNorVector.clear();
	PixelptrMaterial.clear();


	PixelColor.resize(size, RGBa{ 0.0f, 0.0f, 0.0f, 1.0f });
	Depth.resize(size, 1.0);
	PixelCam3DVertex.resize(size, Vec3(0, 0, 0));
	PixelNorVector.resize(size, Vec3(0, 0, 0));
	PixelptrMaterial.resize(size, nullptr);

}

void MoonBuffer::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	PixelColor.assign(total, RGBa{ 0.0f, 0.0f, 0.0f, 1.0f });
	Depth.assign(total, 1.0);  //初始DepthBuffer
	PixelCam3DVertex.assign(total, Vec3(0, 0, 0));
	PixelNorVector.assign(total, Vec3(0, 0, 0));
	PixelptrMaterial.assign(total, nullptr);
};



void MoonBuffer::PutPixelData(const int x, const int y, double depth, RGBa c, Vec3& Cam3Dvertex, Vec3& NorVec, MoonMaterial* ptrmtl) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1] ) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;
		PixelColor[total] = c;
		Depth[total] = depth;
		PixelCam3DVertex[total] = Cam3Dvertex;
		PixelNorVector[total] = NorVec;
		PixelptrMaterial[total] = ptrmtl;
	}
}
void MoonBuffer::PutPixelColor_only(const int x, const int y, RGBa Color) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;
		PixelColor[total] = Color;
	}

}
;



RGBa MoonBuffer::GetPixelColor(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return RGBa();
	}
	else {
		long total = y * Buffer_size[0] + x;
		return PixelColor[total];
		
	}
};

MoonMaterial* MoonBuffer::GetPtrMtl(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return nullptr;
	}
	else {
		long total = y * Buffer_size[0] + x;
		return PixelptrMaterial[total];
	}
}

Vec3 MoonBuffer::GetCam3Dvertex(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return Vec3();
	}
	else {
		long total = y * Buffer_size[0] + x;
		return PixelCam3DVertex[total];
	}
}

Vec3 MoonBuffer::GetNorVec(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return Vec3();
	}
	else {
		long total = y * Buffer_size[0] + x;
		return PixelNorVector[total];
	}

}


bool MoonBuffer::CompareDepth_Smaller(const int x, const int y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		throw std::runtime_error("DepthBuffer Unexpected comparision : out of buffer!");
	}
	if (Depth[y * Buffer_size[0] + x] > depth_in) {
		return true;
	}else return false;
	
}

double MoonBuffer::GetDepth(const int x, const int y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return 1.0;
	} else {
		return Depth[y * Buffer_size[0] + x];
	}

};


/*
void MoonBuffer::merge(const MoonBuffer& Buffer_chunk) {
	// 检查缓冲区大小是否一致
	if (Buffer_chunk.Buffer_size[0] != Buffer_size[0] || Buffer_chunk.Buffer_size[1] != Buffer_size[1]) {
		throw std::runtime_error("Buffer sizes dont match!");
	}

	const long size = Buffer_size[0] * Buffer_size[1];

	// 合并缓冲区
	for (long e = 0; e < size; ++e) {
		if (Buffer_chunk.Depth[e] < Depth[e]) {

			PixelColor[e] = Buffer_chunk.PixelColor[e];
			Depth[e] = Buffer_chunk.Depth[e];
			NorVector[e] = Buffer_chunk.NorVector[e];

		}
	}
}

*/