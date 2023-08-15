#ifndef MODEL_APPLICATION_H
#define MODEL_APPLICATION_H

#include "application/VulkanApplication.h"
#include <filesystem>

class ModelApplication : public VulkanApplication
{
public:
    ModelApplication(const std::filesystem::path& modelFilePath, const std::filesystem::path& imageFilePath);
    ~ModelApplication();

    void run() override;

    const std::filesystem::path m_modelFilePath;
    const std::filesystem::path m_imageFilePath;
};

#endif