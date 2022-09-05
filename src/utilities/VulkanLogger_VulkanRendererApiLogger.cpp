#include "utilities/VulkanLogger.h"

namespace utils
{
    template<> VulkanRendererApiLogger* Singleton<VulkanRendererApiLogger>::m_pSingletonType = nullptr;

    VulkanRendererApiLogger& VulkanRendererApiLogger::getSingleton()
    {
        assert(m_pSingletonType);
        return *m_pSingletonType;
    }

    VulkanRendererApiLogger* VulkanRendererApiLogger::getSingletonPtr()
    {
        return m_pSingletonType;
    }

    const std::string VulkanRendererApiLogger::VULKAN_RENDERER_API_LOGGER_NAME = "vkrenderapi";

    VulkanRendererApiLogger::VulkanRendererApiLogger( const std::initializer_list<spdlog::sink_ptr>& logSinks )
    {
        m_spLogger = std::make_shared<spdlog::logger>( VULKAN_RENDERER_API_LOGGER_NAME, logSinks );
        spdlog::register_logger(m_spLogger);
    }

    VulkanRendererApiLogger::~VulkanRendererApiLogger()
    {}

    utils::Sptr<spdlog::logger> VulkanRendererApiLogger::getLogger()
    {
        return m_spLogger;
    }

    void VulkanRendererApiLogger::createInstance( const std::initializer_list<spdlog::sink_ptr>& logSinks )
    {
        assert( !m_pSingletonType );
        VulkanRendererApiLogger* pVulkanRenderApiLogger = new VulkanRendererApiLogger( logSinks );
    }

} // namespace utils
