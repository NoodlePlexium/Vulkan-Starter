#ifndef ENGINE_GAME_OBJECT_H
#define ENGINE_GAME_OBJECT_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "engine_mesh.h"
#include <memory>

namespace Engine{

struct TransformComponent {
	glm::vec3 translation{};
	glm::vec3 scale{1.0f, 1.0f, 1.0f};
	glm::vec3 rotation{};

	// matrix corresponds to translate * Ry * Rx * Rz * scale transformation
	// Rotation convention uses tait-bryan angles with axes order Y(1), X(2), Z(3)
	glm::mat4 mat4() {
    	const float c3 = glm::cos(rotation.z);
    	const float s3 = glm::sin(rotation.z);
    	const float c2 = glm::cos(rotation.x);
    	const float s2 = glm::sin(rotation.x);
    	const float c1 = glm::cos(rotation.y);
    	const float s1 = glm::sin(rotation.y);
    	return glm::mat4{
	        {
	            scale.x * (c1 * c3 + s1 * s2 * s3),
	            scale.x * (c2 * s3),
	            scale.x * (c1 * s2 * s3 - c3 * s1),
	            0.0f,
	        },
	        {
	            scale.y * (c3 * s1 * s2 - c1 * s3),
	            scale.y * (c2 * c3),
	            scale.y * (c1 * c3 * s2 + s1 * s3),
	            0.0f,
	        },
	        {
	            scale.z * (c2 * s1),
	            scale.z * (-s2),
	            scale.z * (c1 * c2),
	            0.0f,
	        },
	        {translation.x, translation.y, translation.z, 1.0f}};
  	}

  	glm::mat3 normalMatrix(){
    	const float c3 = glm::cos(rotation.z);
    	const float s3 = glm::sin(rotation.z);
    	const float c2 = glm::cos(rotation.x);
    	const float s2 = glm::sin(rotation.x);
    	const float c1 = glm::cos(rotation.y);
    	const float s1 = glm::sin(rotation.y);
    	const glm::vec3 inverseScale = 1.0f / scale;
		return glm::mat3{
	        {
	            inverseScale.x * (c1 * c3 + s1 * s2 * s3),
	            inverseScale.x * (c2 * s3),
	            inverseScale.x * (c1 * s2 * s3 - c3 * s1),
	        },
	        {
	            inverseScale.y * (c3 * s1 * s2 - c1 * s3),
	            inverseScale.y * (c2 * c3),
	            inverseScale.y * (c1 * c3 * s2 + s1 * s3),
	        },
	        {
	            inverseScale.z * (c2 * s1),
	            inverseScale.z * (-s2),
	            inverseScale.z * (c1 * c2),
	        }
	    };    
  	}
};

class EngineGameObject {
public:

	using id_t = unsigned int;	

	static EngineGameObject createGameObject(){
		static id_t currentId = 0;
		return EngineGameObject{currentId++};
	}

	EngineGameObject(const EngineGameObject &) = delete;
	EngineGameObject &operator=(const EngineGameObject &) = delete;
	EngineGameObject(EngineGameObject&&) = default;
	EngineGameObject &operator=(EngineGameObject&&) = default;

	const id_t getId() {return id;}

	std::shared_ptr<EngineMesh> getMesh() const {
        return mesh;
    }

	std::shared_ptr<EngineMesh> mesh{};
	glm::vec3 colour{};
	TransformComponent transform;

private:
	EngineGameObject(id_t objId) : id{objId} {}

	id_t id;
};
} // namespace

#endif