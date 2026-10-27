#pragma once
#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

class CEngineCore
{
public:
	CEngineCore() = default;
	virtual void stepSimulation(float step) = 0;
	virtual void clear() = 0;
	virtual void init() = 0;

	// `rotate`: xyz is the rotation axis, w is the angle in degrees.
	virtual void addLevelMesh(uint64_t id, glm::vec3 origin, glm::vec3 scale, glm::vec4 rotate, const std::vector<glm::vec3>& mesh) = 0;
	virtual void addCharacter() = 0;
	virtual glm::mat4 getTransform(uint64_t id) const = 0;
	virtual glm::vec3 characterOrigin() const = 0;
	virtual glm::mat4 characterRotation() const = 0;
	virtual void setWalkDirection(glm::vec3 dir) = 0;
	// Distance per fixed simulation tick (1/60 s).
	virtual void setWalkSpeed(float speed) = 0;
	virtual bool canJump() = 0;
	// speedFactor scales the base jump speed (1 = normal jump).
	virtual void jump(float speedFactor) = 0;
	virtual void updateRotation(glm::mat4 mat) = 0;
	virtual void addSphereObj(uint64_t entity, glm::vec3 origin, float mass, float radius) = 0;
	virtual void addCubeObj(uint64_t entity, glm::vec3 origin, float mass, glm::vec3 boxSize) = 0;
	virtual ~CEngineCore() = default;
};