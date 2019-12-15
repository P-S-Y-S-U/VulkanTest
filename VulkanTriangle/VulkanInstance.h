#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>

#include "utilities.h"

namespace app
{
	class VulkanInstance
	{
	public:
		VulkanInstance(const std::string& app_name);
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) noexcept = delete;
		~VulkanInstance() = default;

		VulkanInstance& operator=(const VulkanInstance&) noexcept = delete;
		VulkanInstance& operator=(VulkanInstance&&) noexcept = delete;

		void createInstance();
		void destroyInstance();
		
	private:
		VkInstance								_instance;
		utils::Uptr<VkInstanceCreateInfo>		_info;

		void init(utils::Uptr<VkApplicationInfo>);
		void validate_glfw_extensions(const char** glfw_extensions, const std::uint32_t& extension_count);
	};
} // namespace app

