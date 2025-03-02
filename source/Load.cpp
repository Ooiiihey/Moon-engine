#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "moon.h"


// 加载模型并转换为 Mesh
Mesh Load::LoadMesh(const std::string& filename) {
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, reader_config)) {
        std::cerr << "Failed to load: " << filename << ". Error: " << reader.Error() << std::endl;
        exit(114514);
    }

    if (!reader.Valid()) {
        std::cerr << "Model is not valid." << std::endl;
        exit(1919810);
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    Mesh mesh;

    // 转换顶点
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        Vec3 vertex(
            static_cast<double>( - attrib.vertices[i]),
            static_cast<double>(attrib.vertices[i + 2]),
            static_cast<double>(attrib.vertices[i + 1])
        );
        mesh.vertices.push_back(vertex);
    }

    // 转换面并生成颜色

    for (const auto& shape : shapes) {
        for (size_t face_idx = 0; face_idx < shape.mesh.num_face_vertices.size(); ++face_idx) {
            const auto& face = shape.mesh.num_face_vertices[face_idx];
            const auto& material_id = shape.mesh.material_ids[face_idx];
            Face face_indices;
            //RGBa face_color;

            // 获取顶点索引
            std::vector<tinyobj::index_t> indices;
            for (int i = 0; i < face; ++i) {
                indices.push_back(shape.mesh.indices[face_idx * face + i]);
            }

            // 赋值顶点索引
            for (int i = 0; i < face; ++i) {
                face_indices.index[i] = indices[i].vertex_index;
            }
            std::swap(face_indices.index[0], face_indices.index[2]);

            mesh.faces.push_back(face_indices);

        }
    }

    return mesh;
}