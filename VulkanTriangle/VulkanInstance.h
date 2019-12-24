#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include "utilities.h"

namespace app
{
	class VulkanInstance
	{
	public:
		using ExtensionName = const char*;
		using ExtensionContainer = std::vector<ExtensionName>;
		VulkanInstance(const std::string& app_name);
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) noexcept = delete;
		~VulkanInstance() = default;

		VulkanInstance& operator=(const VulkanInstance&) noexcept = delete;
		VulkanInstance& operator=(VulkanInstance&&) noexcept = delete;

		void createInstance();
		void destroyInstance();
		
		const ExtensionContainer validation_layers = ExtensionContainer{ "VK_LAYER_KHRONOS_validation" };
#ifdef NDEBUG
		const bool enable_validation_layer = false;
#else
		const bool enable_validation_layer = true;
#endif // NDEBUG

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	private:
		VkInstance											_instance;
		utils::Uptr<VkInstanceCreateInfo>					_info;
		ExtensionContainer									_extensions;
		utils::Uptr<VkDebugUtilsMessengerCreateInfoEXT>		_debugCreate_info;
	
		VkDebugUtilsMessengerEXT							_debugmessenger;
		utils::Uptr<VkDebugUtilsMessengerCreateInfoEXT>		_debugmessenger_info;

		void init(utils::Uptr<VkApplicationInfo>);

		void setup_debug_messenger();
		utils::Uptr<VkDebugUtilsMessengerCreateInfoEXT> populate_debug_messenger_info();
		VkResult CreateDebugUtilsMessengerEXT(const VkAllocationCallbacks*);
		void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks*);

		void validate_glfw_extensions(ExtensionContainer&);
		bool check_validation_layer_support();
		auto get_extensions()->ExtensionContainer;
	};
} // namespace app

