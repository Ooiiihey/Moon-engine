#include "Moon.h"

//初始DepthBuffer
void DepthBuffer::SetBuffer(int width, int height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	Depth.clear();
	Depth.resize(size, 1.0);


}


void DepthBuffer::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];
	Depth.assign(total, 1.0);

};

bool DepthBuffer::CompareDepth_Smaller(const int x, const int y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		//return true;
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

void DepthBuffer::PutDepth(const int x, const int y, double MapsList) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	long total = y * Buffer_size[0] + x;
	Depth[total] = MapsList;

}




//collection

//初始化
void BufferCollection::SetBuffer(int width, int height) {
	if (width <= 0 || height <= 0) throw std::runtime_error("Unexpected Buffer size!");

	Buffer_size[0] = width;
	Buffer_size[1] = height;
	const size_t size = width * height;

	All_in.clear();
	All_in.resize(size, PixelData());
}

void BufferCollection::CleanBuffer() {
	long total = Buffer_size[0] * Buffer_size[1];

	All_in.assign(total, PixelData());

};


// 放入所有数据
void BufferCollection::PutPixelAll(const int x, const int y, double depth, RGBa c, Vec3& Cam3Dvertex, Vec3& NorVec_in, Material_M* ptrmtl) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1] ) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;

		All_in[total] = {
			c,
			depth,
			Cam3Dvertex,
			NorVec_in,
			ptrmtl,
			RGBa()
		};
	}
}

void BufferCollection::PutPixelAll(const int x, const int y, const PixelData& P) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	else {
		long total = y * Buffer_size[0] + x;

		All_in[total] = P;
	}
};

PixelData BufferCollection::GetPixelAll(const int x, const int y)
{
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return PixelData();
	}
	long total = y * Buffer_size[0] + x;
	return All_in[total];

};



void BufferCollection::AddFrameColor(const int x, const int y, RGBa Color) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	long total = y * Buffer_size[0] + x;
	All_in[total].FrameColor += Color;


};

void BufferCollection::PutFrameColor(const int x, const int y, RGBa Color) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return;
	}
	long total = y * Buffer_size[0] + x;
	All_in[total].FrameColor = Color;

}



RGBa BufferCollection::GetTextureColor(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return RGBa();
	}
	long total = y * Buffer_size[0] + x;
	return All_in[total].TextureColor;

};

RGBa BufferCollection::GetFrameColor(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return RGBa();
	}
	long total = y * Buffer_size[0] + x;
	return All_in[total].FrameColor;

};

Material_M* BufferCollection::GetPtrMtl(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return nullptr;
	}
	long total = y * Buffer_size[0] + x;
	return All_in[total].ptrMaterial;

}

Vec3 BufferCollection::GetCam3Dvertex(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return Vec3();
	}
	long total = y * Buffer_size[0] + x;
	return All_in[total].Cam3DVertex;

}

Vec3 BufferCollection::GetNorVec(const int x, const int y) {
	if (x < 0 || x > Buffer_size[0] - 1 || y < 0 || y > Buffer_size[1] - 1) {
		return Vec3();
	}
	long total = y * Buffer_size[0] + x;
	return All_in[total].NorVec;


}




bool BufferCollection::CompareDepth_Smaller(const int x, const int y, double depth_in) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		throw std::runtime_error("DepthBuffer Unexpected comparision : out of buffer!");
	}

	if (All_in[y * Buffer_size[0] + x].Depth > depth_in) {
		return true;
	}
	return false;
}

double BufferCollection::GetDepth(const int x, const int y) {
	if (x < 0 || x >= Buffer_size[0] || y < 0 || y >= Buffer_size[1]) {
		return 1.0;
	}
	return All_in[y * Buffer_size[0] + x].Depth;


};



void BufferCollection::merge(const BufferCollection& Buffer_chunk) {
	// 检查缓冲区大小是否一致
	if (Buffer_chunk.Buffer_size[0] != Buffer_size[0] || Buffer_chunk.Buffer_size[1] != Buffer_size[1]) {
		throw std::runtime_error("Buffer sizes dont match!");
	}

	const long size = Buffer_size[0] * Buffer_size[1];

	// 合并缓冲区
	for (long e = 0; e < size; ++e) {
		if (Buffer_chunk.All_in[e].Depth < All_in[e].Depth) {
			All_in[e] = Buffer_chunk.All_in[e];

		}
	}
}

