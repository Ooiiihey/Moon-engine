#include "moon.h"

#include <iostream>

std::vector<std::vector<Pixel>> Rasterization_function::Frame_Buffer;

void Rasterization_function::SetFrameBuffer(unsigned int width, unsigned int height) {
	if (width == 0 || height == 0)  throw std::runtime_error("FrameBuffer-Unexpected screen size!");
	Frame_Buffer.resize(width);
	for (std::vector<Pixel> &each_list : Frame_Buffer) {
		each_list.resize(height);
	}

}


void Rasterization_function::PutPixel(double x, double y , double deepth, Color c) {
	if (x < 0 || x > Frame_Buffer.size() || y < 0 || y >Frame_Buffer[0].size())  return;
	Frame_Buffer[static_cast<unsigned int>(x)][static_cast<unsigned int>(y)].color = c;
	Frame_Buffer[static_cast<unsigned int>(x)][static_cast<unsigned int>(y)].deepth = deepth;
	//test
	//std::cout << Frame_Buffer[static_cast<unsigned int>(x)][static_cast<unsigned int>(y)].deepth << std::endl;
}

void Rasterization_function::DrawFlatTopTriangle() {
	std::cout << "test!" << std::endl;
	return;
}

void Rasterization_function::DrawTriangle(double *pv1, double* pv2, double* pv3, Color &c) {
		

		std::cout << "test!" << std::endl;
		return;
}


