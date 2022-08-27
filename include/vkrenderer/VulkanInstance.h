#ifndef VKRENDER_INSTANCE_H
#define VKRENDER_INSTANCE_H

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>

#include "utilities/memory.hpp"
#include "exports.hpp"

namespace vkrender
{
	class VULKAN_EXPORTS VulkanInstance
	{
	public:
		VulkanInstance(const std::string& applicationName);
		VulkanInstance(const VulkanInstance&) = delete;
		VulkanInstance(VulkanInstance&&) noexcept = delete;
		~VulkanInstance();

		VulkanInstance& operator=(const VulkanInstance&) noexcept = delete;
		VulkanInstance& operator=(VulkanInstance&&) noexcept = delete;
		
		void init( const utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>& pDebugMessengerCreateInfo = nullptr );
		void createInstance();
		void destroyInstance();
		
#ifdef NDEBUG
		static constexpr bool ENABLE_VALIDATION_LAYER = false;
#else
		static constexpr bool ENABLE_VALIDATION_LAYER = true;
#endif // NDEBUG
	private:
		std::string		m_applicationName;
		vk::Instance	m_instance;
		std::vector<const char*>	m_extensionContainer;

		utils::Uptr<vk::InstanceCreateInfo>					m_upInstanceCreateInfo;
		utils::Uptr<vk::ApplicationInfo>					m_upApplicationInfo;
		utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>	m_spDebugMessengerCreateInfo;

		void populateApplicationInfo();
		void populateInstanceCreateInfo();

		void validateWindowExtensions();
		bool checkValidationLayerSupport();

		friend class VulkanDebugMessenger;
		friend class VulkanPhysicalDevice;
	};
} // namespace vkrender

#endif

