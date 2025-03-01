#include "moon.h"

void Buffer::SetBuffer(long width, long height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	PixelColor.clear();
	Depth.clear();
	NorVector.clear();


	PixelColor.resize(size, RGBa{ 0.0f, 0.0f, 0.0f, 1.0f });
	Depth.resize(size, 1.0);
	NorVector.resize(size, Vec3(0, 0, 0));

}

void Buffer::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	PixelColor.assign(total, RGBa{ 0.0f, 0.0f, 0.0f, 1.0f });
	Depth.assign(total, 1.0);  //初始DepthBuffer
	NorVector.assign(total, Vec3(0, 0, 0));
};



void Buffer::PutPixel(long x, long  y, double depth, RGBa c, Vec3& NorVec) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1] ) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;
		PixelColor[total] = c;
		Depth[total] = depth;
		NorVector[total] = NorVec;
	}
};


RGBa Buffer::GetPixelColor(const long x, const long y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return RGBa( 0, 0, 0, 0 );
	}
	else {
		long total = y * Buffer_size[0] + x;
		return PixelColor[total];
		
	}

};

bool Buffer::CompareDepth_Smaller(const long x, const long y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		throw std::runtime_error("DepthBuffer Unexpected comparision : out of buffer!");
	}
	if (Depth[y * Buffer_size[0] + x] > depth_in) {
		return true;
	}else return false;
	
}

double Buffer::GetDepth(const long x, const long y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return 1.0;
	} else {
		return Depth[y * Buffer_size[0] + x];
	}

};

void Buffer::merge(const Buffer& Buffer_chunk) {
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