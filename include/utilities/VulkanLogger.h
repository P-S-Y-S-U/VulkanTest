#ifndef UTILS_VULKAN_LOGGER_H
#define UTILS_VULKAN_LOGGER_H

#include "exports.hpp"
#include "utilities/memory.hpp"
#include "utilities/Singleton.hpp"

#include <initializer_list>
#include <spdlog/spdlog.h>

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
} // namespace utils

#endif 