#ifndef VKRENDER_VULKAN_EXTENSIONS_HPP
#define VKRENDER_VULKAN_EXTENSIONS_HPP

#include <initializer_list>
#include <vector>

namespace vkrender
{
	struct VulkanLayer
	{
	public:
		enum class name { VK_KHR_VALIDATION };
		
		using Layer = const char*;
		using LayerContainer = std::vector<Layer>;
		
		explicit VulkanLayer(const std::initializer_list<name>& layer_names)
		{
			append(layer_names);
		}
		~VulkanLayer() = default;	

		LayerContainer m_layers;
	private:
		void append(const std::initializer_list<name>& layer_names)
		{
			for (const auto& layer : layer_names)
			{
				switch (layer)
				{
				case name::VK_KHR_VALIDATION:
					m_layers.push_back("VK_LAYER_KHRONOS_validation");
					break;
				default:
					break;
				}
			}
			m_layers.shrink_to_fit();
		}
	};

	namespace layer
	{
		const VulkanLayer VALIDATION_LAYER{ VulkanLayer::name::VK_KHR_VALIDATION };
	}
} // namespace vkrender

#endif