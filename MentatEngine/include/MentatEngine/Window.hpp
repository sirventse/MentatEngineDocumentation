/**
 *
 * @brief Window opener with GLFW.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#ifndef __WINDOW_HPP__
#define __WINDOW_HPP__ 1

#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <optional>
#include <../include/MentatEngine/GraphicAPI.hpp>

namespace ME {
	class Window {
		public:
			~Window();

			Window(const Window&) = delete;
			Window& operator=(const Window&) = delete;

			Window(Window&& other_);
			Window& operator=(Window&&);

			/**
			 * @brief Returns a pointer to the current window.
			 * @return A GLFWwindow* to the window.
			 */
			GLFWwindow* getGLFWwindow() const { return window_; }

			/**
			 * @brief Get the width of the window.
			 * @return window width.
			 */
			int width() const;

			/**
			 * @brief Get the height of the window.
			 * @return window height.
			 */
			int height() const;

			/**
			 * @brief Checks if the current window is open or not.
			 * @return result of the check.
			 */
			bool isWindowOpen() const;

			/**
			 * @brief Swaps the glfw buffers of the current window.
			 */
			void swapBuffers();

			/**
			 * @brief Polls the events of the current window.
			 */
			void pollEvents();

			/**
			 * @brief Creates a window based on the params.
			 * @param int width of the window.
			 * @param int height of the height.
			 * @param const char* title of the window.
			 * @param APIType chosen api bewteen OpenGL and Vulkan.
			 * @return std::optional<Window> optional reference to the created window.
			 */
			static std::optional<Window> create(int width, int height, const char* title, APIType api);

		protected:

		private:
			Window(GLFWwindow* w) : window_ {w} {}
			GLFWwindow* window_;
			
	};
}


#endif //__WINDOW_HPP__
