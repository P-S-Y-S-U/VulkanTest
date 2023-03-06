#ifndef PYTHON_APPLICATION_H
#define PYTHON_APPLICATION_H

#include "application/VulkanApplication.h"

class PythonInputApplicaiton : public VulkanApplication
{
public:
    PythonInputApplicaiton();
    ~PythonInputApplicaiton();

    void run() override;

    void ParseMeshInputData();
};

#endif