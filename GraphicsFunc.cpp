#include "moon.h"

#include <iostream>

long Graphics::Buffer_size[2];
std::vector<double> Graphics::RedBuffer;
std::vector<double> Graphics::GreenBuffer;
std::vector<double> Graphics::BlueBuffer;
std::vector<double> Graphics::DepthBuffer;


void Graphics::SetBuffer(long width, long height) {
	if (width == 0 || height == 0) {
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
	}
	return;
};

void Graphics::CleanBuffer() {
	long totall = Buffer_size[0] * Buffer_size[1];
	RedBuffer.assign(totall, 0);
	GreenBuffer.assign(totall, 0);
	BlueBuffer.assign(totall, 0);
	DepthBuffer.assign(totall, 0);
};


void Graphics::PutPixel(long x, long  y, double deepth, Color c) {
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


Color Graphics::GetPixelColor(const long x, const long y) {
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



void Graphics::DrawFlatTopTriangle(const vertex& v0, const vertex& v1, const vertex& v2, const Color& c) {
	//斜率倒数
	double m0 = (v2.x - v0.x) / (v2.y - v0.y);
	double m1 = (v2.x - v1.x) / (v2.y - v1.y);

	//起始扫描线
	int yStart = (int)ceil(v0.y - 0.5);
	int yEnd = (int)ceil(v2.y - 0.5);

	if (yStart < 0) {
		yStart = 0;
	}
	else if (yStart > Buffer_size[1]) {
		yStart = Buffer_size[1];

	}
	if (yEnd < 0) {
		yEnd = 0;
	}
	else if (yEnd > Buffer_size[1]) {
		yEnd = Buffer_size[1];

	}

	//这里需要加入一个阻止绘制超界的三角形的代码

	for (int y = yStart; y < yEnd; ++y) {
		const double px0 = m0 * (double(y) + 0.5f - v0.y) + v0.x;
		const double px1 = m1 * (double(y) + 0.5f - v1.y) + v1.x;

		int xStart = (int)ceil(px0 - 0.5f);
		int xEnd = (int)ceil(px1 - 0.5f);

		if (xStart < 0) {
			xStart = 0;
		}
		else if (yStart > Buffer_size[0]) {
			xStart = Buffer_size[0];

		}
		if (xEnd < 0) {
			xEnd = 0;
		}
		else if (xEnd > Buffer_size[0]) {
			xEnd = Buffer_size[0];

		}

		for (int x = xStart; x < xEnd; ++x) {
			PutPixel(x, y, 0, c);

		}
	}
};

void Graphics::DrawFlatBottomTriangle(const vertex& v0, const vertex& v1, const vertex& v2, const Color& c) {
	//斜率倒数
	double m0 = (v1.x - v0.x) / (v1.y - v0.y);
	double m1 = (v2.x - v0.x) / (v2.y - v0.y);

	//起始扫描线
	int yStart = (int)ceil(v0.y - 0.5f);
	int yEnd = (int)ceil(v2.y - 0.5f);

	if (yStart < 0) {
		yStart = 0;
	} else if (yStart > Buffer_size[1]) {
		yStart = Buffer_size[1];

	}
	if (yEnd < 0) {
		yEnd = 0;
	}
	else if (yEnd > Buffer_size[1]) {
		yEnd = Buffer_size[1];

	}


	for (int y = yStart; y < yEnd; ++y) {
		const double px0 = m0 * (double(y) + 0.5f - v0.y) + v0.x;
		const double px1 = m1 * (double(y) + 0.5f - v0.y) + v0.x;

		int xStart = (int)ceil(px0 - 0.5f);
		int xEnd = (int)ceil(px1 - 0.5f);

		if (xStart < 0) {
			xStart = 0;
		}
		else if (yStart > Buffer_size[0]) {
			xStart = Buffer_size[0];

		}
		if (xEnd < 0) {
			xEnd = 0;
		}
		else if (xEnd > Buffer_size[0]) {
			xEnd = Buffer_size[0];

		}

		for (int x = xStart; x < xEnd; ++x) {
			PutPixel(x, y, 0, c);

		}
	}
};

void Graphics::DrawTriangle(const vertex& v0, const vertex& v1, const vertex& v2, Color& c) {
	const vertex* pv0 = &v0;
	const vertex* pv1 = &v1;
	const vertex* pv2 = &v2;

	//交换上下顺序
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);
	if (pv2->y < pv1->y)  std::swap(pv2, pv1);
	if (pv1->y < pv0->y)  std::swap(pv0, pv1);

	//自然平顶三角形
	if (pv1->y == pv0->y) {
		if (pv1->x < pv0->x) std::swap(pv0, pv1);
		DrawFlatTopTriangle(*pv0, *pv1, *pv2, c);
	}
	else if (pv1->y == pv2->y) {
		if (pv1->x > pv2->x) std::swap(pv2, pv1);
		DrawFlatBottomTriangle(*pv0, *pv1, *pv2, c);
	}
	else {
		const double alphaSplit = (pv1->y - pv0->y) / (pv2->y - pv0->y);
		const vertex vi = {	pv0->x + (pv2->x - pv0->x) * alphaSplit,
									pv1->y,
									pv0->z + (pv2->z - pv0->z) * alphaSplit };

		if (pv1->x < vi.x) {
			//major right
			DrawFlatBottomTriangle(*pv0, *pv1, vi, c);
			DrawFlatTopTriangle(*pv1, vi, *pv2, c);
		}
		else {
			//major left
			DrawFlatBottomTriangle(*pv0, vi, *pv1, c);
			DrawFlatTopTriangle(vi, *pv1, *pv2, c);
		}
	}


};


