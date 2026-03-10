/**
 *
 * @brief Program class.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
  * glPhong.hpp
  * Renderer Phong multipass (una pasada por luz, acumulación aditiva).
  *
  * Idea clave:
  * - Usa el ECS y recorre componentes por "owner()" (id entidad).
  * - Para cada luz hace un pase de render de TODA la escena.
  * - Primer pase: escribe Z, sin blending.
  * - Pases siguientes: depthFunc = EQUAL y blending aditivo para sumar contribuciones.
  *
  * Consecuencia:
  * - Si tienes N luces y M entidades, haces N * M drawcalls (caro).
  */

#ifndef __GL_PHONG__HPP__
#define __GL_PHONG__HPP__ 1

#include <../../include/MentatEngine/OpenGL/Shader.hpp>
#include <../../include/MentatEngine/OpenGL/Program.hpp>
#include <../../include/MentatEngine/Iterator.hpp>
#include <../../deps/KFS/include/vec3.h>
#include <../../deps/KFS/include/mat4.h>

namespace ME {
    class glPhong {
    public:
        /**
         * @brief Constructor: compiles shaders, links the program, and caches uniforms.
         */
        glPhong();

        glPhong(glPhong&& other) noexcept : shader_(std::move(other.shader_)),
            program_(std::move(other.program_)) {
        }

        glPhong& operator=(glPhong&& other) noexcept {
            if (this != &other) {
                shader_ = std::move(other.shader_);
                program_ = std::move(other.program_);
            }
            return *this;
        }

        glPhong(const glPhong&) = delete;
        glPhong& operator=(const glPhong&) = delete;

        /**
         * @brief Recalculates world matrices for an entity and its hierarchy.
         * @param id The unique ID of the root entity to update.
         */
        void UpdateTransformRecursive(unsigned long id);

        /**
         * @brief Main per-frame render pass.
         * @param dt Delta time since the last frame.
         */
        void Render(float dt);

        /**
         * @brief Returns a pointer to the internal Shader Program.
         */
        Program* GetProgram();

        /**
         * @brief Returns a pointer to the internal Shader object.
         */
        Shader* GetShader();

        /**
         * @brief Debug helper that polls and prints accumulated OpenGL errors.
         * @param type A label to identify where the error check occurred.
         */
        void GetError(const char* type);

        

    private:

        /**
         * @brief Helper that initializes ECS iterators for a specific component list.
         *
         * Sets the start and end iterators. If the container is empty,
         * it marks them as empty to prevent invalid access during Render().
         *
         * @tparam T The component type to iterate over.
         * @param it Pointer to the start iterator to be initialized.
         * @param end Pointer to the end iterator to be initialized.
         * @param first Pointer to the first element's data if available.
         */
        template<typename T>
        void SetIterators(ME::Iterator<T>* it, ME::Iterator<T>* end, T** first) {
            ME::ContainerFacade<T> container{ ME::ECS::GetComponentList<T>() };

            if (!container.isEmpty()) {
                *it = container.begin();
                it->SetEmpty(false);
                *first = **it;
                *end = container.end();
                end->SetEmpty(false);
            }
            else {
                it->SetEmpty(true);
                end->SetEmpty(true);
            }
        }

        /**
         * @brief Helper that advances the component iterator if it matches the current entity.
         *
         * Assumes that the ECS component lists are sorted by entity ID (owner).
         *
         * @tparam T The component type.
         * @param it Pointer to the current iterator.
         * @param end Pointer to the end iterator.
         * @param first Pointer to the current component data.
         * @param current The ID of the entity currently being processed.
         */
        template<typename T>
        void CheckIteratorsFinished(ME::Iterator<T>* it, ME::Iterator<T>* end, T** first, int current) {
            if (*it != *end && it->owner() == current && !it->IsEmpty()) {
                ++(*it);
                if (*it != *end) {
                    *first = **it;
                }
            }
        }

        /**
         * @brief Generates a "Look-At" view transformation matrix.
         * @param eye Camera position.
         * @param center Point the camera is looking at.
         * @param up The world up vector.
         * @return The resulting view matrix.
         */
        Mat4 MakeLookAt(const Vec3& eye, const Vec3& center, const Vec3& up);

        /**
         * @brief Generates a perspective projection matrix.
         * @param fovRadians Field of view in radians.
         * @param aspect Viewport aspect ratio (width/height).
         * @param znear Distance to the near clipping plane.
         * @param zfar Distance to the far clipping plane.
         * @return The resulting projection matrix.
         */
        Mat4 MakePerspective(float fovRadians, float aspect, float znear, float zfar);

        /**
         * @brief Performs matrix multiplication between two Mat4 objects.
         * @param A Left matrix.
         * @param B Right matrix.
         * @return The resulting Mat4 product.
         */
        Mat4 MulM4(const Mat4& A, const Mat4& B);

        // Shader: compila VS/FS.
        // Program: linkea y es lo que realmente usas con glUseProgram.
        Shader shader_;
        Program program_;

        // Cache de locations de uniforms.
        // Esto ahorra llamar glGetUniformLocation cada frame.
        struct Uniforms {
            bool initialized = false; // si el program no linkea, Render() no hace nada

            GLint uModel = -1, uView = -1, uProj = -1;
            GLint uCameraPos = -1;

            GLint uMatDiffuse = -1, uMatSpecular = -1, uMatShininess = -1;
            GLint uAmbientColor = -1, uAmbientStrength = -1;

            // uLight.*
            GLint l_type = -1, l_enabled = -1;
            GLint l_pos = -1, l_dir = -1;
            GLint l_diff = -1, l_spec = -1;
            GLint l_specIntensity = -1;
            GLint l_cAtt = -1, l_lAtt = -1, l_qAtt = -1;
            GLint l_cut = -1, l_outerCut = -1;
        } uniforms_;
    };
}

#endif // __GL_PHONG_HPP__