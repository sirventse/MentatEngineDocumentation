/**
 *
 * @brief OpenGL Renderer.
 * @author Ferran Barba, Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>
#include "../Include/MentatEngine/ImGuiManager.hpp"
#include <../include/MentatEngine/OpenGL/OpenGLAPI.hpp>
#include <../include/MentatEngine/OpenGL/RenderizableComponent.hpp>
#include <../include/MentatEngine/TagComponent.hpp>
#include <../include/MentatEngine/TransformComponent.hpp>
#include <../include/MentatEngine/ScriptComponent.hpp>
#include <../include/MentatEngine/CameraComponent.hpp>
#include <../include/MentatEngine/LightComponent.hpp>

namespace ME {
    void OpenGLAPI::Init(GLFWwindow* window, bool usingImGui) {
        glfwMakeContextCurrent(window);
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            fprintf(stderr, "GLEW Initialization error: %s\n", glewGetErrorString(err));
        }
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        usingImGui_ = usingImGui;
        InitImGuiEnvironment(window);

        // ECS Component initialization
        ME::ECS::AddComponentList<ME::TagComponent>();
        ME::ECS::AddComponentList<ME::RenderizableComponent>();
        ME::ECS::AddComponentList<ME::TransformComponent>();
        ME::ECS::AddComponentList<ME::ScriptComponent>();
        ME::ECS::AddComponentList<ME::CameraComponent>();
        ME::ECS::AddComponentList<ME::LightComponent>();

        // ECS Index & bool initialization
        ME::ECS::current_index = 1;
        ME::ECS::is_dirty = false;
    }

    void OpenGLAPI::StartRenderers()
    {
		phong = std::make_unique<glPhong>();
		shadowmap = std::make_unique<glShadowMap>();
    }

    void OpenGLAPI::Draw(float dt)
    {
        if (ME::Core::InScenesPage) {

        }
        else {
            switch (render_type_)
            {
                case ME::RenderType::Phong:
                    phong->Render(dt); break;
                case ME::RenderType::ShadowMap:
                    shadowmap->Render(dt); break;
                case ME::RenderType::Translucent:
                    translucent->Render(dt); break;
            }
        }

        ImGuiFrame();
    }

    void OpenGLAPI::InitImGuiEnvironment(GLFWwindow* window) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = "../tempFiles/imgui.ini";
        ImGui::StyleColorsDark();
        

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }

    void OpenGLAPI::ImGuiFrame() {
        if (usingImGui_) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGuiManager::DrawRegisteredWindows();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }

    void OpenGLAPI::ImGuiShutdown() {
        if (usingImGui_) {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    }

    void OpenGLAPI::Cleanup() {
        ImGuiShutdown();
    }
}

