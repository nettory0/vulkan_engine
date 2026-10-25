#include "pe_physic_engine.h"
#include <iostream>
#include "engine_impl.h"

namespace physic_en
{
CPhysicsEngine::CPhysicsEngine()
{
	m_pEngine = std::make_unique<CEngineImpl>();
}
CPhysicsEngine::~CPhysicsEngine()
{
}

void CPhysicsEngine::stepSimulation(float step)
{
	if(m_pEngine)
		m_pEngine->stepSimulation(step);
}

void CPhysicsEngine::clear()
{
	if(m_pEngine)
		m_pEngine->clear();
}

void CPhysicsEngine::init()
{
	if(m_pEngine)
		m_pEngine->init();
}


void CPhysicsEngine::addLevelMesh(uint64_t id, glm::vec3 origin, glm::vec3 scale, glm::vec4 rotate, const std::vector<glm::vec3>& mesh)
{
	if(m_pEngine)
		m_pEngine->addLevelMesh(id, origin, scale, rotate, mesh);
}

glm::mat4 CPhysicsEngine::getTransform(uint64_t id) const
{
	if(m_pEngine)
		return m_pEngine->getTransform(id);
	return glm::mat4(1);
}

glm::vec3 CPhysicsEngine::characterOrigin() const
{
	if(m_pEngine)
		return m_pEngine->characterOrigin();
	return glm::vec3(0);
}

glm::mat4 CPhysicsEngine::characterRotation() const
{
	if(m_pEngine)
		return m_pEngine->characterRotation();
	return glm::mat4(1);
}

void CPhysicsEngine::setWalkDirection(glm::vec3 dir)
{
	if(m_pEngine)
		m_pEngine->setWalkDirection(dir);
}

void CPhysicsEngine::setWalkSpeed(float speed)
{
	if(m_pEngine)
		m_pEngine->setWalkSpeed(speed);
}

bool CPhysicsEngine::canJump()
{
	if(m_pEngine)
		return m_pEngine->canJump();
	return false;
}

void CPhysicsEngine::jump()
{
	if(m_pEngine)
		m_pEngine->jump();
}

void CPhysicsEngine::updateRotation(glm::mat4 mat)
{
	if(m_pEngine)
		m_pEngine->updateRotation(mat);
}

void CPhysicsEngine::addSphereCollider(uint64_t entity, glm::vec3 origin, float mass, float radius)
{
	if(m_pEngine)
		m_pEngine->addSphereObj(entity, origin, mass, radius);
}

void CPhysicsEngine::addBoxCollider(uint64_t entity, glm::vec3 origin, float mass, glm::vec3 boxSize)
{
	if(m_pEngine)
		m_pEngine->addCubeObj(entity, origin, mass, boxSize);
}

}
