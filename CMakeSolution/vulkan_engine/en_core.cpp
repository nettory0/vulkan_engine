#include "en_core.h"
#include <stdexcept>
#include <optional>
#include <array>
#include <iostream>
#include <set>
#include <unordered_map>
#include <fstream>
#include <numbers>
#include <ranges>

#include <codecvt>
#include "en_models_path.h"
#include "interface_system.h"
#include "tg_mesh.h"
#include "co_geom_primitives.h"
#include "en_gl_descriptor_manager.h"
#include "vk_struct_builders.h"
#include "co_structs.h"
#include "co_logging.h"

CEngine::CEngine():m_pInterfaceSystem{std::make_unique<interface_menu::CInterfaceSystem>()}
{
	m_pInterfaceSystem->init();
	common::logError(std::string{"test log error"});
	common::logWarning(std::string{"test log warning"});
	common::logInfo(std::string{"test log info"});
}
CEngine::~CEngine()
{}

void CEngine::initialize(const SEngineInitConfig& config)
{
	m_camera_1 = m_sceneComponents.m_scene.addNode(m_sceneComponents.m_scene.root(), "camera");
	m_camera_2 = m_sceneComponents.m_scene.addNode(m_sceneComponents.m_scene.root(), "camera2");

	m_sceneComponents.componentManager().addComponent<CFreeCamera>(m_camera_1) = {};
	m_sceneComponents.componentManager().addComponent<COrbitCamera>(m_camera_2) = {};

	if(config.m_bEnableRayTracing)
	{
		initializeRayTracing(config);
	}
	else
		initializeScene(config);
}

void CEngine::initializeScene(const SEngineInitConfig& config)
{
	addDefaultTexture();
	m_world.loadLevel();
	m_renderSystem.initializeRaster(config.m_instance, config.m_surface);

	initializePhysicsWorld();
	m_physicsEngine.init();
	//m_camera.initialize(m_physicsEngine.characterOrigin());
	//setSimulationWork(false);
	initializeRenderObjects();

	if(auto* pCam = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1))
		pCam->initialize(m_physicsEngine.characterOrigin());
}

void CEngine::addDefaultTexture()
{
	commons_struct::STexture textureDefault;
	textureDefault.m_data = en_vk_utils::loadTexture(textureDefault.w, textureDefault.h, textureDefault.texChannels, texture_PATH_FLOOR.c_str());
	m_defaultTexture = m_sceneComponents.sceneResources().addTexture(std::move(textureDefault));
}

void CEngine::initializeRenderObjects()
{
	uploadTexture();
	uploadMesh();
	auto computeInit = descriptorCompute();

	m_renderSystem.initializeDescriptors(computeInit, m_camera_1, en_vk_utils::EPipelineLayoutType::e3d, sizeof(en_vk_draw::SUniformBufferObject));
}

SComputeInit CEngine::descriptorCompute()
{
	SComputeInit result;

	uint32_t counter{0};
	int offset = 0;
	auto map3d = m_sceneComponents.getStaticMeshes();

	if(auto mapCompute = m_sceneComponents.getCompute(); !mapCompute.empty())
		result.m_id = mapCompute[0];
	for(auto& id1 : map3d)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id1))
		{
			SMeshDesc meshInfo;
			glm::mat4 transform = m_sceneComponents.calculateTransformation(id1);

			auto mat = m_physicsEngine.getTransform(id1);
			if(mat != glm::mat4(1))
				transform = mat * transform;

			meshInfo.model = transform;
			meshInfo.modelInv = glm::inverse(transform);

			m_renderSystem.getMeshInfo(pMeshId->m_id, result, meshInfo);
			meshInfo.vertexBufferIndex = counter;
			meshInfo.indexBufferIndex = counter;
			meshInfo.triangleOffset = offset;
			meshInfo.objectId.x = (uint32_t)id1;

			result.aMeshInfo.push_back(meshInfo);
			counter++;
			offset += meshInfo.indexCount / 3;
		}
	}

	return result;
}

void CEngine::uploadTexture()
{
	if(auto* pDefaultTexture = m_sceneComponents.sceneResources().getTexture(m_defaultTexture))
		m_renderSystem.uploadSystem().uploadTexture(m_defaultTexture.m_id, pDefaultTexture->m_data, pDefaultTexture->w, pDefaultTexture->h, pDefaultTexture->texChannels);

	auto mapTextures = m_sceneComponents.componentManager().getEntitiesWithData<STextureId>();

	for(const auto& [id, textureId] : mapTextures)
	{
		if(auto* pTexture = m_sceneComponents.sceneResources().getTexture(textureId))
			m_renderSystem.uploadSystem().uploadTexture(textureId.m_id, pTexture->m_data, pTexture->w, pTexture->h, pTexture->texChannels);
	}
}

void CEngine::uploadMesh()
{
	auto aMesh = m_sceneComponents.componentManager().getEntitiesWithData<SMeshId>();

	for(auto& [idResource, meshId] : aMesh)
	{
		if(auto* pMesh = m_sceneComponents.sceneResources().getMeshBase(meshId))
		{
			m_renderSystem.uploadSystem().uploadMesh(meshId.m_id, pMesh->getVertexDataSource());

			if(m_sceneComponents.isMesh2d(idResource))
			{
				//m_renderSystem.addEntityDescriptorSetStatic(idResource, en_vk::EPipelineLayoutType::e2d, meshId.m_id, sizeof(en_vk_draw::SUniformBufferObject), 0, m_defaultTexture.m_id);
			}
			else if(m_sceneComponents.isMeshStatic(idResource))
			{
				auto* pTextureId = m_sceneComponents.componentManager().getComponentConst<STextureId>(idResource);
				m_renderSystem.addEntityDescriptorSetStatic(idResource, en_vk_utils::EPipelineLayoutType::e3d, pTextureId ? pTextureId->m_id : m_defaultTexture.m_id);

			}
			else if(m_sceneComponents.isMeshAnimated(idResource))
			{
				auto* pTextureId = m_sceneComponents.componentManager().getComponentConst<STextureId>(idResource);
				m_renderSystem.addEntityDescriptorSetAnimated(idResource, en_vk_utils::EPipelineLayoutType::e3dAnimation, sizeof(en_model::SMatBones), pTextureId ? pTextureId->m_id : m_defaultTexture.m_id);
			}
		}
	}
}

void CEngine::syncPlayerTransformWithPhysics()
{
	if(auto playerNode = m_sceneComponents.m_player)
	{
		glm::mat4 playerTransform{1};
		playerTransform[3] = glm::vec4(m_physicsEngine.characterOrigin() - glm::vec3(0, 1, 0), 1);
		m_sceneComponents.componentManager().addComponent<glm::mat4>(playerNode) = playerTransform * m_physicsEngine.characterRotation();
	}
}

void CEngine::syncCamera()
{
	auto* pCamera = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1);
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);
	if(m_bSimulation)
	{
		if(auto playerNode = m_sceneComponents.m_player)
		{
			const glm::quat rot2 = glm::quat(glm::vec3(0, glm::radians(-pCamera->orientation().x + m_deltaOrientation), 0));
			m_physicsEngine.updateRotation(glm::toMat4(rot2));
			pCamera->updatePos(m_physicsEngine.characterOrigin());
		}
	}
}

void CEngine::initializePhysicsWorld()
{
	auto mapObjects = m_sceneComponents.componentManager().getEntitiesWithData<SPhysicsObject>();
	for(auto& [id, physicsInfo] : mapObjects)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id))
		{
			switch(physicsInfo.m_type)
			{
			case EPhysicsObjectType::eLevelMesh:
			if(auto* pMesh = m_sceneComponents.sceneResources().getMeshBase(*pMeshId))
			{
				if(pMesh->type() == en_model::CMeshBase::EMeshType::eStatic)
				{
					const auto* pStaticMesh = static_cast<const en_model::CMeshStatic*>(pMesh);
					std::vector<glm::vec3> aTriangleVertices;
					for(const auto& pt : pStaticMesh->m_object.m_indices)
					{
						aTriangleVertices.push_back(pStaticMesh->m_object.m_vertices[pt].pos);
					}
					if(auto* pMatSource = m_sceneComponents.getParentNode<en_model::SMatrixSource>(id))
						m_physicsEngine.addLevelMesh(id, pMatSource->m_pos, pMatSource->m_scale, pMatSource->m_rotate, aTriangleVertices);
					else
						common::logError(std::format("initializePhysicsWorld get Can not find id {}\n", id));
				}
			}
			break;
			case EPhysicsObjectType::eSphere:
			m_physicsEngine.addSphereCollider(id, physicsInfo.origin, physicsInfo.mass, physicsInfo.radius);
			break;
			case EPhysicsObjectType::eBox:
			m_physicsEngine.addBoxCollider(id, physicsInfo.origin, physicsInfo.mass, physicsInfo.boxSize);
			break;
			default:
			break;
			}
		}
	}
}

void CEngine::update(float deltaTime)
{
	if(m_bSimulation)
	{
		m_physicsEngine.stepSimulation(deltaTime);
		m_animationSystem.update(deltaTime);
		syncPlayerTransformWithPhysics();
	}
	else
		m_animationSystem.update(0);
}

const float cameraSpeed = 1;

void CEngine::setWindowResolution(int w, int h)
{
	auto* pCamera = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1);
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);
	if(pCamera)
		pCamera->resize(w, h);
	if(pCamera2)
		pCamera2->resize(w, h);
}

void CEngine::handleScroll(double x, double y)
{
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);
	if(!m_bSimulation && pCamera2)
		pCamera2->zoom(y);
}

void CEngine::handleMouseMove(double x, double y)
{

	auto* pCamera = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1);
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);
	if(pCamera && pCamera2)
	{
		glm::vec2 deltaMove{0,0};
		auto deltaOrient = m_inputSystem.mouseMove(x, y, deltaMove);
		if(m_inputSystem.isKeyPressed(EKey::eMouseLeft))
		{
			pCamera->updateAngle(deltaOrient, true);

			pCamera2->pan(deltaMove.x, deltaMove.y);
			syncCamera();
		}
		else if(m_inputSystem.isKeyPressed(EKey::eMouseRight))
			pCamera2->rotate(deltaMove.x, deltaMove.y);

		auto frameUbo = buildFrameRenderData({x, y});
		if(auto highlighted = m_renderSystem.rayPick(frameUbo); highlighted != UINT32_MAX)
		{
			m_selectSystem.setHighlighted(highlighted);
		}
		else
			m_selectSystem.setHighlighted(common::cInvalidID);
	}
}

void CEngine::handleKeyInput(EKey key, int scancode, int action, int mods)
{
	auto* pCamera = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1);
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);
	m_inputSystem.keyCallback(key, scancode, action, mods);

	if(!m_bSimulation)
	{
		if(m_inputSystem.isKeyPressed(EKey::eMouseLeft))
		{
			m_selectSystem.setSelected(m_inputSystem.isKeyPressed(EKey::LEFT_CTRL));
			return;
		}
	}
	else
	{
		std::string playerAction = {"Idle"};
		float deltaOrientation = 0;
		glm::vec3 deltaMove{0, 0, 0};
		const float speed = 0.1;// currentTime - m_lastFrame;

		bool bW = m_inputSystem.isKeyPressed(EKey::W);
		bool bS = m_inputSystem.isKeyPressed(EKey::S);
		bool bA = m_inputSystem.isKeyPressed(EKey::A);
		bool bD = m_inputSystem.isKeyPressed(EKey::D);

		// противоположные клавиши гасят друг друга (W+S, A+D)
		if(bW && bS)
		{
			bW = false; bS = false;
		}
		if(bA && bD)
		{
			bA = false; bD = false;
		}

		if(bW)
		{
			deltaMove += cameraSpeed * speed * pCamera->front();
			playerAction = {"Running"};
		}
		if(bS)
		{
			deltaMove += -cameraSpeed * speed * pCamera->front();
			playerAction = {"Running"};
		}
		if(bA)
		{
			deltaMove += -glm::normalize(glm::cross(pCamera->front(), pCamera->up())) * speed * cameraSpeed;
			playerAction = {"Running"};
		}
		if(bD)
		{
			deltaMove += glm::normalize(glm::cross(pCamera->front(), pCamera->up())) * speed * cameraSpeed;
			playerAction = {"Running"};
		}

		if(bW && bA)       deltaOrientation = 45.0;
		else if(bW && bD)  deltaOrientation = -45.0;
		else if(bS && bA)  deltaOrientation = 135.0;
		else if(bS && bD)  deltaOrientation = -135.0;
		else if(bS)        deltaOrientation = 180.0;
		else if(bA)        deltaOrientation = 90.0;
		else if(bD)        deltaOrientation = -90.0;
		else if(bW)        deltaOrientation = 0.0;

		if(deltaMove != glm::vec3(0))
			deltaMove = glm::normalize(deltaMove);

		m_physicsEngine.setWalkDirection(deltaMove);

		if(!m_physicsEngine.canJump())
			playerAction = {"Jump2"};
		if(m_inputSystem.isKeyPressed(EKey::SPACE))// && action == GLFW_PRESS)
		{
			//if(physic->canJump())
			m_physicsEngine.jump();
		}
		pCamera->updatePos(pCamera->origin() + (deltaMove * glm::vec3(0.1)));
		setPlayerAnimation(playerAction);
		m_deltaOrientation = deltaOrientation;
		syncCamera();
	}
}


void CEngine::setPlayerAnimation(std::string animation)
{
	if(auto pPlayerNode = m_sceneComponents.m_player)
	{
		if(auto pAnimator = m_sceneComponents.componentManager().getComponentConst<std::unique_ptr<en_model::CAnimator>>(m_sceneComponents.m_scene.getChildren(pPlayerNode).front()))
		{
			auto& t = *pAnimator;
			t->setActiveAnimation(animation);
		}
	}
}

void CEngine::destroy()
{
	m_renderSystem.destroy();
}
void CEngine::resizeSwapchain(int w, int h)
{
	setWindowResolution(w, h);
	m_renderSystem.recreateSwapchain();
}

void CEngine::setSimulationWork(bool bWork)
{
	m_bSimulation = bWork;
}

const en_scene::CScene& CEngine::getScene() const
{
	return m_sceneComponents.m_scene;
}

void CEngine::prepareInterface(SFrameRenderData& frameData)
{
	m_interfaceResources.updateToDelete();
	for(auto& renderInterface : m_pInterfaceSystem->getDrawQueue())
	{
		if(renderInterface.m_pTexture != nullptr)
		{
			if(auto it = m_interfaceResources.m_aInterfaceTexture.find(renderInterface.m_pTexture); it == m_interfaceResources.m_aInterfaceTexture.end())
			{
				auto& texture = m_interfaceResources.m_aInterfaceTexture[renderInterface.m_pTexture];
				m_renderSystem.createTexture(texture, *renderInterface.m_pTexture);

				auto& descriptor = m_interfaceResources.m_aInterfaceTextureDescriptors[renderInterface.m_pTexture];
				m_renderSystem.createDescriptor(descriptor, texture);
			}

			if(auto it = m_interfaceResources.m_aInterfaceBuffer.find(renderInterface.m_pGeometry); it == m_interfaceResources.m_aInterfaceBuffer.end())
			{
				auto& buffer = m_interfaceResources.m_aInterfaceBuffer[renderInterface.m_pGeometry];
				m_renderSystem.createGeometryBuffer(buffer, *renderInterface.m_pGeometry);

				auto& drawInfo = m_interfaceResources.m_aInterfaceDrawInfo[renderInterface.m_pGeometry];
				drawInfo.m_sizeIndexes = renderInterface.m_pGeometry->m_aIndices.size();
				drawInfo.m_sizeVertices = renderInterface.m_pGeometry->m_aVx.size();
				drawInfo.m_structSize = sizeof(renderInterface.m_pGeometry->m_aVx[0]);
			}

			frameData.m_aInterface.push_back({&m_interfaceResources.m_aInterfaceTextureDescriptors[renderInterface.m_pTexture],
											 & m_interfaceResources.m_aInterfaceBuffer[renderInterface.m_pGeometry],
											 & m_interfaceResources.m_aInterfaceDrawInfo[renderInterface.m_pGeometry],
											 renderInterface.m_translation});
		}
	}
	
	for(auto& [pTexture, texture] : m_interfaceResources.m_aInterfaceTexture)
		if(auto it = m_pInterfaceSystem->avalibelTextures().find((commons_struct::STexture*)pTexture); it == m_pInterfaceSystem->avalibelTextures().end())
			m_interfaceResources.addTextureToDelete(pTexture);

	for(auto& [pBuffer, buffer] : m_interfaceResources.m_aInterfaceBuffer)
		if(auto it = m_pInterfaceSystem->avalibelGeometry().find((commons_struct::SIndexedVx*)pBuffer); it == m_pInterfaceSystem->avalibelGeometry().end())
			m_interfaceResources.addBufferToDelete(pBuffer);
}

void CEngine::renderSceneFrame(bool bUpdateBufferFrame, glm::uvec2 mouseCoord)
{
	m_pInterfaceSystem->update();
	m_pInterfaceSystem->render();
	auto frameUbo = buildFrameRenderData(mouseCoord);

	m_renderSystem.renderFrame(frameUbo, bUpdateBufferFrame);
}

SFrameRenderData CEngine::buildFrameRenderData(glm::uvec2 mouseCoord)
{
	uint32_t totalTriangles{0};
	auto mapStatic = m_sceneComponents.getStaticMeshes();
	auto mapAnimated = m_sceneComponents.getAnimatedMeshes();
	auto map2d = m_sceneComponents.get2dMeshes();
	auto map2dUI = m_sceneComponents.getUIMeshes();
	for(auto& id : mapStatic)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id))
		{
			SMeshDesc meshInfo;
			{
				meshInfo.model = m_sceneComponents.calculateTransformation(id);

				auto mat = m_physicsEngine.getTransform(id);
				if(mat != glm::mat4(1))
					meshInfo.model = mat * meshInfo.model;
				meshInfo.modelInv = glm::inverse(meshInfo.model);
			}
			totalTriangles += m_renderSystem.getTriangleSize(pMeshId->m_id) / 3;
		}
	}


	SFrameRenderData data;
	data.m_cameraId = m_camera_1;
	auto* pCamera = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1);
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);
	if(m_bSimulation)
	{
		data.m_camera.proj = pCamera->project();
		data.m_camera.view = pCamera->view();
		data.m_computeInfo.m_cameraPos = pCamera->position();
		data.m_computeInfo.m_screenSize = glm::vec2(pCamera->width(), pCamera->height());
	}
	else
	{
		data.m_camera.proj = pCamera2->project();
		data.m_camera.view = pCamera2->view();
		data.m_computeInfo.m_cameraPos = pCamera2->position();
		data.m_computeInfo.m_screenSize = glm::vec2(pCamera2->width(), pCamera2->height());

	}

	if(auto aCompute = m_sceneComponents.getCompute(); !aCompute.empty())
		data.m_computeInfo.m_computeEntity = aCompute[0];
	else
		data.m_computeInfo.m_computeEntity = common::cInvalidID;

	data.m_computeInfo.m_mouseCoord = mouseCoord;
	data.m_computeInfo.m_totalTriangles = totalTriangles;
	for(auto& id1 : mapStatic)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id1))
		{
			glm::mat4 transform = m_sceneComponents.calculateTransformation(id1);

			auto mat = m_physicsEngine.getTransform(id1);
			if(mat != glm::mat4(1))
				transform = mat * transform;

			data.m_aStaticMeshes.push_back({});
			data.m_aStaticMeshes.back().m_id = id1;
			data.m_aStaticMeshes.back().m_meshId = pMeshId->m_id;
			data.m_aStaticMeshes.back().model = transform;
		}
	}

	for(auto& id1 : mapAnimated)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id1))
		{
			en_model::SMatBones tmp;
			data.m_aSkinnedMeshes.push_back({});
			data.m_aSkinnedMeshes.back().m_id = id1;
			data.m_aSkinnedMeshes.back().m_meshId = pMeshId->m_id;
			data.m_aSkinnedMeshes.back().model = m_sceneComponents.calculateTransformation(id1);

			if(auto* pAnimator = m_sceneComponents.getParentNode<std::unique_ptr<en_model::CAnimator>>(id1))
			{
				auto& aAnimation = pAnimator->get()->m_finalBoneMatrices;
				data.m_aSkinnedMeshes.back().bones.reserve(aAnimation.size());
				for(int t = 0; t < aAnimation.size(); t++)
					data.m_aSkinnedMeshes.back().bones.push_back(aAnimation[t]);
			}
			else
			{
				for(auto& mat : tmp.bones)
					data.m_aSkinnedMeshes.back().bones.push_back(glm::mat4(1));
			}
		}
	}

	for(auto& id : map2d)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id))
			data.m_a2dMeshes.push_back({id, pMeshId->m_id});
	}

	for(auto& id : map2dUI)
	{
		if(auto* pMeshId = m_sceneComponents.componentManager().getComponentConst<SMeshId>(id))
			data.m_a2dMeshesUI.push_back({id, pMeshId->m_id});
	}

	data.m_highlighted = m_selectSystem.getHighlighted();
	data.m_aSelected = m_selectSystem.getSelected();


	prepareInterface(data);

	return data;
}

void CEngine::initializeRayTracing(const SEngineInitConfig& config)
{
	en_model::CModel model{modelPath7RT, false};

	std::vector<uint32_t> indexVertex;
	std::vector<en_vk::SVertexRT> points;
	uint32_t counter{0};
	for(auto& mesh : model.m_aMeshes)
	{
		auto* pStaticMesh = static_cast<en_model::CMeshStatic*>(mesh.get());
		for(auto pIndex : pStaticMesh->m_object.m_indices)
		{
			indexVertex.push_back(points.size());
			points.push_back({});// mesh.m_object.m_vertices[pIndex]);
			points.back().pos = pStaticMesh->m_object.m_vertices[pIndex].pos;
			points.back().normal = pStaticMesh->m_object.m_vertices[pIndex].normal;
		}
	}
	m_renderSystem.initializeRayTracing(config.m_instance, config.m_surface, indexVertex, points);
}

void CEngine::renderRayTracingFrame(bool bUpdateBufferFrame)
{
	auto* pCamera = m_sceneComponents.componentManager().getComponent<CFreeCamera>(m_camera_1);
	auto* pCamera2 = m_sceneComponents.componentManager().getComponent<COrbitCamera>(m_camera_2);

	en_vk_draw::SUniformStructure ubo;
	ubo.projection = pCamera->projectRt();
	ubo.view = pCamera->viewRt();

	m_renderSystem.renderRayTracingFrame(bUpdateBufferFrame, sizeof(en_vk_draw::SUniformStructure), &ubo);
}

void SInterfaceResources::updateToDelete()
{
	std::erase_if(m_textureToDelete, [&](auto& item)
	{
		auto& [pTexture, counter] = item;

		--counter;

		if(counter < 0)
		{
			m_aInterfaceTexture.erase(pTexture);
			m_aInterfaceTextureDescriptors.erase(pTexture);
			return true;
		}

		return false;
	});

	std::erase_if(m_buferToDelete, [&](auto& item)
	{
		auto& [pBuffer, counter] = item;

		--counter;

		if(counter < 0)
		{
			m_aInterfaceBuffer.erase(pBuffer);
			m_aInterfaceDrawInfo.erase(pBuffer);
			return true;
		}

		return false;
	});
}

void SInterfaceResources::addTextureToDelete(void* pTexture)
{
	m_textureToDelete[pTexture] = 2;
}

void SInterfaceResources::addBufferToDelete(void* pBuffer)
{
	m_textureToDelete[pBuffer] = 2;
}
