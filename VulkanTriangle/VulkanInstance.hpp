#ifndef APP_VULKAN_INSTANCE
#define APP_VULKAN_INSTANCE

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "utilities.hpp"
#include "VulkanDebugMessenger.hpp"

namespace app
{
	class VulkanInstance
	{
	public:
		using ExtensionName = const char*;
		using ExtensionContainer = std::vector<ExtensionName>;
		VulkanInstance(const std::string&);
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) noexcept = delete;
		~VulkanInstance() = default;

		VulkanInstance& operator=(const VulkanInstance&) noexcept = delete;
		VulkanInstance& operator=(VulkanInstance&&) noexcept = delete;
		
		VkInstance* get_instance();
		const VkInstance* get_instance() const;
		void createInstance();
		void destroyInstance();
		
		const ExtensionContainer validation_layers = ExtensionContainer{ "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
		const bool enable_validation_layer = false;
#else
		const bool enable_validation_layer = true;
#endif // NDEBUG
		friend class debug::VulkanDebugMessenger;
	private:
		VkInstance											_instance;
		utils::Uptr<VkInstanceCreateInfo>					_info;
		utils::Uptr<VkApplicationInfo>						_app_info;
		ExtensionContainer									_extensions;

		debug::DebugMsgInfoPtr								_debug_create_info;

		void setup_application_info(const std::string&);
		void init();

		void validate_glfw_extensions(ExtensionContainer&);
		bool check_validation_layer_support();
		auto get_extensions()->ExtensionContainer;
	};
} // namespace app

#endif

