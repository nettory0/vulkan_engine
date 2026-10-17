#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "pe_physic_export.h"
#include "pe_engine_core.h"

namespace physic_en
{

class PHYSIC_API CPhysicsEngine
{
public:
	CPhysicsEngine();
	~CPhysicsEngine();

	void stepSimulation(float step);
	void clear();
	void init();

	// `rotate`: xyz is the rotation axis, w is the angle in degrees.
	void addLevelMesh(uint64_t id, glm::vec3 origin, glm::vec3 scale, glm::vec4 rotate, const std::vector<glm::vec3>& mesh);
	glm::mat4 getTransform(uint64_t id) const;
	glm::vec3 characterOrigin() const;
	glm::mat4 characterRotation() const;
	void setWalkDirection(glm::vec3 dir);
	bool canJump();
	void jump();
	void updateRotation(glm::mat4 mat);
	void addSphereCollider(uint64_t entity, glm::vec3 origin, float mass, float radius);
	void addBoxCollider(uint64_t entity, glm::vec3 origin, float mass, glm::vec3 boxSize);

private:
	std::unique_ptr<CEngineCore> m_pEngine;

};

}
