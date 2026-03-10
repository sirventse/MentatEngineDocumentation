/**
 *
 * @brief ImGui manager static class to control both render APIs.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __IMGUIMANAGER_HPP__
#define __IMGUIMANAGER_HPP__ 1


#pragma once

#include <functional>
#include <vector>
#include <string>�
#include <../deps/KFS/include/vec3.h>
#include <MentatEngine/Input.hpp>

namespace ME {
    class Input;
    class ImGuiManager {

    public:

        enum BasicShape {
            CUBE,
            SPHERE
        };

        enum class GizmoSpace {
            WORLD,
            LOCAL
        };

        enum LightType {
            AMBIENT,
            DIRECTIONAL,
            SPOT,
            POINT
        };

        enum class GizmoAxis {
            NONE,
            X,
            Y,
            Z
        };

        enum class GizmoMode {
			TRANSLATE,
            ROTATE,
            SCALE
        };

        struct GizmoState {
            bool visible = false;
            GizmoAxis hotAxis = GizmoAxis::NONE;
            GizmoAxis activeAxis = GizmoAxis::NONE;

            ImVec2 originScreen{ 0.0f, 0.0f };
            ImVec2 axisXEnd{ 0.0f, 0.0f };
            ImVec2 axisYEnd{ 0.0f, 0.0f };
            ImVec2 axisZEnd{ 0.0f, 0.0f };

            ImVec2 dragStartMouse{ 0.0f, 0.0f };
            Vec3   dragStartWorldPos{ 0.0f, 0.0f, 0.0f };

            float pixelsPerWorldUnitX = 1.0f;
            float pixelsPerWorldUnitY = 1.0f;
            float pixelsPerWorldUnitZ = 1.0f;

            Vec3 dragStartRotation{ 0.0f, 0.0f, 0.0f };
            Vec3 dragStartScale{ 1.0f, 1.0f, 1.0f };
            ImVec2 dragRingTangent{ 0.0f, 0.0f };
            float rotatePixelsToDegrees = 1.0f;
        };

        static GizmoState gizmo_state_;

        struct BasicShapeInfo {
            BasicShape shape;
            std::string name;
        };
        struct LightSpawnInfo {
            LightType type;
            std::string name;
        };

        ImGuiManager();
        ~ImGuiManager();

        using GuiCallback = std::function<void()>;

        /**
         * @brief Registers a new UI window callback to be rendered.
         * @param cb A GuiCallback function or lambda containing ImGui window logic.
         */
        static void RegisterWindow(GuiCallback cb);

        /**
         * @brief Registers the main engine/editor interface window.
         */
        static void RegisterMainWindow();

        static void RegisterScenesWindow();

        /**
         * @brief Renders all registered UI windows and processes pending ECS component changes.
         *
         * This method iterates through all stored callbacks to draw the UI.
         * Additionally, it handles a "pending update" queue to safely add or remove
         * ECS components (Transform, Renderizable, Light, etc.) outside of the main
         * systems' update loops.
         *
         * @note If a RenderizableComponent is added, it may trigger an asynchronous
         * mesh loading task via the JobSystem.
         */
        static void DrawRegisteredWindows();

        /**
         * @brief Clears the list of registered UI window callbacks.
         */
        static void ClearWindows();

        /**
         * @brief Checks if the engine is currently in an active gameplay state.
         * @return True if the core is playing and not paused.
         */
        static bool IsRuntimeInputMode();

        /**
         * @brief Sets the input handler instance for the UI manager.
         * @param input Pointer to the Input system.
         */
        static void SetInput(Input* input);

        static Input* input_;
        static bool show_editor_bindings_window_;

        static bool show_performance_window_;
        static bool show_outline_window_;
        static bool show_details_window_;
        static bool show_camera_config_window_;
        static bool show_spawning_window_;
        static bool show_explorer_window_;
        static bool show_play_window_;

        static bool core_is_playing_;
        static bool core_is_paused_;
        static bool core_is_start_done_;


        static BasicShapeInfo selectedBasicShapeToSpawn_;
        static std::vector<BasicShapeInfo> basic_shapes_info_;

        static LightSpawnInfo selectedLightToSpawn_;
        static std::vector<LightSpawnInfo> light_spawn_info_;

        static GizmoMode current_gizmo_mode_;
        static bool show_gizmo_window_;

        static float gizmo_translate_sensitivity_;
        static float gizmo_rotate_sensitivity_;
        static float gizmo_scale_sensitivity_;

        static GizmoSpace current_gizmo_space_;

    private:
        inline static std::vector<GuiCallback> s_callbacks_;
    };
}

#endif //IMGUIMANAGER