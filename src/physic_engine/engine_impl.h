#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "pe_engine_core.h"
#include <glm/gtc/type_ptr.hpp>
#include "BulletCollision/CollisionDispatch/btGhostObject.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include <btBulletDynamicsCommon.h>


namespace physic_en
{
struct SPhysicsBody
{
	std::unique_ptr<btRigidBody> m_pBody;
	std::unique_ptr<btMotionState> m_pMotionState;
	std::unique_ptr<btCollisionShape> m_pColShape;

};

struct SPhysicsMesh
{
	std::unique_ptr<btRigidBody> m_pBody;
	std::unique_ptr<btMotionState> m_pMotionState;
	std::unique_ptr<btBvhTriangleMeshShape> m_pMesh;
	std::unique_ptr<btTriangleMesh> m_pTriangleMesh;
};

glm::mat4 bulletTransformToGlm(const btTransform& transform);

class CCharacter
{
public:
	CCharacter();
	~CCharacter();
	bool canJump() const;
	void setJumpSpeed(float speed);
	void jump(float speedFactor);
	void setWalkDirection(glm::vec3 direction);
	void setWalkSpeed(float speed);
	void updateWalk();
	glm::vec3 getOrigin() const;
	void setbtKinematicCharacter(std::unique_ptr<btKinematicCharacterController>&& pKinematicCharacter);
	btKinematicCharacterController* kinematicCharacter();

	void create();
	btPairCachingGhostObject* getGhostObject();
	void setRotation(const glm::mat4& mat);
	glm::mat4 getRotation();

private:

	std::unique_ptr<btConvexShape> m_pCapsule;
	std::unique_ptr<btPairCachingGhostObject> m_pGhostObject;
	glm::mat4 m_rotation;

	std::unique_ptr<btKinematicCharacterController> m_pKinematicCharacter;
	glm::vec3 m_direction;
	float m_walkSpeed{0.1f};
};


class CEngineImpl: public CEngineCore
{
public:
	CEngineImpl();
	~CEngineImpl() override;
	void stepSimulation(float step);
	void clear();
	void init();

	void addLevelMesh(uint64_t id, glm::vec3 origin, glm::vec3 scale, glm::vec4 rotate, const std::vector<glm::vec3>& mesh) override final;
	void addCharacter()override final;
	glm::mat4 getTransform(uint64_t id) const override final;
	glm::vec3 characterOrigin() const override final;
	glm::mat4 characterRotation() const override final;
	void setWalkDirection(glm::vec3 dir) override final;
	void setWalkSpeed(float speed) override final;
	bool canJump() override final;
	void jump(float speedFactor) override final;
	void updateRotation(glm::mat4 mat) override final;
	void addSphereObj(uint64_t entity, glm::vec3 origin, float mass, float radius) override final;
	void addCubeObj(uint64_t entity, glm::vec3 origin, float mass, glm::vec3 boxSize) override final;
private:
	void addBody(uint64_t id, std::unique_ptr<btCollisionShape>&& shape, glm::vec3 origin, btScalar mass);

private:
	std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	std::unique_ptr<btCollisionDispatcher>  m_dispatcher;
	std::unique_ptr<btBroadphaseInterface>  m_overlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver>  m_solver;
	std::unique_ptr<btDiscreteDynamicsWorld>  m_dynamicsWorld;
	std::unordered_map<uint64_t, std::unique_ptr<SPhysicsBody>> m_aBodies;
	std::unordered_map<uint64_t, std::unique_ptr<SPhysicsMesh>> m_aLevel;

	std::unique_ptr<CCharacter> m_pCharacter;
	std::unique_ptr<btGhostPairCallback> m_ghostCallback;
};
}