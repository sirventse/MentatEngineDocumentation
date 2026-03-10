/**
 *
 * @brief Engine core class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#include <vector>
#include <cstddef>
#include <../deps/KFS/include/vec3.h>
#include <../include/MentatEngine/OpenGL/VertexArray.hpp>
#include <../include/MentatEngine/OpenGL/Buffer.hpp>


#ifndef __RENDERIZABLE_COMPONENT_HPP__
#define __RENDERIZABLE_COMPONENT_HPP__ 1

static Vec3 NormalizeSafe(const Vec3& v) {
    float len = sqrtf(v.x_ * v.x_ + v.y_ * v.y_ + v.z_ * v.z_);
    if (len < 1e-6f) return Vec3{ 0.0f, 1.0f, 0.0f };
    return Vec3{ v.x_ / len, v.y_ / len, v.z_ / len };
}

static Vec3 Cross(const Vec3& a, const Vec3& b) {
    return Vec3{
      a.y_ * b.z_ - a.z_ * b.y_,
      a.z_ * b.x_ - a.x_ * b.z_,
      a.x_ * b.y_ - a.y_ * b.x_
    };
}

static Vec3 Sub(const Vec3& a, const Vec3& b) {
    return Vec3{ a.x_ - b.x_, a.y_ - b.y_, a.z_ - b.z_ };
}

static void ComputeSmoothNormals(
    const std::vector<Vec3>& positions,
    const std::vector<unsigned int>& indices,
    std::vector<Vec3>& outNormals
) {
    outNormals.assign(positions.size(), Vec3{ 0,0,0 });

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        const auto& p0 = positions[i0];
        const auto& p1 = positions[i1];
        const auto& p2 = positions[i2];

        Vec3 e1 = Sub(p1, p0);
        Vec3 e2 = Sub(p2, p0);
        Vec3 n = Cross(e1, e2); // sin normalizar para ponderar por área

        outNormals[i0].x_ += n.x_; outNormals[i0].y_ += n.y_; outNormals[i0].z_ += n.z_;
        outNormals[i1].x_ += n.x_; outNormals[i1].y_ += n.y_; outNormals[i1].z_ += n.z_;
        outNormals[i2].x_ += n.x_; outNormals[i2].y_ += n.y_; outNormals[i2].z_ += n.z_;
    }

    for (auto& n : outNormals) n = NormalizeSafe(n);
}

namespace ME {
    

    struct VertexPN {
        Vec3 pos;
        Vec3 normal;
    };

	class RenderizableComponent {
        public:
            /**
             * @brief Constructs a RenderizableComponent with initial mesh data.
             * @param vertices Pointer to the vertex positions array.
             * @param vert_num Number of vertices in the array.
             * @param indexes Pointer to the index buffer array.
             * @param index_count Number of indices for indexed drawing.
             * @param color The base RGB color of the mesh.
             * @param path The filesystem path to the mesh source file. Leave empty in case its not needed.
             */
            RenderizableComponent(const Vec3* vertices,
                const size_t& vert_num,
                const unsigned int* indexes,
                const size_t& index_count,
                const Vec3 color,
                std::string path);

            RenderizableComponent(RenderizableComponent&& other) noexcept : vao_(std::move(other.vao_)), vbo_(std::move(other.vbo_)),
                ebo_(std::move(other.ebo_)), vertex_amount_(other.vertex_amount_), index_count_(other.index_count_),
                original_vertices_(std::move(other.original_vertices_)), color_{ other.color_ }, mesh_path_{ other.mesh_path_ }
            {
                other.vertex_amount_ = 0;
                other.index_count_ = 0;
            }

            RenderizableComponent& operator=(RenderizableComponent&& other) noexcept {
                if (this != &other) {
                    vao_ = std::move(other.vao_);
                    vbo_ = std::move(other.vbo_);
                    ebo_ = std::move(other.ebo_);
                    vertex_amount_ = other.vertex_amount_;
                    index_count_ = other.index_count_;
                    original_vertices_ = std::move(other.original_vertices_);
                    mesh_path_ = other.mesh_path_;
                    color_ = other.color_;
                    other.vertex_amount_ = 0;
                    other.index_count_ = 0;
                }
                return *this;
            }

            RenderizableComponent(const RenderizableComponent&) = delete;

            RenderizableComponent& operator=(const RenderizableComponent&) = delete;

            ~RenderizableComponent();

            /**
             * @brief Replaces the current mesh data with a new set of buffers.
             * @param vertices New vertex position array.
             * @param vert_num New vertex count.
             * @param indexes New index buffer array.
             * @param index_count New index count.
             * @param new_path Updated source path for the mesh. Leave empty in case its not needed.
             */
            void ReplaceMesh(const Vec3* vertices, size_t vert_num, const unsigned int* indexes, size_t index_count, char* new_path);

            /**
             * @brief Binds the internal Vertex Array Object to the GPU context.
             */
            void Bind() const;

            /**
             * @brief Unbinds the internal Vertex Array Object from the GPU context.
             */
            void Unbind() const;

            /**
             * @brief Updates the existing vertex buffer data on the GPU.
             * @param vertices Pointer to the updated vertex array.
             * @param vert_num Number of vertices to update.
             */
            void UpdateGeometry(Vec3* vertices, unsigned int vert_num);

            /**
             * @brief Returns a reference to the originally loaded vertex data.
             */
            const std::vector<Vec3>& GetOriginalVertices() const { return original_vertices_; }

            /**
             * @brief Returns the filesystem path used to load the mesh.
             */
            std::string GetMeshPath() { return mesh_path_; }

            /**
             * @brief Sets the filesystem path for the diffuse texture.
             */
            void SetTexurePath(std::string path) { texure_path_ = path; }

            /**
             * @brief Returns the current filesystem path of the texture.
             */
            std::string GetTexurePath() { return texure_path_; }

            /**
             * @brief Returns the raw OpenGL handle for the Vertex Array Object.
             */
            GLuint GetVAO();

            /**
             * @brief Returns the total count of vertices in the mesh.
             */
            GLuint GetVertexAmount();

            /**
             * @brief Returns the total count of indices in the index buffer.
             */
            GLuint GetIndexCount();

            /**
             * @brief Checks if the component is currently marked for rendering.
             */
            bool Visible() { return isVisible_; };

            /**
             * @brief Toggles the visibility state of the component.
             */
            void SetVisibility(bool v) { isVisible_ = v; };

            /**
             * @brief Returns the base RGB color of the component.
             */
            Vec3 GetColor();

            /**
             * @brief Sets a new base RGB color for the component.
             */
            void SetColor(const Vec3& c) { color_ = c; };

            /**
             * @brief Checks if the base color is active in the shader.
             */
            bool IsColorActive();

            /**
             * @brief Enables or disables the use of the base color in rendering.
             */
            void SetColorActive(bool isActive) { is_color_active_ = isActive; };


        private:
            VertexArray vao_;
            Buffer vbo_, ebo_;
            GLuint vertex_amount_, index_count_, texture_id_;
            std::vector<Vec3> original_vertices_;
            std::vector<unsigned int> original_indices_;
            std::vector<VertexPN> vertices_pn_; // buffer real con normales
            std::string mesh_path_, texure_path_;
            Vec3 color_;
            bool is_color_active_;
            bool isVisible_;
	};
}

#endif // __RENDERIZABLE_COMPONENT_HPP__