#include "moon.h"

void Buffer::SetBuffer(long width, long height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;
	/*
	Red.clear();
	Green.clear();
	Blue.clear();
	Alapha.clear();
	*/
	PixelColor.clear();
	Depth.clear();

	/*
	Red.resize(size, 0.0f);
	Green.resize(size, 0.0f);
	Blue.resize(size, 0.0f);
	Alapha.resize(size, 1.0f);
	*/
	PixelColor.resize(size, Color{ 0.05f, 0.05f, 0.06f, 1.0f });
	Depth.resize(size, 1.0);

}

void Buffer::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	/*
	Red.assign(total, 0.05f);
	Green.assign(total, 0.05f);
	Blue.assign(total, 0.06f);
	Alapha.assign(total, 1.0f);
	*/
	PixelColor.assign(total, Color{ 0.05f, 0.05f, 0.06f, 1.0f });
	Depth.assign(total, 1.0);  //初始DepthBuffer
};



void Buffer::PutPixel(long x, long  y, double depth, Color c) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1] ) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;
		PixelColor[total] = c;
		/*
		Red[total] = c.R;
		Green[total] = c.G;
		Blue[total] = c.B;
		*/
		
		Depth[total] = depth;
	}
};


Color Buffer::GetPixelColor(const long x, const long y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return { 0,0,0 };
	}
	else {
		long total = y * Buffer_size[0] + x;
		/*
		Color back;
		
		back.R = Red[total];
		back.G = Green[total];
		back.B = Blue[total];
		back.a = Alapha[total];
		*/
		return PixelColor[total];
		//return back;
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
			/*
			Red[e] = Buffer_chunk.Red[e];
			Green[e] = Buffer_chunk.Green[e];
			Blue[e] = Buffer_chunk.Blue[e];
			*/
			PixelColor[e] = Buffer_chunk.PixelColor[e];
			Depth[e] = Buffer_chunk.Depth[e];
		}
	}
}