#ifndef VULKAN_APPLICATION_H
#define VULKAN_APPLICATION_H

#include <string>
#include <vector>
#include <filesystem>

#include "window/window.h"
#include "vkrenderer/VulkanSwapChainStructs.hpp"
#include "vkrenderer/VulkanQueueFamily.hpp"

#include <vulkan/vulkan.hpp>

class VulkanApplication
{
public:
    VulkanApplication( const std::string& applicationName );
    virtual ~VulkanApplication();

#ifdef NDEBUG
		static constexpr bool ENABLE_VALIDATION_LAYER = false;
#else
		static constexpr bool ENABLE_VALIDATION_LAYER = true;
#endif // NDEBUG

    void initialise();
protected:
    virtual void run() = 0;

    void initWindow();
    void initVulkan();
    void shutdown();

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createGraphicsPipeline();

    vk::ShaderModule createShaderModule(const std::vector<char>& shaderSourceBuffer);
    void populateShaderBufferFromSourceFile( const std::filesystem::path& filePath, std::vector<char>& shaderSourceBuffer );

    vkrender::QueueFamilyIndices findQueueFamilyIndices( const vk::PhysicalDevice& physicalDevice, vk::SurfaceKHR* pVkSurface );

    void populateDebugUtilsMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& createInfo );
    void populateDeviceQueueCreateInfo( 
        vk::DeviceQueueCreateInfo& vkDeviceQueueCreateInfo, 
        const std::uint32_t& queueFamilyIndex, const std::vector<float>& queuePriorities 
    );
    void populateDeviceCreateInfo(
        vk::DeviceCreateInfo& vkDeviceCreateInfo,
        const std::vector<vk::DeviceQueueCreateInfo>& queueCreateInfos,
        const vk::PhysicalDeviceFeatures* pEnabledFeatures 
    );
    
    vkrender::SwapChainSupportDetails querySwapChainSupport( const vk::PhysicalDevice& vkPhysicalDevice, const vk::SurfaceKHR& vkSurface );
    
    void setupDebugMessenger();

    bool checkValidationLayerSupport();
    
    std::string m_applicationName;

    vk::Instance m_vkInstance;
    vk::DebugUtilsMessengerEXT m_vkDebugUtilsMessenger;
    vk::SurfaceKHR m_vkSurface;
    vk::PhysicalDevice m_vkPhysicalDevice;

    vk::Device m_vkLogicalDevice;
    vk::Queue m_vkGraphicsQueue;
    vk::Queue m_vkPresentationQueue;

    vk::Format m_vkSwapchainImageFormat;
    vk::Extent2D m_vkSwapchainExtent;
    vk::SwapchainKHR m_vkSwapchain;
    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::ImageView> m_swapchainImageViews;

    std::vector<const char*> m_instanceExtensionContainer;
    std::vector<const char*> m_deviceExtensionContainer;
    vkrender::Window m_window;
};

#endif 