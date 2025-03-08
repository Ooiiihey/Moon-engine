#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include "tiny_obj_loader.h"

#include "Moon.h"


// 加载模型并转换为 Mesh
Mesh Load::LoadMesh(const std::string& filename) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true; // 启用自动三角化
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, reader_config)) {
        std::cerr << "Failed to load: " << filename << ". Error: " << reader.Error() << std::endl;
        exit(114);
    }

    if (!reader.Valid()) {
        std::cerr << "Model is not valid." << std::endl;
        exit(514);
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    Mesh mesh;

    // 加载顶点（调整坐标系：x轴反转，y和z交换）
    mesh.vertices.reserve(attrib.vertices.size() / 3);
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        mesh.vertices.emplace_back(
            -attrib.vertices[i],      // x轴反转
            attrib.vertices[i + 2],   // z -> y
            attrib.vertices[i + 1]    // y -> z
        );
    }

    // 加载UV坐标
    mesh.UVCoords.reserve(attrib.texcoords.size() / 2);
    for (size_t i = 0; i < attrib.texcoords.size(); i += 2) {
        mesh.UVCoords.emplace_back(
            attrib.texcoords[i],
            1.0 - attrib.texcoords[i + 1] // 适配DirectX的UV坐标系
        );
    }

    // 确保至少有一个默认UV坐标
    if (mesh.UVCoords.empty()) {
        mesh.UVCoords.emplace_back(0.0, 0.0);
    }

    // 加载面数据（已三角化）
    for (const auto& shape : shapes) {
        const auto& indices = shape.mesh.indices;
        const size_t num_faces = shape.mesh.num_face_vertices.size();

        mesh.facesIndex.reserve(mesh.facesIndex.size() + num_faces);
        mesh.texIndex.reserve(mesh.texIndex.size() + num_faces);

        for (size_t face_idx = 0; face_idx < num_faces; ++face_idx) {
            const size_t base_idx = face_idx * 3;

            // 顶点索引（交换顺序以修正缠绕顺序）
            Face face;
            face.index[0] = indices[base_idx + 2].vertex_index;
            face.index[1] = indices[base_idx + 1].vertex_index;
            face.index[2] = indices[base_idx].vertex_index;
            mesh.facesIndex.push_back(face);

            // UV索引（同步调整顺序）
            Face uv_face;
            for (int i = 0; i < 3; ++i) {
                int uv_idx = indices[base_idx + (2 - i)].texcoord_index; // 同步顺序调整
                uv_face.index[i] = (uv_idx >= 0 && uv_idx < (int)mesh.UVCoords.size()) ? uv_idx : 0;
            }
            mesh.texIndex.push_back(uv_face);
        }
    }

    // 数据完整性校验
    if (mesh.facesIndex.size() != mesh.texIndex.size()) {
        std::cerr << "Error: Face count (" << mesh.facesIndex.size()
            << ") does not match UV face count (" << mesh.texIndex.size() << ")" << std::endl;
        exit(12);
    }

    return mesh;
}

MoonTexture Load::LoadTextureImage(const std::string& filename) {
    int Width, Height, Channels;
    unsigned char* ImageData = stbi_load(filename.c_str(), &Width, &Height, &Channels, 4);

    if (!ImageData) {
        std::cerr << "Failed to load: " << filename  << std::endl;
        exit(36);
    }

    MoonTexture tex;
    tex.SetTexture(Width, Height);

    for (int y = 0; y < Height; ++y) {
        for (int x = 0; x < Width; ++x) {
            int index = (y * Width + x) * Channels;
            RGBa pixel;

            pixel.R = (float)ImageData[index + 0] / 255.0f;
            pixel.G = (float)ImageData[index + 1] / 255.0f;
            pixel.B = (float)ImageData[index + 2] / 255.0f;
            pixel.a = (float)ImageData[index + 3] / 255.0f;

            tex.Pixels.emplace_back(pixel);
            /*
            for (int c = 0; c < Channels; ++c) {
                int index = (y * Width + x) * Channels + c;
                unsigned char value = ImageData[index];

                float t = (float)value / 255.0f;

            }
        */
        }
    }

    stbi_image_free(ImageData);

    return std::move(tex);

}