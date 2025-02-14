#include "moon.h"

void Buffer::SetBuffer(long width, long height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	std::lock_guard<std::mutex> lock(mtx);
	Red.clear();
	Green.clear();
	Blue.clear();
	Alapha.clear();
	Depth.clear();

	Red.resize(size, 0);
	Green.resize(size, 0);
	Blue.resize(size, 0);
	Alapha.resize(size, 1.0);
	Depth.resize(size, 1.0);

}

void Buffer::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	std::lock_guard<std::mutex> lock(mtx);
	Red.assign(total, 0.05);
	Green.assign(total, 0.05);
	Blue.assign(total, 0.06);
	Alapha.assign(total, 1.0);
	Depth.assign(total, 1.0);  //初始DepthBuffer
};



void Buffer::PutPixel(long x, long  y, double depth, Color c) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1] ) {
		return;
	}
	else {
		std::lock_guard<std::mutex> lock(mtx);
		long total = y * Buffer_size[0] + x;
		Red[total] = c.R;
		Green[total] = c.G;
		Blue[total] = c.B;
		Depth[total] = depth;
	}
};


Color Buffer::GetPixelColor(const long x, const long y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return { 0,0,0 };
	}
	else {
		Color back;
		long total = y * Buffer_size[0] + x;
		back.R = Red[total];
		back.G = Green[total];
		back.B = Blue[total];
		return back;
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
		return 1;
	} else {
		return Depth[y * Buffer_size[0] + x];
	}

};

void Buffer::merge(const Buffer& Buffer_chunk) {
	std::lock_guard<std::mutex> lock(mtx);
	// 检查缓冲区大小是否一致
	if (Buffer_chunk.Buffer_size[0] != Buffer_size[0] || Buffer_chunk.Buffer_size[1] != Buffer_size[1]) {
		throw std::runtime_error("Buffer sizes dont match!");
	}

	const long size = Buffer_size[0] * Buffer_size[1];

	// 合并缓冲区
	for (long e = 0; e < size; ++e) {
		if (Buffer_chunk.Depth[e] < Depth[e]) {
			Red[e] = Buffer_chunk.Red[e];
			Green[e] = Buffer_chunk.Green[e];
			Blue[e] = Buffer_chunk.Blue[e];
			Depth[e] = Buffer_chunk.Depth[e];
		}
	}
}