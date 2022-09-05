#include "utilities/VulkanLogger.h"

namespace utils
{
    template<> VulkanValidationLayerLogger* Singleton<VulkanValidationLayerLogger>::m_pSingletonType = nullptr;

    VulkanValidationLayerLogger& VulkanValidationLayerLogger::getSingleton()
    {
        assert(m_pSingletonType);
        return *m_pSingletonType;
    }

    VulkanValidationLayerLogger* VulkanValidationLayerLogger::getSingletonPtr()
    {
        return m_pSingletonType;
    }

    const std::string VulkanValidationLayerLogger::VALIDATION_LOGGER_NAME = "Validation_Layer";

    VulkanValidationLayerLogger::VulkanValidationLayerLogger( const std::initializer_list<spdlog::sink_ptr>& logSinks )
    {
        m_spLogger = std::make_shared<spdlog::logger>( VALIDATION_LOGGER_NAME, logSinks );
        spdlog::register_logger(m_spLogger);
    }

    VulkanValidationLayerLogger::~VulkanValidationLayerLogger()
    {}

    utils::Sptr<spdlog::logger> VulkanValidationLayerLogger::getLogger()
    {
        return m_spLogger;
    }

    void VulkanValidationLayerLogger::createInstance( const std::initializer_list<spdlog::sink_ptr>& logSinks )
    {
        assert( !m_pSingletonType );
        VulkanValidationLayerLogger* pValidationLayerLogger = new VulkanValidationLayerLogger( logSinks );
    }

} // namespace utils
