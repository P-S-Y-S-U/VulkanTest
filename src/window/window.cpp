#include "window/window.hpp"

namespace app
{
	Window::Window()
		:_window{ nullptr }
		,window_width{ 800 }
		,window_height{ 600 }
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	}

	void Window::init()
	{
		_window = glfwCreateWindow(window_width, window_height, "Vulkan", nullptr, nullptr );
	}

	void Window::loop()
	{
		while ( !glfwWindowShouldClose(_window) )
		{
			glfwPollEvents();
		}
	}

	void Window::destroy()
	{
		glfwDestroyWindow(_window);
		glfwTerminate();
	}
} // namespace app