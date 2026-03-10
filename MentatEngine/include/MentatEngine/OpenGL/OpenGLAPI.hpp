/**
 *
 * @brief Buffer class.
 * @author Ferran Barba | Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __OPEN_GL_API_HPP__
#define __OPEN_GL_API_HPP__ 1

#include <../include/MentatEngine/GraphicAPI.hpp>
#include "../deps/imgui/imgui_impl_opengl3.h"
#include <../include/MentatEngine/Entity.hpp>
#include <../include/MentatEngine/TransformComponent.hpp>
#include <../include/MentatEngine/Iterator.hpp>
#include <../include/MentatEngine/OpenGL/Renderers/glPhong.hpp>
#include <../include/MentatEngine/OpenGL/Renderers/glShadowMap.hpp>
#include <../deps/KFS/include/mat4.h>
#include <vector>

namespace ME {
    class OpenGLAPI : public GraphicAPI {
        public:
            /**
             * @brief Default constructor for the OpenGL implementation.
             */
            OpenGLAPI() = default;

            /**
             * @brief Initializes the OpenGL context, loaders (GLAD), and global states.
             * @param window Pointer to the GLFW window handle.
             * @param usingImGui Flag to determine if the ImGui environment should be set up.
             */
            void Init(GLFWwindow* window, bool usingImGui) override;

            /**
             * @brief Prepares OpenGL specific renderers, shaders, and global buffers.
             */
            void StartRenderers() override;

            /**
             * @brief Executes the OpenGL draw calls, clearing buffers and swapping frames.
             * @param dt Delta time since the last frame.
             */
            void Draw(float dt) override;

            /**
             * @brief Releases all OpenGL-specific resources and contexts.
             */
            void Cleanup() override;

        protected:
            /**
             * @brief Initializes the ImGui backend specifically for OpenGL and GLFW.
             * @param window Pointer to the GLFW window.
             */
            void InitImGuiEnvironment(GLFWwindow* window) override;

            /**
             * @brief Handles the creation of a new ImGui frame for OpenGL.
             */
            void ImGuiFrame() override;

            /**
             * @brief Shuts down the ImGui OpenGL backend and releases resources.
             */
            void ImGuiShutdown() override;


            std::unique_ptr<glPhong> phong;
            std::unique_ptr<glShadowMap> shadowmap;
            std::unique_ptr<glPhong> translucent; // TODO

            /*
            template<typename T>
            void SetIterators(ME::Iterator<T>& it, ME::Iterator<T>& end, T*& first) {
                ME::ContainerFacade<T> container{ ME::ECS::GetComponentList<T>() };

                if (!container.isEmpty()) {
                    it = container.begin();
                    it.SetEmpty(false);
                    first = *it;
                    end = container.end();
                    end.SetEmpty(false);
                }
                else {
                    it.SetEmpty(true);
                    end.SetEmpty(true);
                }
            }
            */
    };
}

#endif // __OPEN_GL_API_HPP__