#include "vkrenderer/VulkanShaderModule.h"
#include "utilities/VulkanLogger.h"
#include <fstream>

namespace vkrender
{
    VulkanShaderModule::VulkanShaderModule( const std::filesystem::path& shaderSourcePath, VulkanLogicalDevice* pLogicalDevice )
        :m_pLogicalDevice{ pLogicalDevice }
    {
        readSourceBuffer( shaderSourcePath );
    }

    VulkanShaderModule::VulkanShaderModule( const std::vector<char>& sourceBuffer, VulkanLogicalDevice* pLogicalDevice )
        :m_shaderSourceBuffer{ sourceBuffer }
        ,m_pLogicalDevice{ pLogicalDevice }
    {}
    
    VulkanShaderModule::~VulkanShaderModule()
    {
       destroyShaderModule(); 
    }

    void VulkanShaderModule::createShaderModule()
    {
        populateShaderModuleCreateInfo();
        m_shaderModuleHandle = m_pLogicalDevice->m_deviceHandle.createShaderModule( *m_spShaderModuleCreateInfo );
    }

    void VulkanShaderModule::destroyShaderModule()
    {
        m_pLogicalDevice->m_deviceHandle.destroyShaderModule( m_shaderModuleHandle );
    }

    void VulkanShaderModule::readSourceBuffer( const std::filesystem::path& shaderFilePath )
    {
        if( shaderFilePath.extension() != std::filesystem::path{ ".spv" } )
        {
            std::string errorMsg = "ILLEGAL SHADER FILE FORMAT!";
            utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->error(errorMsg);
            throw std::runtime_error(errorMsg);
        }

        std::ifstream fstream( shaderFilePath.string(), std::ios::ate | std::ios::binary );

        std::size_t fileSize = static_cast<std::size_t>( fstream.tellg() );
        m_shaderSourceBuffer.resize( fileSize );

        fstream.seekg(0);
        fstream.read( m_shaderSourceBuffer.data(), fileSize );

        fstream.close();
    }

    void VulkanShaderModule::populateShaderModuleCreateInfo()
    {
        m_spShaderModuleCreateInfo = std::make_shared<vk::ShaderModuleCreateInfo>();
        m_spShaderModuleCreateInfo->sType = vk::StructureType::eShaderModuleCreateInfo;
        m_spShaderModuleCreateInfo->codeSize = m_shaderSourceBuffer.size();
        m_spShaderModuleCreateInfo->pCode = reinterpret_cast<const std::uint32_t*>( m_shaderSourceBuffer.data() ); 
    }
} // namespace vkrender
