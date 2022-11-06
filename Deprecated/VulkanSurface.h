#ifndef VKRENDER_VULKAN_SURFACE_H
#define VKRENDER_VULKAN_SURFACE_H

#include <vulkan/vulkan.hpp>

#include "vkrenderer/VulkanInstance.h"
#include "utilities/memory.hpp"
#include "exports.hpp"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanSurface 
    {
    public:
        ~VulkanSurface();

        void destroySurface();
    private:
        explicit VulkanSurface( VulkanInstance* pInstance, utils::Uptr<vk::SurfaceKHR> pSurfaceHandle );

        utils::Uptr<vk::SurfaceKHR> m_upSurfaceHandle;
        VulkanInstance* m_pInstance;

        friend class VulkanQueueFamily;
        friend class VulkanPhysicalDevice;
        friend class VulkanSwapChainFactory;
        friend class VulkanSwapChain;
        friend class Window;
    };

} // namespace vkrender


#endif 