#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "moon.h"

// 计算向量归一化
Vec3 Normalize(Vec3 vec) {
    double length = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
    if (length == 0) return vec;
    return Vec3(vec.x / length, vec.y / length, vec.z / length);
}

// 计算面法向量
Vec3 CalculateFaceNormal(const Vec3& v0, const Vec3& v1, const Vec3& v2) {
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
    Vec3 normal = cross(edge1, edge2);
    return Normalize(normal);
}


// 加载模型并转换为 Mesh
Mesh LoadModel(const std::string& filename) {
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filename, reader_config)) {
        std::cerr << "Failed to load: " << filename << ". Error: " << reader.Error() << std::endl;
        exit(1);
    }

    if (!reader.Valid()) {
        std::cerr << "Model is not valid." << std::endl;
        exit(1);
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();

    Mesh mesh;

    // 转换顶点
    for (size_t i = 0; i < attrib.vertices.size(); i += 3) {
        Vec3 vertex(
            static_cast<float>(  attrib.vertices[i]),
            static_cast<float>(attrib.vertices[i + 1]),
            static_cast<float>(attrib.vertices[i + 2])
        );
        mesh.vertices.push_back(vertex);
    }

    // 转换面并生成颜色

    for (const auto& shape : shapes) {
        for (size_t face_idx = 0; face_idx < shape.mesh.num_face_vertices.size(); ++face_idx) {
            const auto& face = shape.mesh.num_face_vertices[face_idx];
            const auto& material_id = shape.mesh.material_ids[face_idx];
            Face face_indices;
            Color face_color;

            // 获取顶点索引
            std::vector<tinyobj::index_t> indices;
            for (int i = 0; i < face; ++i) {
                indices.push_back(shape.mesh.indices[face_idx * face + i]);
            }

            // 获取顶点坐标
            Vec3 v0 = mesh.vertices[indices[0].vertex_index];
            Vec3 v1 = mesh.vertices[indices[1].vertex_index];
            Vec3 v2 = mesh.vertices[indices[2].vertex_index];

            // 计算面法向量
            Vec3 normal = CalculateFaceNormal(v0, v1, v2);

            // 定义平行光方向
            Vec3 lightDirection(0.5f, -0.2f, -1.0f); // 平行光方向

            // 计算光照强度（使用法向量与光线方向的点积）
            Vec3 normalizedNormal = Normalize(normal);
            Vec3 normalizedLightDirection = Normalize(lightDirection);
            double diffuseIntensity = dot(normalizedNormal, normalizedLightDirection);

            // 将光照强度映射到颜色
            diffuseIntensity = (diffuseIntensity + 1.0) / 2.0; // 将范围从 [-1, 1] 映射到 [0, 1]

            // 设置颜色
            face_color.R = static_cast<float>(diffuseIntensity);
            face_color.G = static_cast<float>(diffuseIntensity);
            face_color.B = static_cast<float>(diffuseIntensity);
            face_color.a = 1.0f;

            // 赋值顶点索引
            for (int i = 0; i < face; ++i) {
                face_indices.index[i] = indices[i].vertex_index;
            }
            std::swap(face_indices.index[0], face_indices.index[2]);

            mesh.faces.push_back(face_indices);
            mesh.color.push_back(face_color);
        }
    }

    return mesh;
}