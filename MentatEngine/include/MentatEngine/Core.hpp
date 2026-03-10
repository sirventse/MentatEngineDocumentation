/**
 *
 * @brief Engine core class.
 * @author Sergi Sirvent, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __CORE_HPP__
#define __CORE_HPP__ 1

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <../include/MentatEngine/Window.hpp>
#include <../include/MentatEngine/Entity.hpp>
#include <../include/MentatEngine/GraphicAPI.hpp>
#include <../include/MentatEngine/JobSystem.hpp>

namespace ME {
	class Core {
        public:
            Core(const Core& right) = delete;
            Core& operator=(const Core& right) = delete;

            Core(Core&& other) noexcept 
                : window_{ std::move(other.window_) }, 
                    api_type_{ other.api_type_ },
                    api_{ std::move(other.api_) },
                    job_system_{ std::move(other.job_system_) },
                    lastFrame_{ other.lastFrame_ },
                    currentFrame_{other.currentFrame_},
                    deltaTime_{other.deltaTime_}
            {};
            Core& operator=(Core&& other);
            ~Core();

            /**
             * @brief Initializes the rendering system and optional UI context.
             * @param useImGui True to enable the ImGui environment.
             */
            void InitRenderer(bool useImGui);

            /**
             * @brief Begins a new frame.
             * Processes input events and prepares the graphics API for drawing.
             */
            void StartFrame();

            /**
             * @brief Checks if the main application window is still active.
             * @return True if the window has not been requested to close.
             */
            bool IsWindowOpen() const;

            /**
             * @brief Finalizes the current frame.
             * Swaps buffers and updates the pending resources from job system threads to complete the frame cycle.
             */
            void EndFrame();

            /**
             * @brief Provides access to the underlying Graphics API.
             * @return A unique_ptr reference to the current GraphicAPI implementation.
             */
            std::unique_ptr<GraphicAPI>& GetGraphicAPI();

            /**
             * @brief Identifies the active Graphics API type (OpenGL or Vulkan).
             * @return The APIType enum value.
             */
            APIType GetGraphicAPIType();

            /**
             * @brief Gets a raw pointer to the internal JobSystem.
             * @return A pointer to the JobSystem for task scheduling.
             */
            JobSystem* GetJobSystemRef() { return job_system_.get(); };

            /**
             * @brief Shuts down the engine and releases all allocated resources.
             */
            void CleanUp();

            /** 
             * @brief Returns the timestamp of the previous frame.
             */
            float LastFrame() const;

            /** 
             * @brief Returns the time elapsed between the current and last frame.
             */
            float DeltaTime() const;

            /** 
             * @brief Returns the current total execution time in seconds.
             */
            float CurrentFrame() const;

            /**
             * @brief Gets the main window instance.
             * @return A reference to the Window object.
             */
            Window& window();

            /**
             * @brief Static factory method to initialize the Core engine safely.
             *
             * Creates the window and initializes basic systems. Returns an empty optional
             * if the creation fails (e.g., window context couldn't be created).
             *
             * @param width The initial window width.
             * @param height The initial window height.
             * @param title The window title text.
             * @param api The desired graphics API to use.
             * @return An std::optional containing the Core instance if successful.
             */
            static std::optional<Core> create(int width, int height, const char* title, APIType api);

            static bool InScenesPage;
            static std::string CurrentSceneName;
        private:
            Core(Window&& window, APIType apit, std::unique_ptr<GraphicAPI> api) : window_{ std::move(window) },
                api_type_{ apit },
                api_{ std::move(api) },
                job_system_( std::make_unique<JobSystem>(std::thread::hardware_concurrency()) )
            {
                lastFrame_ = 0.0f;
                deltaTime_ = 0.0f;
                currentFrame_ = 0.0f;
                //glfwSwapInterval(0); quitar vsync, descapar FPS?
            };
            Window window_;
            APIType api_type_;
            std::unique_ptr<JobSystem> job_system_;
            std::unique_ptr<GraphicAPI> api_;
            float lastFrame_;
            float deltaTime_;
            float currentFrame_;
	};
}

#endif // __CORE_HPP__