#ifndef VKRENDER_WINDOW_H
#define VKRENDER_WINDOW_H

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#include <cstdint>
#include <utility>
#include <vector>

#include "vkrenderer/VulkanSurface.h"
#include "vkrenderer/VulkanInstance.h"
#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS Window
	{
	public:
		explicit Window( const std::uint32_t& width, const std::uint32_t& height );
		explicit Window();
		Window(const Window&) = delete;
		Window(Window&&) noexcept = delete;
		~Window();
		
		Window& operator=(const Window&) noexcept = delete;
		Window& operator=(Window&&) noexcept = delete;

		void init();
		utils::Uptr<VulkanSurface> createSurface( VulkanInstance* pInstance );
		void processEvents();
		void destroy();
		bool quit() const;

		std::pair<std::uint32_t, std::uint32_t> getDimensions() const ;
		std::pair<std::uint32_t, std::uint32_t> getFrameBufferSize() const ;
		
		static std::vector<const char*> populateAvailableExtensions();
	private:
		GLFWwindow*		m_pWindow;

		std::uint32_t	m_windowWidth;
		std::uint32_t	m_windowHeight;

		bool m_bQuit;
	};

} // namespace vkrender

#endif

