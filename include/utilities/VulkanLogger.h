#ifndef UTILS_VULKAN_LOGGER_H
#define UTILS_VULKAN_LOGGER_H

#include "exports.hpp"
#include "utilities/memory.hpp"
#include "utilities/Singleton.hpp"

#include <initializer_list>
#include <spdlog/spdlog.h>

namespace logger 
{
    enum level
    {
        LOG_DEBUG,
        LOG_INFO,
        LOG_ERROR
    };
};

namespace utils
{
    class VULKAN_EXPORTS VulkanValidationLayerLogger : public Singleton<VulkanValidationLayerLogger>
    {
    public:
        ~VulkanValidationLayerLogger();
        
        utils::Sptr<spdlog::logger> getLogger();

        static const std::string VALIDATION_LOGGER_NAME;

        static void createInstance( const std::initializer_list<spdlog::sink_ptr>& logSinks );
        static VulkanValidationLayerLogger& getSingleton();
        static VulkanValidationLayerLogger* getSingletonPtr();
    private:
        VulkanValidationLayerLogger( const std::initializer_list<spdlog::sink_ptr>& logSinks );

        utils::Sptr<spdlog::logger> m_spLogger;
    };

    class VULKAN_EXPORTS VulkanRendererApiLogger : public Singleton<VulkanRendererApiLogger>
    {
    public:
        ~VulkanRendererApiLogger();
        
        utils::Sptr<spdlog::logger> getLogger();

        static const std::string VULKAN_RENDERER_API_LOGGER_NAME;

        static void createInstance( const std::initializer_list<spdlog::sink_ptr>& logSinks );
        static VulkanRendererApiLogger& getSingleton();
        static VulkanRendererApiLogger* getSingletonPtr();
    private:
        VulkanRendererApiLogger( const std::initializer_list<spdlog::sink_ptr>& logSinks );

        utils::Sptr<spdlog::logger> m_spLogger;
    };

    class VulkanLoggerFactory
    {
    public:
        static void logMessage( const std::string& message, const logger::level& level )
        {
            auto pLogger = VulkanRendererApiLogger::getSingletonPtr()->getLogger();

            switch (level)
            {
            case logger::LOG_DEBUG:
                pLogger->debug( message );
                break;
            case logger::LOG_INFO:
                pLogger->info( message );
                break;
            case logger::LOG_ERROR:
                pLogger->error( message );
            default:
                break;
            }
        };
    };

} // namespace utils

#define LOG_MESSAGE( message, level ) utils::VulkanLoggerFactory::logMessage( message, level )
#define LOG_DEBUG( message ) LOG_MESSAGE( message, logger::LOG_DEBUG )
#define LOG_ERROR( message ) LOG_MESSAGE( message, logger::LOG_ERROR )
#define LOG_INFO( message ) LOG_MESSAGE( message, logger::LOG_INFO )

#endif 