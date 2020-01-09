#ifndef APP_VULKAN_EXTENSIONS_HPP
#define APP_VULKAN_EXTENSIONS_HPP

#include <initializer_list>
#include <vector>

namespace app
{
	struct VulkanLayer
	{
	public:
		enum class name { VK_KHR_VALIDATION };
		using Layer = const char*;
		using LayerContainer = std::vector<Layer>;
		VulkanLayer(const std::initializer_list<name>& layer_names)
		{
			append(layer_names);
		}
		~VulkanLayer() = default;	

		LayerContainer layers;
	private:
		void append(const std::initializer_list<name>& layer_names)
		{
			for (const auto& layer : layer_names)
			{
				switch (layer)
				{
				case name::VK_KHR_VALIDATION:
					layers.push_back("VK_LAYER_KHRONOS_validation");
					break;
				default:
					break;
				}
			}
			layers.shrink_to_fit();
		}
	};

	namespace layer
	{
	const auto validation_layer = VulkanLayer{ VulkanLayer::name::VK_KHR_VALIDATION };
	}
} // namespace app

#endif // !APP_VULKAN_EXTENSIONS_HPP