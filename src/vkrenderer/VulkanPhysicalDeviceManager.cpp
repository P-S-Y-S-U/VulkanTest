#include "vkrenderer/VulkanPhysicalDeviceManager.h"
#include "vkrenderer/VulkanQueueFamily.h"
#include "utilities/VulkanLogger.h"

#include <iostream>

namespace vkrender
{
    VulkanPhysicalDeviceManager::VulkanPhysicalDeviceManager( VulkanInstance* pInstance )
        :m_pInstance{ pInstance }
    {}

    VulkanPhysicalDeviceManager::~VulkanPhysicalDeviceManager()
    {}

    VulkanPhysicalDevice* VulkanPhysicalDeviceManager::createSuitableDevice()
    {
        auto devices = getAvailableDevices();

        if( devices.empty() )
        {
            std::string errorMsg = "NO VULKAN DEVICE FOUND!";
            utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->error(errorMsg);
            throw std::runtime_error(errorMsg);
        }

        for( auto& deviceHandle : devices )
        {
            probePhysicalDeviceHandle( deviceHandle );
            VulkanPhysicalDevice temporaryDevice( createTemporaryDevice( deviceHandle ) );

            if( isDeviceSuitable( temporaryDevice ) )
            {
                auto upPhysicalDevice = std::make_unique<VulkanPhysicalDevice>(temporaryDevice);
                VulkanPhysicalDevice* pPhysicalDevice = upPhysicalDevice.get();
                m_ActiveDevices.push_back( std::move(upPhysicalDevice) );

                utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->info("Selected Suitable Vulkan GPU!");
                probePhysicalDevice( *pPhysicalDevice );

                return pPhysicalDevice;
            }
        }

        {
            std::string errorMsg = "FAILED TO SELECT A SUITABLE VULKAN GPU!";
            utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->error(errorMsg);
            throw std::runtime_error(errorMsg);
        }
        return nullptr;
    }

    void VulkanPhysicalDeviceManager::probePhysicalDevice( const VulkanPhysicalDevice& physicalDevice )
    {
        probePhysicalDeviceHandle( physicalDevice.m_deviceHandle) ;
    }

    std::vector<vk::PhysicalDevice, std::allocator<vk::PhysicalDevice>> VulkanPhysicalDeviceManager::getAvailableDevices()
    {
        return m_pInstance->m_instance.enumeratePhysicalDevices();
    }

    VulkanPhysicalDevice VulkanPhysicalDeviceManager::createTemporaryDevice( vk::PhysicalDevice& deviceHandle )
    {
        VulkanPhysicalDevice temporaryDevice{ m_pInstance, deviceHandle };
        
        auto&& [pDeviceProperties, pDeviceFeatures] = populateDeviceProperties( deviceHandle );
        temporaryDevice.m_spDeviceProperties = pDeviceProperties;
        temporaryDevice.m_spDeviceFeatures = pDeviceFeatures;

        return temporaryDevice;
    }

    bool VulkanPhysicalDeviceManager::isDeviceSuitable( const VulkanPhysicalDevice& physicalDevice )
    {
        QueueFamilyIndices queueFamilyIndices = VulkanQueueFamily::findQueueFamilyIndices( physicalDevice );

        bool bShader =  physicalDevice.m_spDeviceProperties->deviceType == vk::PhysicalDeviceType::eDiscreteGpu && 
                        physicalDevice.m_spDeviceFeatures->geometryShader;
        
        bool bGraphicsFamily = queueFamilyIndices.m_graphicsFamily.has_value();
        
        return bShader && bGraphicsFamily;
    }

    void VulkanPhysicalDeviceManager::probePhysicalDeviceHandle( const vk::PhysicalDevice& deviceHandle )
    {
        const vk::PhysicalDeviceProperties& deviceProperties = deviceHandle.getProperties();
        utils::VulkanRendererApiLogger::getSingletonPtr()->getLogger()->info(
            "Device ID : {} Device Name : {} Vendor: {}", deviceProperties.deviceID, deviceProperties.deviceName, deviceProperties.vendorID 
        );
    }

    VulkanPhysicalDeviceManager::DeviceCapabilitiesPair VulkanPhysicalDeviceManager::populateDeviceProperties( vk::PhysicalDevice& pDeviceHandle )
    {
        utils::Sptr<vk::PhysicalDeviceProperties> pDeviceProperties = std::make_shared<vk::PhysicalDeviceProperties>();
        utils::Sptr<vk::PhysicalDeviceFeatures> pDeviceFeatures = std::make_shared<vk::PhysicalDeviceFeatures>();

        pDeviceHandle.getProperties( pDeviceProperties.get() );
        pDeviceHandle.getFeatures( pDeviceFeatures.get() );

        return std::make_pair( pDeviceProperties, pDeviceFeatures );
    }

} // namespace vkrender
