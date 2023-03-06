#ifndef PYTHON_APPLICATION_H
#define PYTHON_APPLICATION_H

#include "application/VulkanApplication.h"

class PythonInputApplicaiton : public VulkanApplication
{
public:
    PythonInputApplicaiton();
    ~PythonInputApplicaiton();

    void run() override;

    bool ParseMeshInputData( const int& argc, const char* argv[] );
    
private:
    void PopulateMeshData( 
        std::vector<vertex>&& vertexData,
        std::vector<std::uint16_t>&& indexData
    );
};

#endif