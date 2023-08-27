#ifndef GRAPHICS_VERTEX_HPP
#define GRAPHICS_VERTEX_HPP

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan.hpp>

struct vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    bool operator==(const vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
    
    static vk::VertexInputBindingDescription getBindingDescription(){
        vk::VertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(vertex);
        bindingDescription.inputRate = vk::VertexInputRate::eVertex;    

        return bindingDescription;
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions(){
        std::array<vk::VertexInputAttributeDescription, 3> vertexAttributes;

        vertexAttributes[0].binding = 0;
        vertexAttributes[0].location = 0;
        vertexAttributes[0].format = vk::Format::eR32G32B32Sfloat;
        vertexAttributes[0].offset = offsetof( vertex, pos );

        vertexAttributes[1].binding = 0;
        vertexAttributes[1].location = 1;
        vertexAttributes[1].format = vk::Format::eR32G32B32Sfloat;
        vertexAttributes[1].offset = offsetof( vertex, color );

        vertexAttributes[2].binding = 0;
        vertexAttributes[2].location = 2;
        vertexAttributes[2].format = vk::Format::eR32G32Sfloat;
        vertexAttributes[2].offset = offsetof( vertex, texCoord );

        return vertexAttributes;
    };
};

namespace std {
    template<> struct hash<vertex> {
        size_t operator()(vertex const& vertexData) const {
            return ( (hash<glm::vec3>()(vertexData.pos) ^ (hash<glm::vec3>()(vertexData.color) << 1) ) >> 1 ) ^ ( hash<glm::vec2>()(vertexData.texCoord) << 1 );
        }
    };
}

#endif 
