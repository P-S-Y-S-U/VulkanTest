#ifndef VULKAN_APPLICATION_H
#define VULKAN_APPLICATION_H

#include <string>
#include <array>
#include <vector>
#include <filesystem>

#include "window/window.h"
#include "vkrenderer/VulkanSwapChainStructs.hpp"
#include "vkrenderer/VulkanQueueFamily.hpp"
#include "graphics/Vertex.hpp"
#include "exports.hpp"

#include <vulkan/vulkan.hpp>

class VULKAN_EXPORTS VulkanApplication
{
public:
    using VertexData = std::vector<vertex>;
    using IndexData = std::vector<std::uint16_t>;

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
    void mainLoop();
    void drawFrame();
    void shutdown();

    void createInstance();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain();
    void createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFrameBuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createVertexBuffer();
    void createIndexBuffer();
    void createSyncObjects();
    void recreateSwapChain();
    void destroySwapChain();

    void recordCommandBuffer( vk::CommandBuffer& vkCommandBuffer, const std::uint32_t& imageIndex );

    vk::ShaderModule createShaderModule(const std::vector<char>& shaderSourceBuffer);
    void createBuffer(
        const vk::DeviceSize& bufferSizeInBytes,
        const vk::BufferUsageFlags& bufferUsage,
        const vk::MemoryPropertyFlags& memProps,
        vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory
    );
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

    vk::ResultValue<std::uint32_t> swapchainNextImageWrapper(
        const vk::Device& logicalDevice,
        const vk::SwapchainKHR& swapchain,
        std::uint64_t timeout,
        vk::Semaphore imageAcquireSemaphore,
        vk::Fence imageAcquireFence
    );

    vk::Result queuePresentWrapper(
        const vk::Queue& presentationQueue,
        const vk::PresentInfoKHR& presentInfo
    );
    
    std::uint32_t findMemoryType( const std::uint32_t& typeFilter, const vk::MemoryPropertyFlags& propertyFlags );
    void copyBuffer( const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, const vk::DeviceSize& sizeInBytes );

    static constexpr std::uint8_t MAX_FRAMES_IN_FLIGHT = 2;

    std::string m_applicationName;

    vk::Instance m_vkInstance;
    vk::DebugUtilsMessengerEXT m_vkDebugUtilsMessenger;
    vk::SurfaceKHR m_vkSurface;
    vk::PhysicalDevice m_vkPhysicalDevice;

    vk::Device m_vkLogicalDevice;
    vk::Queue m_vkGraphicsQueue;
    vk::Queue m_vkPresentationQueue;
    vk::Queue m_vkTransferQueue;

    vk::Format m_vkSwapchainImageFormat;
    vk::Extent2D m_vkSwapchainExtent;
    vk::SwapchainKHR m_vkSwapchain;
    std::vector<vk::Image> m_swapchainImages;
    std::vector<vk::ImageView> m_swapchainImageViews;

    vk::RenderPass m_vkRenderPass;
    vk::PipelineLayout m_vkPipelineLayout;
    vk::Pipeline m_vkGraphicsPipeline;

    std::vector<vk::Framebuffer> m_swapchainFrameBuffers;

    vk::CommandPool m_vkGraphicsCommandPool;
    std::vector<vk::CommandBuffer> m_vkGraphicsCommandBuffers;
    vk::Buffer m_vkVertexBuffer;
    vk::DeviceMemory m_vkVertexBufferMemory;
    vk::Buffer m_vkIndexBuffer;
    vk::DeviceMemory m_vkIndexBufferMemory;
    
    std::vector<vk::Semaphore> m_vkImageAvailableSemaphores;
    std::vector<vk::Semaphore> m_vkRenderFinishedSemaphores;
    std::vector<vk::Fence> m_vkInFlightFences;
    std::uint8_t m_currentFrame;

    std::vector<const char*> m_instanceExtensionContainer;
    std::vector<const char*> m_deviceExtensionContainer;
    vkrender::Window m_window;

    VertexData m_inputVertexData;
    IndexData m_inputIndexData;
};

#endif 