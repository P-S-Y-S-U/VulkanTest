#include "PythonInputApplication.h"
#include "utilities/VulkanLogger.h"
#include <exception>
#include <iostream>
#include <Python.h>
#include <spdlog/fmt/fmt.h>

PythonInputApplicaiton::PythonInputApplicaiton()
    :VulkanApplication::VulkanApplication{
        "PythonInputApp"
    }
{

}

PythonInputApplicaiton::~PythonInputApplicaiton()
{

}

void PythonInputApplicaiton::run()
{
    initialise();
    mainLoop();
}

bool PythonInputApplicaiton::ParseMeshInputData( const int& argc, const char* argv[] )
{
    if( argc < 3 )
    {
        utils::VulkanLoggerFactory::logMessage(
            "Usage: appName [pythonfile] [funcname]",
            logger::level::LOG_ERROR
        );
        return false;
    }

    PyObject* pName;
    PyObject* pModule;

    std::string moduleName{ argv[1] };
    std::string functionName{ argv[2] };
    bool bParseOperationSuccess = false;

    Py_Initialize();
    pName = PyUnicode_DecodeFSDefault( moduleName.data() );

    pModule = PyImport_Import( pName );
    Py_DECREF(pName);

    std::vector<vertex> inputVertexData;
    std::vector<std::uint16_t> inputIndexData;

    if( pModule )
    {
        PyObject* pFunc = PyObject_GetAttrString(
            pModule,
            functionName.data()
        );

        if( pFunc && PyCallable_Check(pFunc) )
        {
            PyObject* pFuncReturn;

            pFuncReturn = PyObject_CallFunction(
                pFunc,
                nullptr
            );

            if( pFuncReturn )
            {
                std::size_t meshTupleSize = static_cast<std::size_t>( PyTuple_Size( pFuncReturn ) );

                PyObject* pVertexObj = PyTuple_GetItem( pFuncReturn, 0 );
                PyObject* pIndexObj = PyTuple_GetItem( pFuncReturn, 1 );

                std::size_t numOfVertices = static_cast<std::size_t>( PyList_Size( pVertexObj ) );
                std::size_t numOfIndices = static_cast<std::size_t>( PyList_Size( pIndexObj ) );
                
                inputVertexData.resize( numOfVertices );
                inputIndexData.resize( numOfIndices );

                // Parsing Vertex Data
                for( std::size_t i = 0; i < numOfVertices; i++ )
                {
                    PyObject* pVertexTuple = PyList_GetItem( pVertexObj, i );

                    PyObject* pPositionObj = PyTuple_GetItem(pVertexTuple, 0);
                    PyObject* pColorObj = PyTuple_GetItem(pVertexTuple, 1);


                    // Parsing Position Data
                    float posX = static_cast<float>(
                        PyFloat_AsDouble(
                            PyList_GetItem( pPositionObj, 0 )
                        )
                    );
                    float posY = static_cast<float>(
                        PyFloat_AsDouble(
                            PyList_GetItem( pPositionObj, 1 )
                        )
                    );

                    // Parsing Color Data
                    float colorR = static_cast<float>(
                        PyFloat_AsDouble(
                            PyList_GetItem( pColorObj, 0 )
                        )
                    );
                    float colorG = static_cast<float>(
                        PyFloat_AsDouble(
                            PyList_GetItem( pColorObj, 1 )
                        )
                    );
                    float colorB = static_cast<float>(
                        PyFloat_AsDouble(
                            PyList_GetItem( pColorObj, 2 )
                        )
                    );
                    
                    vertex& vkApiVertex = inputVertexData[i];
                    
                    vkApiVertex.pos = glm::vec2{ posX, posY };
                    vkApiVertex.color = glm::vec3{ colorR, colorG, colorB };

                    //Py_DECREF( pVertexTuple );
                }

                // Parsing Index Data
                for (std::size_t i = 0; i < numOfIndices; i++)
                {
                    PyObject* pIndexedValue = PyList_GetItem(pIndexObj, i);
                    
                    inputIndexData[i] = static_cast<std::uint16_t>(
                        PyLong_AsUnsignedLong(pIndexedValue)
                    );

                    //Py_DECREF(pIndexedValue);
                }
                
                Py_DECREF(pFuncReturn);
            }
            else {
                PyErr_Print();
                utils::VulkanLoggerFactory::logMessage(
                    fmt::format("Call to {} failed",functionName),
                    logger::level::LOG_ERROR
                );
            }
        }
        else{
            if(PyErr_Occurred())
            {
                PyErr_Print();
            }
            
            utils::VulkanLoggerFactory::logMessage(
                fmt::format("Can't Find the function {} in the module {}", functionName, moduleName ),
                logger::level::LOG_ERROR
            );
        }

        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    else
    {
        PyErr_Print();
        utils::VulkanLoggerFactory::logMessage(
            fmt::format("Failed to load the {} module", argv[1]),
            logger::level::LOG_ERROR
        );
    }

    if( !inputVertexData.empty() && !inputIndexData.empty() )
    {
        bParseOperationSuccess = true;
        PopulateMeshData(
            std::move(inputVertexData),
            std::move(inputIndexData)
        );
    }

    if( Py_FinalizeEx() < 0 )
    {
        utils::VulkanLoggerFactory::logMessage(
            "Probles while de-initialising Python interpreter",
            logger::level::LOG_ERROR
        );
    }

    return bParseOperationSuccess;
}

void PythonInputApplicaiton::PopulateMeshData( 
    std::vector<vertex>&& vertexData,
    std::vector<std::uint16_t>&& indexData
)
{
    m_inputVertexData = std::move(vertexData);
    m_inputIndexData = std::move(indexData);
}

int main(int argc, const char* argv[])
{
    auto application = PythonInputApplicaiton{};
    
    bool bSucess = application.ParseMeshInputData(argc, argv);
    
    if( bSucess )
    {
        try
        {
            application.run();
        }
        catch( const std::exception& e )
        {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else
    {
        utils::VulkanLoggerFactory::logMessage(
            "FATAL!!!! Parse Operation Failed",
            logger::level::LOG_ERROR
        );
        return EXIT_FAILURE;
    }
}