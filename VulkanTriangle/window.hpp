#pragma once
#include <GLFW/glfw3.h>

namespace app
{
	class Window
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

