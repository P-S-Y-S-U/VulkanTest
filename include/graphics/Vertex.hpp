#ifndef GRAPHICS_VERTEX_HPP
#define GRAPHICS_VERTEX_HPP

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

struct vertex
{
    glm::vec2 pos;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription(){
        vk::VertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;    

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions(){
        std::array<vk::VertexInputAttributeDescription, 2> vertexAttributes;

        vertexAttributes[0].binding = 0;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].format = vk::Format::eR32G32Sfloat;
        vertexAttributes[0].offset = offsetof( vertex, pos );

        vertexAttributes[1].binding = 0;
        vertexAttributes[1].location = 1;
        vertexAttributes[1].format = vk::Format::eR32G32B32Sfloat;
        vertexAttributes[1].offset = offsetof( vertex, color );

        return vertexAttributes;
    };
};

#endif 
