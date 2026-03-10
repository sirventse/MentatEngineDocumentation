/**
 *
 * @brief Window opener with GLFW.
 * @author Ferran Barba, ESAT 2025-2026
 * @subject Engine Programming
 *
 */

#include <GLFW/glfw3.h>
#include <../include/MentatEngine/Window.hpp>

namespace ME {
	std::optional<Window> Window::create(int width, int height, const char* title, APIType api) {
		if (!glfwInit())
		{
			return std::nullopt;
		}

		switch (api) {
			case APIType::OpenGL:
				glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
				glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
				glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
				break;
			case APIType::Vulkan:
				glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
				break;
		}

		GLFWwindow* gw = glfwCreateWindow(width, height, title, NULL, NULL);
		if(gw == nullptr) return std::nullopt;

		return std::make_optional(Window(gw));
	}

	int Window::width() const {
		int width_;
		glfwGetWindowSize(window_, &width_, NULL);
		return width_;
	}

	int Window::height() const {
		int height_;
		glfwGetWindowSize(window_, NULL, &height_);
		return height_;
	}

	Window::~Window() {
		if (window_) {
			glfwDestroyWindow(window_);
			glfwTerminate();
		}
	}

	Window::Window(Window&& other_) : window_ {other_.window_} {
		other_.window_ = nullptr;
	}

	Window& Window::operator=(Window&& other_) {
		window_ = other_.window_;
		other_.window_ = nullptr;
		return *this;
	}

	bool Window::isWindowOpen() const {
		return !glfwWindowShouldClose(window_);
	}

	void Window::swapBuffers() {
		glfwSwapBuffers(window_);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}

