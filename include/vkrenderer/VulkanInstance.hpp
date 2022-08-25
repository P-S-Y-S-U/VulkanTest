#ifndef APP_VULKAN_INSTANCE_HPP
#define APP_VULKAN_INSTANCE_HPP

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include "utilities/memory.hpp"
#include "exports.hpp"

namespace app
{
	namespace debug
	{
		class VulkanDebugMessenger;
	} // namespace app::debug;

	class VULKAN_EXPORTS VulkanInstance
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
		
		void createInstance();
		void destroyInstance();
		
		//const ExtensionContainer validation_layers = ExtensionContainer{ "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
		const bool enable_validation_layer = false;
#else
		const bool enable_validation_layer = true;
#endif // NDEBUG
	private:
		vk::Instance											_instance;
		utils::Uptr<vk::InstanceCreateInfo>					_info;
		utils::Uptr<vk::ApplicationInfo>						_app_info;
		ExtensionContainer									_extensions;

		utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>								_debug_create_info;

		friend class debug::VulkanDebugMessenger;
		friend class VulkanPhysicalDevice;
		
		void setup_application_info(const std::string&);
		void init();

		void validate_glfw_extensions(ExtensionContainer&);
		bool check_validation_layer_support();
		auto get_extensions()->ExtensionContainer;
	};
} // namespace app

#endif

