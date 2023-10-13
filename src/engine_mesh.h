#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "engine_device.h"
#include "engine_buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <vector>
#include <cassert>
#include <cstring>
#include <memory>


namespace Engine{

class EngineMesh{
public:

	struct Vertex{
		glm::vec3 position{};
		glm::vec3 colour{};
		glm::vec3 normal{};
		glm::vec2 uv{};
		
		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions(){
			std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(Vertex);
			bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescriptions;
		}
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(){
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

			attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
			attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, colour)});
			attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
			attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

			// Position
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			// Colour
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, colour);
			return attributeDescriptions;
		}
	};

	struct Builder{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};

        void loadModel(const std::string &filepath){
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
				throw std::runtime_error(warn + err);
			}


            vertices.clear();
            indices.clear();

            for (const auto &shape : shapes)
            {
                for (const auto &index : shape.mesh.indices)
                {
                    Vertex vertex{};

                    if (index.vertex_index >= 0)
                    {
                        vertex.position = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2]
                        };

                        vertex.colour = {
							attrib.colors[3 * index.vertex_index + 0],
							attrib.colors[3 * index.vertex_index + 1],
							attrib.colors[3 * index.vertex_index + 2]
						};
                    }

                    if (index.normal_index >= 0)
                    {
                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };
                    }

                    if (index.texcoord_index >= 0)
                    {
                        vertex.uv = {
                            attrib.texcoords[2 * index.texcoord_index + 0],
                            attrib.texcoords[2 * index.texcoord_index + 1]
                        };
                    }

                    vertices.push_back(vertex);
                }
            }
        }
	};

	EngineMesh(EngineDevice& _engineDevice, const EngineMesh::Builder &_builder) : engineDevice{_engineDevice}, builder{_builder} {
		createVertexBuffers(_builder.vertices);
		createIndexBuffers(_builder.indices);
	}

	~EngineMesh() {}
	
	EngineMesh(const EngineMesh &) = delete;
	EngineMesh &operator=(const EngineMesh &) = delete;		

    static std::unique_ptr<EngineMesh> createMeshFromFile(EngineDevice &device, const std::string &filepath){
        Builder builder{};
        builder.loadModel(filepath);

        return std::make_unique<EngineMesh>(device, builder);
    }

	void bind(VkCommandBuffer commandBuffer) {
		VkBuffer buffers[] = {vertexBuffer->getBuffer()};
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		if (hasIndexBuffer){
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void draw(VkCommandBuffer commandBuffer) {
		if (hasIndexBuffer) {vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);}
		else                {vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);}
	}

	const std::vector<Vertex>& getVertices() const {
		assert(builder.vertices.size() >= 3 && "Mesh requires 3 or more vertices!");
        return builder.vertices;
    }

    const std::vector<uint32_t>& getIndices() const {
        return builder.indices;
    }


private:

	// ADDED STAGING BUFFER - TEST PERFORMANCE AND REFER TO https://www.youtube.com/watch?v=qxuvQVtehII&t=385s FOR INFO
	void createVertexBuffers(const std::vector<Vertex> &vertices){
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		EngineBuffer stagingBuffer{
			engineDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void *) vertices.data());

		vertexBuffer = std::make_unique<EngineBuffer>(
			engineDevice,
			vertexSize,
			vertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// Copy staging buffer to vertex buffer
		engineDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
	}

	// ADDED STAGING BUFFER - TEST PERFORMANCE AND REFER TO https://www.youtube.com/watch?v=qxuvQVtehII&t=385s FOR INFO
	void createIndexBuffers(const std::vector<uint32_t> &indices){
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;

		if (!hasIndexBuffer) return;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
		uint32_t indexSize = sizeof(indices[0]);


		EngineBuffer stagingBuffer{
			engineDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void *) indices.data());

		indexBuffer = std::make_unique<EngineBuffer>(
			engineDevice,
			indexSize,
			indexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		// Copy staging buffer to vertex buffer
		engineDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
	}

	EngineDevice& engineDevice;
	Builder builder;

    // VERTICES
	std::unique_ptr<EngineBuffer> vertexBuffer;
	uint32_t vertexCount; 

    // INDICES
	bool hasIndexBuffer = false;
	std::unique_ptr<EngineBuffer> indexBuffer;
	uint32_t indexCount; 
};	
} // namespace



#endif