#include "vkrenderer/VulkanInstance.h"
#include "vkrenderer/VulkanDebugMessenger.hpp"
#include "vkrenderer/VulkanLayer.hpp"
#include "window/window.h"
#include <cstdlib>
#include <vector>
#include <iostream>

namespace vkrender
{
	VulkanInstance::VulkanInstance(const std::string& applicationName)
		:m_applicationName{ applicationName }
	{}
	
	VulkanInstance::~VulkanInstance()
	{
		destroyInstance();
	}

	void VulkanInstance::init( const utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT>& pDebugMessengerCreateInfo )
	{
		if (ENABLE_VALIDATION_LAYER && !checkValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, not available");
		}
		if (ENABLE_VALIDATION_LAYER)
		{
			m_spDebugMessengerCreateInfo = pDebugMessengerCreateInfo;
		}

		populateApplicationInfo();
		populateInstanceCreateInfo();
		validateWindowExtensions();
	}

	void VulkanInstance::createInstance()
	{
		// creating Vulkan Instance
		if (vk::createInstance(m_upInstanceCreateInfo.get(), nullptr, &m_instance) != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create instance!");
		}
		std::cout << "Created Vulkan instance successfully" << std::endl;
	}

	void VulkanInstance::destroyInstance()
	{
		vkDestroyInstance(m_instance, nullptr);
		m_upInstanceCreateInfo.reset();
		m_upApplicationInfo.reset();
		m_spDebugMessengerCreateInfo.reset();
	}

	void VulkanInstance::populateApplicationInfo()
	{
		m_upApplicationInfo = std::make_unique<vk::ApplicationInfo>();
		m_upApplicationInfo->sType = vk::StructureType::eApplicationInfo;
		m_upApplicationInfo->pApplicationName = m_applicationName.c_str();
		m_upApplicationInfo->applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		m_upApplicationInfo->pEngineName = "No Engine";
		m_upApplicationInfo->engineVersion = VK_MAKE_VERSION(1, 0, 0);
		m_upApplicationInfo->apiVersion = VK_API_VERSION_1_3;
		m_upApplicationInfo->pNext = nullptr;
	}

	void VulkanInstance::populateInstanceCreateInfo()
	{
		m_upInstanceCreateInfo = std::make_unique<vk::InstanceCreateInfo>();
		m_upInstanceCreateInfo->sType = vk::StructureType::eInstanceCreateInfo;
		m_upInstanceCreateInfo->pApplicationInfo = m_upApplicationInfo.get();
		
		m_extensionContainer = Window::populateAvailableExtensions();

		if( ENABLE_VALIDATION_LAYER )
		{
			m_extensionContainer.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		
		m_upInstanceCreateInfo->enabledExtensionCount = static_cast<std::uint32_t>(m_extensionContainer.size());
		m_upInstanceCreateInfo->ppEnabledExtensionNames = m_extensionContainer.data();
		
		if (ENABLE_VALIDATION_LAYER)
		{
			m_upInstanceCreateInfo->enabledLayerCount = static_cast<std::uint32_t>(layer::VALIDATION_LAYER.m_layers.size());
			m_upInstanceCreateInfo->ppEnabledLayerNames = layer::VALIDATION_LAYER.m_layers.data();
			m_upInstanceCreateInfo->pNext = m_spDebugMessengerCreateInfo.get();
		}
		else {
			m_upInstanceCreateInfo->enabledLayerCount = 0;
		}
	}

	void VulkanInstance::validateWindowExtensions()
	{
		// Checking for Vulkan Instance extensions
		std::vector<vk::ExtensionProperties> vulkanExtensionProperties = vk::enumerateInstanceExtensionProperties( nullptr );

		for (auto extensionName : m_extensionContainer)
		{
			bool found = false;
			for (const auto& vulkanExtension : vulkanExtensionProperties)
			{
				if (std::strcmp(extensionName, vulkanExtension.extensionName) == 0) {
					std::cout << "\t" <<  extensionName << " Extension found" << std::endl;
					found = true;
				}
			}
			if (found == false)
			{
				throw std::runtime_error("extension not found");
			}
		}
	}

	bool VulkanInstance::checkValidationLayerSupport()
	{
		// Enumerating Insatnce layer properties
		std::vector<vk::LayerProperties> availableLayers = vk::enumerateInstanceLayerProperties();

		// validating Layer support
		for (const auto& layerName : layer::VALIDATION_LAYER.m_layers)
		{
			bool bLayerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					bLayerFound = true;
				}
			}
			if (!bLayerFound) { return false;  }
		}

		return true;
	}

} // namespace appp