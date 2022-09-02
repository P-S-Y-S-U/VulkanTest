#ifndef VKRENDER_VULKAN_SURFACE_H
#define VKRENDER_VULKAN_SURFACE_H

#include <vulkan/vulkan.hpp>

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
        explicit VulkanSurface( utils::Uptr<vk::SurfaceKHR> pSurfaceHandle );

        utils::Uptr<vk::SurfaceKHR> m_upSurfaceHandle;

        friend class Window;
    };

} // namespace vkrender


#endif 