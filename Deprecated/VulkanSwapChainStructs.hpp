#ifndef VKRENDER_VULKAN_SWAP_CHAIN_STRUCTS_HPP
#define VKRENDER_VULKAN_SWAP_CHAIN_STRUCTS_HPP

#include <vulkan/vulkan.hpp>
#include <cstdint>

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
		vk::SurfaceCapabilitiesKHR			capabilities;
		vk::SurfaceFormatKHR	surfaceFormat;
		vk::Extent2D			imageExtent;
		vk::PresentModeKHR		presentMode;
		std::uint32_t			imageCount;	
		vk::SharingMode			sharingMode;
		std::vector<std::uint32_t>	queueFamilyIndices;
	};
} // namespace vkrender


#endif