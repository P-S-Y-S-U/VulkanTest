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

	utils::Uptr<VulkanSurface> Window::createSurface( VulkanInstance* pInstance )
	{
		utils::Uptr<vk::SurfaceKHR> upSurfaceHandle = std::make_unique<vk::SurfaceKHR>();
		if( glfwCreateWindowSurface( 
			pInstance->m_instance, 
			m_pWindow, 
			nullptr, 
			reinterpret_cast<vk::SurfaceKHR::NativeType*>( upSurfaceHandle.get() )
			)
		)
		{
			throw std::runtime_error("Failed to create Window Surface!");
		}

		VulkanSurface* pVulkanSurface = new VulkanSurface(std::move(upSurfaceHandle));
		utils::Uptr<VulkanSurface> upSurfaceWrapper{ pVulkanSurface };

		return std::move(upSurfaceWrapper);
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

		return extensionContainer;
	}
} // namespace vkrender