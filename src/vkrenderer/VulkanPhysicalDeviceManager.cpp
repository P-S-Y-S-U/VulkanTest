#include "vkrenderer/VulkanPhysicalDeviceManager.h"
#include "vkrenderer/VulkanQueueFamily.h"

#include <iostream>

namespace vkrender
{
    VulkanPhysicalDeviceManager::VulkanPhysicalDeviceManager( VulkanInstance* pInstance )
        :m_pInstance{ pInstance }
    {}

    VulkanPhysicalDeviceManager::~VulkanPhysicalDeviceManager()
    {}

    VulkanPhysicalDevice* VulkanPhysicalDeviceManager::CreateSuitableDevice()
    {
        auto devices = getAvailableDevices();

        if( devices.empty() )
        {
            throw std::runtime_error("no suitable vulkan device found!");
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

                return pPhysicalDevice;
            }
        }

        throw std::runtime_error("Failed to select a compatible Vulkan GPU");
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
        std::cout << "Device ID : " << deviceProperties.deviceID << " Device Name : " << deviceProperties.deviceName << " Vendor : " << deviceProperties.vendorID << std::endl;
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
