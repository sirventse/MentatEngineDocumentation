#include <../include/MentatEngine/MeshLoader.hpp>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <functional>

namespace ME {
	MeshLoader::MeshLoader()
	{
	}

    MeshLoader::~MeshLoader()
    {
    }

    // --- Hash personalizado para std::tuple<float,float,float> ---
    struct Vec3KeyHash {
        std::size_t operator()(const std::tuple<float, float, float>& key) const noexcept {
            auto [x, y, z] = key;
            // Combina los hashes de forma robusta
            std::size_t h1 = std::hash<float>{}(x);
            std::size_t h2 = std::hash<float>{}(y);
            std::size_t h3 = std::hash<float>{}(z);
            return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1);
        }
    };

    // --- Comparador de igualdad para floats (con tolerancia) ---
    struct Vec3KeyEqual {
        bool operator()(const std::tuple<float, float, float>& a,
            const std::tuple<float, float, float>& b) const noexcept {
            auto [ax, ay, az] = a;
            auto [bx, by, bz] = b;
            const float eps = 1e-6f;
            return (std::fabs(ax - bx) < eps) &&
                (std::fabs(ay - by) < eps) &&
                (std::fabs(az - bz) < eps);
        }
    };

    // --- Función principal ---
    ME::LoadedDrawable ME::MeshLoader::LoadObjPositionsFlat(const std::string& filepath) {
        ME::LoadedDrawable mesh; // Create a local instance

        // --- tinyobjloader environment needs ---
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str());
        // ---

        if (!warn.empty()) {
            printf("[TINY_OBJ_WARNING] %s\n", warn.c_str());
        }

        if (!err.empty()) {
            printf("[TINY_OBJ_ERROR] %s\n", err.c_str());
            return ME::LoadedDrawable(); //empty mesh on failure
        }

        if (!ret) {
            // Failed to load, return empty mesh
            return ME::LoadedDrawable();
        }

        for (size_t s = 0; s < shapes.size(); s++) {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                int fv = shapes[s].mesh.num_face_vertices[f];

                for (size_t v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    mesh.vertices.emplace_back(
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    );

                    mesh.indexes.push_back(static_cast<unsigned int>(mesh.indexes.size()));
                }
                index_offset += fv;
            }
        }
        return mesh;
    }
}

