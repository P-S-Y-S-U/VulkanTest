#ifndef TRIANGLE_APPLICATION_H
#define TRIANGLE_APPLICATION_H

#include "VulkanApplication.h"


class TriangleApplication : public VulkanApplication
{
public:
    TriangleApplication();
    ~TriangleApplication();

    void run() override;
};

#endif