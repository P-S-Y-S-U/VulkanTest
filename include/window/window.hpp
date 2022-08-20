#ifndef APP_WINDOW_HPP
#define APP_WINDOW_HPP

#include <GLFW/glfw3.h>
#include "exports.hpp"

namespace app
{
	class VULKAN_EXPORTS Window
	{
	public:
		explicit Window();
		Window(const Window&) = delete;
		Window(Window&&) noexcept = delete;
		~Window() = default;
		
		Window& operator=(const Window&) noexcept = delete;
		Window& operator=(Window&&) noexcept = delete;

		void init();
		void loop();
		void destroy();

		const int	window_width;
		const int	window_height;
	private:
		GLFWwindow* _window;
	};

} // namespace app

#endif

