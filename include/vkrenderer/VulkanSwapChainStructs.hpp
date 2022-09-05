#ifndef VKRENDER_VULKAN_SWAP_CHAIN_STRUCTS_HPP
#define VKRENDER_VULKAN_SWAP_CHAIN_STRUCTS_HPP

#include <vulkan/vulkan.hpp>

namespace vkrender
{
    struct SwapChainSupportDetails
	{
		vk::SurfaceCapabilitiesKHR			capabilities;
		std::vector<vk::SurfaceFormatKHR>	surfaceFormats;
		std::vector<vk::PresentModeKHR>		presentModes;
	};

	struct SwapChainPreset
	{

	};
} // namespace vkrender


#endif