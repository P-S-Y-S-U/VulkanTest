#ifndef VKRENDER_VULKAN_SWAP_CHAIN_SUPPORT_HPP
#define VKRENDER_VULKAN_SWAP_CHAIN_SUPPORT_HPP

#include <vulkan/vulkan.hpp>

namespace vkrender
{
    struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR			capabilities;
		std::vector<vk::SurfaceFormatKHR>	surfaceFormats;
		std::vector<vk::PresentModeKHR>		presentModes;
	};
} // namespace vkrender


#endif