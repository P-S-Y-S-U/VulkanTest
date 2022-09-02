#include "vkrenderer/VulkanSurface.h"

namespace vkrender
{
    VulkanSurface::VulkanSurface( VulkanInstance* pInstance, utils::Uptr<vk::SurfaceKHR> pSurfaceHandle )
        :m_pInstance{ pInstance }
        ,m_upSurfaceHandle{ std::move(pSurfaceHandle) }
    {}

    VulkanSurface::~VulkanSurface()
    {
        destroySurface();
    }
    
    void VulkanSurface::destroySurface()
    {
        m_pInstance->m_instance.destroySurfaceKHR( *m_upSurfaceHandle.get() );
        m_upSurfaceHandle.reset();
    }
} // namespace vkrender