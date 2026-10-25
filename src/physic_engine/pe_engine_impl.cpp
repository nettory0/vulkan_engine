#include "engine_impl.h"
#include <algorithm>

namespace physic_en
{
namespace
{
constexpr float kGravityY = -9.8f;
}

CEngineImpl::CEngineImpl()
{
	m_collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfiguration.get());
	m_overlappingPairCache = std::make_unique <btDbvtBroadphase>();
	m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	m_dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(m_dispatcher.get(), m_overlappingPairCache.get(), m_solver.get(), m_collisionConfiguration.get());
}

void CEngineImpl::stepSimulation(float step)
{
	if(m_pCharacter)
		m_pCharacter->updateWalk();

	constexpr int maxSubSteps = 10;
	constexpr float fixedTimeStep = 1.f / 60.f;
	// Clamp so a lagged/large frame can't ask Bullet to catch up more time
	// than maxSubSteps*fixedTimeStep can cover - past that point Bullet
	// silently drops the extra time instead of simulating it.
	const float clampedStep = std::min(step, maxSubSteps * fixedTimeStep);
	m_dynamicsWorld->stepSimulation(clampedStep, maxSubSteps, fixedTimeStep);
}

void CEngineImpl::clear()
{
	for(auto& kv : m_aBodies)
		m_dynamicsWorld->removeRigidBody(kv.second->m_pBody.get());
	m_aBodies.clear();

	for(auto& kv : m_aLevel)
		m_dynamicsWorld->removeRigidBody(kv.second->m_pBody.get());
	m_aLevel.clear();

	if(m_pCharacter)
	{
		m_dynamicsWorld->removeAction(m_pCharacter->kinematicCharacter());
		m_dynamicsWorld->removeCollisionObject(m_pCharacter->getGhostObject());
		m_pCharacter.reset();
	}
}

CEngineImpl::~CEngineImpl()
{
	// Bullet's dynamics world keeps raw pointers to everything added to it,
	// so bodies/character must be unregistered before their unique_ptrs
	// (and the world itself) get destroyed - clear() does exactly that.
	clear();
}

void CEngineImpl::init()
{
	m_dynamicsWorld->setGravity(btVector3(0, kGravityY, 0));
	addCharacter();
}

void CEngineImpl::addBody(uint64_t id, std::unique_ptr<btCollisionShape>&& shape, glm::vec3 origin, btScalar mass)
{
	m_aBodies[id] = std::make_unique<physic_en::SPhysicsBody>();
	auto& pPhisycBody = m_aBodies[id];
	pPhisycBody->m_pColShape = std::move(shape);
	btVector3 localInertia{0, 0, 0};

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(origin.x, origin.y, origin.z));

	bool isDynamic = (mass != 0.f);

	if(isDynamic)
		pPhisycBody->m_pColShape->calculateLocalInertia(mass, localInertia);

	//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
	pPhisycBody->m_pMotionState = std::make_unique<btDefaultMotionState>(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, pPhisycBody->m_pMotionState.get(), pPhisycBody->m_pColShape.get(), localInertia);
	pPhisycBody->m_pBody = std::make_unique<btRigidBody>(rbInfo);

	int group = (mass == 0.0f) ? btBroadphaseProxy::StaticFilter : btBroadphaseProxy::DefaultFilter;
	int mask = btBroadphaseProxy::AllFilter;


	if(mass == 0.0f)
	{
		pPhisycBody->m_pBody->setCollisionFlags(btCollisionObject::CF_STATIC_OBJECT);
	}

	m_dynamicsWorld->addRigidBody(pPhisycBody->m_pBody.get(), group, mask);
}

void CEngineImpl::addLevelMesh(uint64_t id, glm::vec3 origin, glm::vec3 scale, glm::vec4 rotate, const std::vector<glm::vec3>& mesh)
{
	m_aLevel[id] = std::make_unique<physic_en::SPhysicsMesh>();
	auto& physicMesh = m_aLevel[id];
	physicMesh->m_pTriangleMesh = std::make_unique<btTriangleMesh>();

	const size_t triangleCount = mesh.size() / 3;
	for(size_t t = 0; t < triangleCount; ++t)
	{
		const size_t pt0 = t * 3, pt1 = pt0 + 1, pt2 = pt0 + 2;
		btVector3 point0{mesh[pt0].x, mesh[pt0].y, mesh[pt0].z};
		btVector3 point1{mesh[pt1].x, mesh[pt1].y, mesh[pt1].z};
		btVector3 point2{mesh[pt2].x, mesh[pt2].y, mesh[pt2].z};
		physicMesh->m_pTriangleMesh->addTriangle(point0, point1, point2);
	}

	bool useQuantizedAabbCompression = true;

	physicMesh->m_pMesh = std::make_unique<btBvhTriangleMeshShape>(physicMesh->m_pTriangleMesh.get(), useQuantizedAabbCompression);
	physicMesh->m_pMesh->setLocalScaling(btVector3(scale.x, scale.y, scale.z));

	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(origin.x, origin.y, origin.z));

	btVector3 rotationAxis{btVector3(rotate.x, rotate.y, rotate.z)};
	btScalar angleRadians = glm::radians(rotate.w);
	btQuaternion rotation(rotationAxis, angleRadians);

	groundTransform.setRotation(rotation);
	physicMesh->m_pMotionState = std::make_unique<btDefaultMotionState>(groundTransform);

	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, physicMesh->m_pMotionState.get(), physicMesh->m_pMesh.get());
	int group = btBroadphaseProxy::StaticFilter;
	int mask = btBroadphaseProxy::AllFilter;

	physicMesh->m_pBody = std::make_unique<btRigidBody>(rbInfo);
	m_dynamicsWorld->addRigidBody(physicMesh->m_pBody.get(), group, mask);
}

void CEngineImpl::addCharacter()
{
	m_ghostCallback = std::make_unique<btGhostPairCallback>();
	m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(m_ghostCallback.get());

	m_pCharacter = std::make_unique<CCharacter>();

	m_dynamicsWorld->addCollisionObject(m_pCharacter->getGhostObject(), btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);
	m_dynamicsWorld->addAction(m_pCharacter->kinematicCharacter());

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(10, 10, 10));

	m_pCharacter->getGhostObject()->setWorldTransform(startTransform);
}

glm::mat4 CEngineImpl::getTransform(uint64_t id) const
{
	const auto it = m_aBodies.find(id);
	if(it != m_aBodies.end())
	{
		btTransform transform;
		it->second->m_pBody->getMotionState()->getWorldTransform(transform);
		return bulletTransformToGlm(transform);
	}
	return glm::mat4{1};

}

glm::vec3 CEngineImpl::characterOrigin() const
{
	if(m_pCharacter)
		return m_pCharacter->getOrigin();
	return glm::vec3{0};
}

glm::mat4 CEngineImpl::characterRotation() const
{
	if(m_pCharacter)
		return m_pCharacter->getRotation();
	return glm::mat4{1.};
}

void CEngineImpl::setWalkDirection(glm::vec3 dir)
{
	if(m_pCharacter)
		m_pCharacter->setWalkDirection(dir);
}

bool CEngineImpl::canJump()
{
	if(m_pCharacter)
		return m_pCharacter->canJump();
	return false;
}

void CEngineImpl::jump()
{
	if(m_pCharacter)
		m_pCharacter->jump();
}

void CEngineImpl::updateRotation(glm::mat4 mat)
{
	if(m_pCharacter)
		m_pCharacter->setRotation(mat);
}

void CEngineImpl::addSphereObj(uint64_t entity, glm::vec3 origin, float mass, float radius)
{
	addBody(entity, std::make_unique<btSphereShape>(radius), {origin.x, origin.y, origin.z}, mass);
}

void CEngineImpl::addCubeObj(uint64_t entity, glm::vec3 origin, float mass, glm::vec3 boxSize)
{
	addBody(entity, std::make_unique<btBoxShape>(btVector3(boxSize.x, boxSize.y, boxSize.z)), origin, mass);
}

CCharacter::CCharacter()
{
	create();
}

CCharacter::~CCharacter()
{
}

bool CCharacter::canJump() const
{
	return m_pKinematicCharacter->canJump();
}

void CCharacter::setJumpSpeed(float speed)
{
	m_pKinematicCharacter->setJumpSpeed(speed);
}

void CCharacter::jump()
{
	m_pKinematicCharacter->jump();
}

void CCharacter::setWalkDirection(glm::vec3 direction)
{
	// Walking is horizontal (gravity/jump handle Y) and the speed is set in updateWalk,
	// so keep only a unit XZ direction - any input gives the same speed.
	direction.y = 0.0f;
	const float len = glm::length(direction);
	m_direction = len > 1e-6f ? direction / len : glm::vec3{0.0f};
}

void CCharacter::updateWalk()
{
	const float speed = 0.1;
	m_pKinematicCharacter->setWalkDirection(btVector3(m_direction.x, m_direction.y, m_direction.z) * speed);
}

glm::vec3 CCharacter::getOrigin() const
{
	btTransform transform = m_pGhostObject->getWorldTransform();
	btVector3 position = transform.getOrigin();
	return glm::vec3{position.x(), position.y(), position.z()};
}

void CCharacter::setbtKinematicCharacter(std::unique_ptr<btKinematicCharacterController>&& pKinematicCharacter)
{
	m_pKinematicCharacter = std::move(pKinematicCharacter);
};

btKinematicCharacterController* CCharacter::kinematicCharacter()
{
	return m_pKinematicCharacter.get();
}

void CCharacter::create()
{
	btScalar characterHeight = 1.f;
	btScalar characterRadius = 0.5f;

	m_pCapsule = std::make_unique<btCapsuleShape>(characterRadius, characterHeight);
	m_pGhostObject = std::make_unique<btPairCachingGhostObject>();

	// Spawn transform is set by CEngineImpl::addCharacter() right after
	// construction, so the ghost object's default (identity) transform here
	// is never used for simulation.
	m_pGhostObject->setCollisionShape(m_pCapsule.get());

	m_pGhostObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT | btCollisionObject::CF_KINEMATIC_OBJECT);
	m_pGhostObject->setActivationState(DISABLE_DEACTIVATION);

	btScalar stepHeight = 0.35f;

	setbtKinematicCharacter(std::make_unique<btKinematicCharacterController>(m_pGhostObject.get(), m_pCapsule.get(), stepHeight));
	// Match the dynamics world's gravity (see CEngineImpl::init) so the
	// character falls at the same rate as everything else in the world.
	kinematicCharacter()->setGravity(btVector3(0, kGravityY, 0));
	setJumpSpeed(3.1);
}

btPairCachingGhostObject* CCharacter::getGhostObject()
{
	return m_pGhostObject.get();
};

void CCharacter::setRotation(const glm::mat4& mat)
{
	m_rotation = mat;
}
glm::mat4 CCharacter::getRotation()
{
	return m_rotation;
}

glm::mat4 bulletTransformToGlm(const btTransform& transform)
{
	glm::mat4 result;
	transform.getOpenGLMatrix(glm::value_ptr(result));
	return result;
}

}