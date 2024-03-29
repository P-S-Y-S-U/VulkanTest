#ifndef VKRENDER_VULKAN_LOGICAL_DEVICE_H
#define VKRENDER_VULKAN_LOGICAL_DEVICE_H

#include <vulkan/vulkan.hpp>
#include "vkrenderer/VulkanPhysicalDevice.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include "exports.hpp"

#include <map>

namespace vkrender
{
	class VULKAN_EXPORTS VulkanLogicalDevice
	{
	public:
		explicit VulkanLogicalDevice(VulkanPhysicalDevice* pPhysicalDevice, VulkanSurface* pSurface = nullptr);
		~VulkanLogicalDevice();

		void createLogicalDevice();
		void destroyLogicalDevice();

		vk::Device* getHandle() { return &m_deviceHandle; }
		vk::Queue* getQueue( const std::uint32_t& index ) { return &m_queueIndicesMap.at(index); }
	private:
		vk::Device								m_deviceHandle;

		VulkanPhysicalDevice* m_pPhysicalDevice;
		VulkanSurface* m_pSurface;

		std::vector<vk::DeviceQueueCreateInfo>	m_queueCreateInfos;
		utils::Sptr<vk::DeviceCreateInfo>		m_spDeviceCreateInfo;

		QueueFamilyIndices						m_queueFamilyIndices;
		std::map< std::uint32_t, vk::Queue >	m_queueIndicesMap;
		std::map< std::uint32_t, float>			m_queuePrioritiesMap;

		std::vector<vk::DeviceQueueCreateInfo> populateDeviceQueueCreateInfo();
		void populateDeviceCreateInfo(const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos);

		vk::Queue createDeviceQueue(const std::uint32_t& queueFamilyIndex, const std::uint32_t& index = 0u);

		friend class VulkanLogicalDeviceManager;
		friend class VulkanSwapChain;
		friend class VulkanImageView;
		friend class VulkanShaderModule;
		friend class VulkanPipelineLayout;
		friend class VulkanRenderPass;
		friend class VulkanGraphicsPipeline;
		friend class VulkanFrameBuffer;
		friend class VulkanCommandPool;
	};
} // namespace vkrender

#endif
