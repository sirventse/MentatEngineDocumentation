/**
 *
 * @brief glShadowMap render system class.
 * @author Sergi, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __GL_SHADOW_MAP__HPP__
#define __GL_SHADOW_MAP__HPP__ 1

#include <../../include/MentatEngine/OpenGL/Shader.hpp>
#include <../../include/MentatEngine/OpenGL/Program.hpp>
#include <../../include/MentatEngine/Iterator.hpp>
#include <../../deps/KFS/include/vec3.h>
#include <../../deps/KFS/include/mat4.h>

namespace ME {
    class glShadowMap {
    public:
        /**
         * @brief Constructor that initializes shadow mapping resources.
         */
        glShadowMap();

        ~glShadowMap();

        glShadowMap(glShadowMap&& other) noexcept;

        glShadowMap& operator=(glShadowMap&& other) noexcept;

        glShadowMap(const glShadowMap&) = delete;

        glShadowMap& operator=(const glShadowMap&) = delete;

        /**
         * @brief Main shadow pass.
         * Renders the scene from the light's perspective to update the shadow map.
         * @param dt Delta time since the last frame.
         */
        void Render(float dt);

        /**
         * @brief Allocates the Framebuffer and Cubemap for omnidirectional point shadows.
         */
        void CreatePointShadowBuffer();

        /**
         * @brief Calculates the six view-projection matrices for a point light cubemap.
         * @param lightPos The world position of the light source.
         * @param nearPlane The near clipping plane for the shadow camera.
         * @param farPlane The far clipping plane for the shadow camera.
         * @return A vector containing the 6 transformation matrices (one per cube face).
         */
        std::vector<Mat4> BuildPointShadowTransforms(const Vec3& lightPos, float nearPlane, float farPlane);


    private:
        // ========= PROGRAMS =========
        Program shadow_pass_;
        Program light_pass_;
		Program point_shadow_pass_;

        Shader shadow_shader_;
        Shader light_shader_;
        Shader point_shadow_shader_;

        // ========= SHADOW TARGET =========
        GLuint shadow_fbo_ = 0;
        GLuint shadow_texture_ = 0;

        int shadow_width_ = 4096;
        int shadow_height_ = 4096;

        // ========= POINT SHADOW TARGET =========
        GLuint point_shadow_fbo_ = 0;
        GLuint point_shadow_cubemap_ = 0;

        int point_shadow_size_ = 1024;

        // ========= uniforms shadow =========
        struct ShadowUniforms {
            GLint uModel = -1;
            GLint uLightViewProj = -1;
        } shadow_u_;

        struct PointShadowUniforms {
            GLint uModel = -1;
            GLint uShadowMatrices[6] = { -1, -1, -1, -1, -1, -1 };
            GLint uLightPos = -1;
            GLint uFarPlane = -1;
        } point_shadow_u_;

        // ========= uniforms light =========
        struct LightUniforms {
            GLint uModel = -1;
            GLint uView = -1;
            GLint uProj = -1;

            GLint uLightPos = -1;
            GLint uLightDir = -1;
            GLint uLightColor = -1;
            GLint uCameraPos = -1;

            GLint uLightType = -1;
            GLint uCastsShadow = -1;

            GLint uConstAtt = -1;
            GLint uLinearAtt = -1;
            GLint uQuadAtt = -1;

            GLint uCutOff = -1;
            GLint uOuterCutOff = -1;

            GLint uLightSpaceMatrix = -1;
            GLint uShadowMap = -1;

            GLint uObjectColor = -1;

            GLint uUseAmbient = -1;
            GLint uAmbientColor = -1;

            GLint uPointShadowMap = -1;
            GLint uPointShadowFarPlane = -1;
        } light_u_;

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

            *first = nullptr;

            if (container.isEmpty()) {
                // deja it y end en un estado consistente
                *it = container.end();
                *end = container.end();
                it->SetEmpty(true);
                end->SetEmpty(true);
                return;
            }

            *it = container.begin();
            *end = container.end();
            it->SetEmpty(false);
            end->SetEmpty(false);

            if (*it != *end) {
                *first = **it;
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
            if (it->IsEmpty() || *it == *end) {
                *first = nullptr;
                return;
            }

            if (it->owner() == current) {
                ++(*it);
                if (*it != *end) {
                    *first = **it;
                }
                else {
                    *first = nullptr;
                }
            }
        }

        /**
         * @brief Recursively updates the world transformation matrices of an entity and its children.
         *
         * This method traverses the scene hierarchy starting from the given ID,
         * ensuring that parent transformations are correctly applied to all descendants.
         *
         * @param id The unique identifier of the root entity to start the update.
         */
        void UpdateTransformRecursive(unsigned long id);

        /**
         * @brief Initializes the framebuffer and texture resources for shadow depth mapping.
         */
        void CreateShadowBuffer(); // inicializar buffer de la sombra

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

        Mat4 MakeOrtho(float left, float right, float bottom, float top, float znear, float zfar);

        /**
         * @brief Performs matrix multiplication between two Mat4 objects.
         * @param A Left matrix.
         * @param B Right matrix.
         * @return The resulting Mat4 product.
         */
        Mat4 MulM4(const Mat4& A, const Mat4& B);

        // ========= DEBUG PASS =========
        Program debug_pass_;
        Shader  debug_shader_;
        GLuint  debugVAO_ = 0;
        GLuint  debugVBO_ = 0;
        GLint   uDepth_ = -1;
        bool    showShadowDebug_ = false;

        // ======= Camera uniforms ========
        GLint uViewProj_;

        /**
         * @brief Initializes the resources and shaders for the shadow map depth pass.
         */
        void InitializeShadowPass();

        /**
         * @brief Initializes the main lighting pass, including uniform caching and light states.
         */
        void InitializeLightPass();

        /**
         * @brief Initializes the debug pass used for visualizing wireframes or normals.
         */
        void InitializeDebugPass();

        /**
         * @brief Initializes the point shadow pass for omnidirectional cubemap depth mapping.
         */
        void InitializePointShadowPass();
    };
}

#endif