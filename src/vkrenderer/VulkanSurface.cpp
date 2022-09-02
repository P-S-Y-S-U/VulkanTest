#include "vkrenderer/VulkanSurface.h"

namespace vkrender
{
    VulkanSurface::VulkanSurface( utils::Uptr<vk::SurfaceKHR> pSurfaceHandle )
        :m_upSurfaceHandle{ std::move(pSurfaceHandle) }
    {}

    VulkanSurface::~VulkanSurface()
    {
        destroySurface();
    }
    
    void VulkanSurface::destroySurface()
    {
        m_upSurfaceHandle.reset();
    }
} // namespace vkrender