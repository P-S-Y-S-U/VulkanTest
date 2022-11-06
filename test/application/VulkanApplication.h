#ifndef VULKAN_APPLICATION_H
#define VULKAN_APPLICATION_H

#include <string>
#include <vector>

#include "window/window.h"
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
    
    void populateDebugUtilsMessengerCreateInfo( vk::DebugUtilsMessengerCreateInfoEXT& createInfo );
    void setupDebugMessenger();

    bool checkValidationLayerSupport();
    
    std::string m_applicationName;

    vk::Instance m_vkInstance;
    vk::DebugUtilsMessengerEXT m_vkDebugUtilsMessenger;

    std::vector<const char*> m_extensionContainer;
    vkrender::Window m_window;
};

#endif 