#include "Moon.h"


void DepthBuffer::SetBuffer(int width, int height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	Depth.clear();
	Depth.resize(size, INFINITY);


}

void DepthBuffer::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	Depth.assign(total, INFINITY);  //初始DepthBuffer

};

bool DepthBuffer::CompareDepth_Smaller(const int x, const int y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		throw std::runtime_error("DepthBuffer Unexpected comparision : out of buffer!");
	}
	if (Depth[y * Buffer_size[0] + x] > depth_in) {
		return true;
	}
	return false;

}

double DepthBuffer::GetDepth(const int x, const int y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return 1.0;
	}
	return Depth[y * Buffer_size[0] + x];

};

void DepthBuffer::PutDepth(const int x, const int y, double depthBuffersList) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	long total = y * Buffer_size[0] + x;
	Depth[total] = depthBuffersList;

}




//collection


void BufferCollection::SetBuffer(int width, int height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	TextureColor.clear();
	Depth.clear();
	Cam3DVertex.clear();
	NorVec.clear();
	ptrMaterial.clear();
	Frame.clear();


	TextureColor.resize(size, RGBa());
	Depth.resize(size, 1.0);
	Cam3DVertex.resize(size, Vec3());
	NorVec.resize(size, Vec3());
	ptrMaterial.resize(size, nullptr);
	Frame.resize(size, RGBa());

}

void BufferCollection::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	TextureColor.assign(total, RGBa());
	Depth.assign(total, 1.0);  //初始DepthBuffer
	Cam3DVertex.assign(total, Vec3());
	NorVec.assign(total, Vec3());
	ptrMaterial.assign(total, nullptr);
	Frame.assign(total, RGBa());
};



void BufferCollection::PutPixelData(const int x, const int y, double depthBuffersList, RGBa c, Vec3& Cam3Dvertex, Vec3& NorVec_in, MoonMaterial* ptrmtl) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1] ) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;
		TextureColor[total] = c;
		Depth[total] = depthBuffersList;
		Cam3DVertex[total] = Cam3Dvertex;
		NorVec[total] = NorVec_in;
		ptrMaterial[total] = ptrmtl;
	}
}

void BufferCollection::AddFinalColor(const int x, const int y, RGBa Color) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	long total = y * Buffer_size[0] + x;

	Frame[total] = Frame[total] + Color;


};

void BufferCollection::PutFinalColor(const int x, const int y, RGBa Color) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	long total = y * Buffer_size[0] + x;

	Frame[total] = Color;


}



RGBa BufferCollection::GetPixelTextureColor(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return RGBa();
	}
	long total = y * Buffer_size[0] + x;
	return TextureColor[total];

};

RGBa BufferCollection::GetFramePixelColor(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return RGBa();
	}
	long total = y * Buffer_size[0] + x;
	return RGBa(
		std::clamp(Frame[total].R, 0.0f, 1.0f),
		std::clamp(Frame[total].G, 0.0f, 1.0f),
		std::clamp(Frame[total].B, 0.0f, 1.0f));

};

MoonMaterial* BufferCollection::GetPtrMtl(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return nullptr;
	}
	long total = y * Buffer_size[0] + x;
	return ptrMaterial[total];

}

Vec3 BufferCollection::GetCam3Dvertex(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return Vec3();
	}
	long total = y * Buffer_size[0] + x;
	return Cam3DVertex[total];

}

Vec3 BufferCollection::GetNorVec(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return Vec3();
	}
	long total = y * Buffer_size[0] + x;
	return NorVec[total];


}


bool BufferCollection::CompareDepth_Smaller(const int x, const int y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		throw std::runtime_error("DepthBuffer Unexpected comparision : out of buffer!");
	}
	if (Depth[y * Buffer_size[0] + x] > depth_in) {
		return true;
	}
	return false;
	
}

double BufferCollection::GetDepth(const int x, const int y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return 1.0;
	}
	return Depth[y * Buffer_size[0] + x];


};



void BufferCollection::merge(const BufferCollection& Buffer_chunk) {
	// 检查缓冲区大小是否一致
	if (Buffer_chunk.Buffer_size[0] != Buffer_size[0] || Buffer_chunk.Buffer_size[1] != Buffer_size[1]) {
		throw std::runtime_error("Buffer sizes dont match!");
	}

	const long size = Buffer_size[0] * Buffer_size[1];

	// 合并缓冲区
	for (long e = 0; e < size; ++e) {
		if (Buffer_chunk.Depth[e] < Depth[e]) {

			TextureColor[e] = Buffer_chunk.TextureColor[e];
			Depth[e] = Buffer_chunk.Depth[e];
			Cam3DVertex[e] = Buffer_chunk.Cam3DVertex[e];
			NorVec[e] = Buffer_chunk.NorVec[e];
			ptrMaterial[e] = Buffer_chunk.ptrMaterial[e];
			Frame[e] = Buffer_chunk.Frame[e];

		}
	}
}

