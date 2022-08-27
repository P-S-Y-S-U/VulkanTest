#ifndef VKRENDER_WINDOW_H
#define VKRENDER_WINDOW_H

#include <GLFW/glfw3.h>
#include <cstdint>
#include <utility>
#include <vector>

#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS Window
	{
	public:
		explicit Window();
		Window(const Window&) = delete;
		Window(Window&&) noexcept = delete;
		~Window();
		
		Window& operator=(const Window&) noexcept = delete;
		Window& operator=(Window&&) noexcept = delete;

		void init();
		void processEvents();
		void destroy();
		bool quit() const;

		std::pair<std::uint32_t, std::uint32_t> getDimensions();
		
		static std::vector<const char*> populateAvailableExtensions();
	private:
		GLFWwindow*		m_pWindow;

		std::uint32_t	m_windowWidth;
		std::uint32_t	m_windowHeight;

		bool m_bQuit;
	};

} // namespace vkrender

#endif

