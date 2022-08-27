#ifndef VKRENDER_WINDOW_H
#define VKRENDER_WINDOW_H

#include <GLFW/glfw3.h>
#include <cstdint>
#include <utility>

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
		void loop();
		void destroy();
		
		std::pair<std::uint32_t, std::uint32_t> getDimensions() const;
		
	private:
		GLFWwindow*		m_pWindow;

		std::uint32_t	m_windowWidth;
		std::uint32_t	m_windowHeight;
	};

} // namespace vkrender

#endif

