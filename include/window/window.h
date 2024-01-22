#ifndef VKRENDER_WINDOW_H
#define VKRENDER_WINDOW_H

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#else
#define VK_USE_KHR_WAYLAND_SURFACE
#endif

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#include <GLFW/glfw3native.h>

#include <cstdint>
#include <utility>
#include <vector>

#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS Window
	{
	public:
#ifdef _WIN32
		using PlatformWindowHandle = HWND;
#else
		using PlatformWindowHandle = wl_surface*;
		using PlatformDisplayHandle = wl_display*;
#endif 
		explicit Window( const std::uint32_t& width, const std::uint32_t& height );
		explicit Window();
		Window(const Window&) = delete;
		Window(Window&&) noexcept = delete;
		~Window();
		
		Window& operator=(const Window&) noexcept = delete;
		Window& operator=(Window&&) noexcept = delete;

		void init();
		// utils::Uptr<VulkanSurface> createSurface( VulkanInstance* pInstance );
		void processEvents();
		void destroy();
		bool quit() const;

		PlatformWindowHandle getHandle() const;
#ifndef _WIN32
		PlatformDisplayHandle getDisplayHandle() const;
#endif

		std::pair<std::uint32_t, std::uint32_t> getDimensions() const ;
		std::pair<std::uint32_t, std::uint32_t> getFrameBufferSize() const ;
		
		// resets to false after returning
		bool isFrameBufferResized();

		static std::vector<const char*> populateAvailableExtensions();

		static void framebufferResizeCallback(GLFWwindow* pWindow, int width, int height);
	private:
		GLFWwindow*		m_pWindow;

		std::uint32_t	m_windowWidth;
		std::uint32_t	m_windowHeight;

		bool m_bQuit;
		bool m_bFrameBufferResized;
	};

} // namespace vkrender

#endif

