#include "window/window.h"
#include "vkrenderer/VulkanInstance.h"

namespace vkrender
{
	Window::Window()
		:m_pWindow{ nullptr }
		,m_windowWidth{ 800 }
		,m_windowHeight{ 600 }
		,m_bQuit{ false }
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

	void Window::processEvents()
	{
		m_bQuit = glfwWindowShouldClose(m_pWindow);

		if( !m_bQuit )
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

	bool Window::quit() const 
	{
		return m_bQuit;
	}

	std::pair<std::uint32_t, std::uint32_t> Window::getDimensions() 
	{
		return std::pair{ m_windowWidth, m_windowHeight };
	}

	std::vector<const char*> Window::populateAvailableExtensions()
	{
		glfwInit();
		
		std::uint32_t extensionCount = 0;
		const char** extensionNames;
		extensionNames = glfwGetRequiredInstanceExtensions( &extensionCount );

		std::vector<const char*> extensionContainer( extensionNames, extensionNames + extensionCount );

		if( VulkanInstance::ENABLE_VALIDATION_LAYER )
		{
			extensionContainer.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		extensionContainer.shrink_to_fit();

		return extensionContainer;
	}
} // namespace vkrender