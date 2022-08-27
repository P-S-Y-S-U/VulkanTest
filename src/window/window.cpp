#include "window/window.h"

namespace vkrender
{
	Window::Window()
		:m_pWindow{ nullptr }
		,m_windowWidth{ 800 }
		,m_windowHeight{ 600 }
	{
	}

	Window::~Window()
	{
		destroy();
	}

	void Window::init()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		m_pWindow = glfwCreateWindow(m_windowWidth, m_windowHeight, "Vulkan", nullptr, nullptr );
	}

	void Window::loop()
	{
		while ( !glfwWindowShouldClose(m_pWindow) )
		{
			glfwPollEvents();
		}
	}

	void Window::destroy()
	{
		if( m_pWindow )
		{
			glfwDestroyWindow(m_pWindow);
		}
		glfwTerminate();
	}
} // namespace vkrender