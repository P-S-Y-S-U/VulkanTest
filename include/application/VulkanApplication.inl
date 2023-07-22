#include "VulkanApplication.h"

template<typename countType, typename timeUnit>
std::chrono::duration<countType, timeUnit> VulkanApplication::durationSinceLastFrameUpdate()
{
    auto timeNow = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration<countType, timeUnit>( timeNow - m_timeSinceLastUpdateFrame );
}