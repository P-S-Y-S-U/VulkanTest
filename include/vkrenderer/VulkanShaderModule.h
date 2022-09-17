#ifndef VKRENDER_VULKAN_SHADER_MODULE_H
#define VKRENDER_VULKAN_SHADER_MODULE_H

#include <vulkan/vulkan.hpp>
#include <filesystem>

#include "exports.hpp"
#include "utilities/memory.hpp"
#include "vkrenderer/VulkanLogicalDevice.h"

namespace vkrender
{
    class VULKAN_EXPORTS VulkanShaderModule
    {
    public:
        explicit VulkanShaderModule( const std::filesystem::path& shaderFilePath, VulkanLogicalDevice* pLogicalDevice );
        explicit VulkanShaderModule( const std::vector<char>& sourceBuffer, VulkanLogicalDevice* pLogicalDevice );
        ~VulkanShaderModule();

        void createShaderModule();
        void destroyShaderModule();
    private:
        vk::ShaderModule m_shaderModuleHandle;
        utils::Sptr<vk::ShaderModuleCreateInfo> m_spShaderModuleCreateInfo;

        VulkanLogicalDevice* m_pLogicalDevice;

        std::vector<char> m_shaderSourceBuffer;

        void readSourceBuffer( const std::filesystem::path& shaderFilePath );
        void populateShaderModuleCreateInfo();

        friend class VulkanGraphicsPipeline;
    };
} // namespace vkrender

#endif