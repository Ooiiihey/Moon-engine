#include "moon.h"


void Buffer::SetBuffer(long width, long height) {
	if (width < 0 || height <= 0) {
		throw std::runtime_error("FrameBuffer-Unexpected screen size!");
	}
	else {
		Buffer_size[0] = width;
		Buffer_size[1] = height;

		RedBuffer.clear();
		GreenBuffer.clear();
		BlueBuffer.clear();
		DepthBuffer.clear();

		RedBuffer.resize(width * height);
		GreenBuffer.resize(width * height);
		BlueBuffer.resize(width * height);
		DepthBuffer.resize(width * height);
		DepthBuffer.assign(width * height, 1);
	}
	return;
};

void Buffer::CleanBuffer() {
	long totall = Buffer_size[0] * Buffer_size[1];
	RedBuffer.assign(totall, 0);
	GreenBuffer.assign(totall, 0);
	BlueBuffer.assign(totall, 0);
	DepthBuffer.assign(totall, 1);  //³õÊ¼DepthBuffer
};


void Buffer::PutPixel(long x, long  y, double deepth, Color c) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return;
	}
	else {
		long totall = y * Buffer_size[0] + x;
		RedBuffer[totall] = c.R;
		GreenBuffer[totall] = c.G;
		BlueBuffer[totall] = c.B;
		DepthBuffer[totall] = deepth;
	}
};


Color Buffer::GetPixelColor(const long x, const long y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return { 0,0,0 };
	}
	else {
		Color back;
		long totall = y * Buffer_size[0] + x;
		back.R = RedBuffer[totall];
		back.G = GreenBuffer[totall];
		back.B = BlueBuffer[totall];
		return back;
	}

};

bool Buffer::CompareDepth_Smaller(const long x, const long y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		throw std::runtime_error("FrameBuffer-Unexpected comparision!");
	}
	if (DepthBuffer[y * Buffer_size[0] + x] > depth_in) {
		return true;
	}
	else {
		return false;
	}
}

double Buffer::GetDepth(const long x, const long y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return 1;
	}
	else {
		return DepthBuffer[y * Buffer_size[0] + x];
	}

};

