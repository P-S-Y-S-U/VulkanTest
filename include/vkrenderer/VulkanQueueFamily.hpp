#ifndef VKRENDER_VULKAN_QUEUE_FAMILY_HPP
#define VKRENDER_VULKAN_QUEUE_FAMILY_HPP

#include "utilities/memory.hpp"
#include "exports.hpp"

#include <optional>

namespace vkrender
{
	struct QueueFamilyIndices
	{
		std::optional<std::uint32_t>	m_graphicsFamily;
		std::optional<std::uint32_t>	m_presentFamily;
	};

} // namespace vkrender

#endif