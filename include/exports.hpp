#ifndef APP_EXPORTS_HPP
#define APP_EXPORTS_HPP

#ifdef WIN32
    #ifdef vulkanrenderer_EXPORTS
        #define VULKAN_EXPORTS __declspec(dllexport)
    #else
        #define VULKAN_EXPORTS __declspec(dllimport)
    #endif // #ifdef vulkanrenderer_EXPORTS
#else
    #define VULKAN_EXPORTS
#endif // #ifdef WIN32 

#endif