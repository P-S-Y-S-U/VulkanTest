#include "vkrenderer/VulkanObjectCreateInfoFactory.h"
#include "vkrenderer/VulkanDebugMessenger.hpp"

namespace vkrender
{

utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT> populateDebugMessengerCreateInfoExt()
{
	utils::Sptr<vk::DebugUtilsMessengerCreateInfoEXT> debugMessengerInfo = std::make_shared<vk::DebugUtilsMessengerCreateInfoEXT>();
	debugMessengerInfo->sType = vk::StructureType::eDebugUtilsMessengerCreateInfoEXT;
	debugMessengerInfo->messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
	debugMessengerInfo->messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
	debugMessengerInfo->pfnUserCallback = VulkanDebugMessenger::debugCallback;

    return debugMessengerInfo;
}

} // namespace vkrender