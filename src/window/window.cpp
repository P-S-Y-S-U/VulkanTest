#include "window/window.h"

namespace vkrender
{
	Window::Window( const std::uint32_t& width, const std::uint32_t& height )
		:m_pWindow{ nullptr }
		,m_windowWidth{ width }
		,m_windowHeight{ height }
		,m_bQuit{ false }
	{}

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

/*
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
		VulkanSurface* pVulkanSurface = new VulkanSurface( pInstance, std::move(upSurfaceHandle));
		utils::Uptr<VulkanSurface> upSurfaceWrapper{ pVulkanSurface };

		return std::move(upSurfaceWrapper);
	}
*/

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

	std::pair<std::uint32_t, std::uint32_t> Window::getDimensions() const
	{
		return std::pair{ m_windowWidth, m_windowHeight };
	}

	std::pair<std::uint32_t, std::uint32_t> Window::getFrameBufferSize() const
	{
		std::int32_t frameBufferWidth = 0;
		std::int32_t frameBufferHeight = 0;

		glfwGetFramebufferSize( m_pWindow, &frameBufferWidth, &frameBufferHeight );

		return std::make_pair<std::uint32_t, std::uint32_t>( static_cast<std::uint32_t>( frameBufferWidth ), static_cast<std::uint32_t>( frameBufferHeight ) );
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