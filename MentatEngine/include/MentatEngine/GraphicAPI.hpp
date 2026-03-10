/**
 *
 * @brief Renderer base class.
 * @author Ferran Barba | Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __GRAPHIC_API_HPP__
#define __GRAPHIC_API_HPP__ 1

#include "../deps/imgui/imgui.h"
#include "../deps/imgui/imgui_impl_glfw.h"

struct GLFWwindow; // <-- FORWARD DECLARATION

namespace ME {
    enum class APIType {
        OpenGL,
        Vulkan
    };

	enum RenderType {
		Phong,
		ShadowMap,
		Translucent
	};

	class GraphicAPI {
		public:

            virtual ~GraphicAPI() = default;

            /**
             * @brief Initializes the graphics API context.
             * @param window Pointer to the GLFW window handle.
             * @param usingImGui Flag to determine if the ImGui environment should be set up.
             */
            virtual void Init(GLFWwindow* window, bool usingImGui) = 0;

            /**
             * @brief Prepares and starts internal rendering systems or shaders.
             */
            virtual void StartRenderers() = 0;

            /**
             * @brief Executes the main draw call for the current frame.
             * @param dt Delta time since the last frame.
             */
            virtual void Draw(float dt) = 0;

            /**
             * @brief Cleans up all graphics resources and contexts.
             */
            virtual void Cleanup() = 0;

            /**
             * @brief Sets the current rendering mode (e.g., Forward, Deferred).
             * @param rt The desired RenderType.
             */
            void SetRenderType(RenderType rt) {
                render_type_ = rt;
            }

        protected:
            /**
             * @brief Internally initializes the ImGui backend for the specific API.
             * @param window Pointer to the GLFW window.
             */
            virtual void InitImGuiEnvironment(GLFWwindow* window) = 0;

            /**
             * @brief Handles the ImGui frame creation and dispatching.
             */
            virtual void ImGuiFrame() = 0;

            /**
             * @brief Shuts down the ImGui backend and releases its resources.
             */
            virtual void ImGuiShutdown() = 0;

            bool usingImGui_ = false;
            RenderType render_type_;
	};
}

#endif // __GRAPHIC_API_HPP__