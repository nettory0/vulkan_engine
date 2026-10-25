#pragma once
#include <string>
#include <chrono>
#include "en_animation_structs.h"
#include "vk_base.h"
#include "en_utils.h"
#include "en_context.h"
#include "pe_physic_engine.h"
#include "en_draw_object.h"

#include "en_ray_tracing.h"
#include "en_compute_system.h"
#include "en_scene_component.h"
#include "en_render_system.h"
#include "en_descriptor_manager.h"
#include "en_input_system.h"
#include "en_camera.h"
#include "en_animation_system.h"
#include "en_world.h"
#include "en_select_system.h"
#include "co_structs.h"

namespace interface_menu
{
class CInterfaceSystem;
}

struct VULKAN_ENGINE SEngineInitConfig
{
	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	bool m_bEnableRayTracing{false};
};

struct SInterfaceResources
{
	std::map<void*, vk_structs::CTexture> m_aInterfaceTexture;
	std::map<void*, vk_structs::CBuffer> m_aInterfaceBuffer;
	std::map<void*, en_vk::SDraw> m_aInterfaceDrawInfo;
	std::map<void*, vk_structs::CDescriptorBufferResource> m_aInterfaceTextureDescriptors;
	std::map<void*, int> m_textureToDelete;
	std::map<void*, int> m_bufferToDelete;
	void updateToDelete();
	void addTextureToDelete(void*);
	void addBufferToDelete(void*);
	void destroy();
};

class VULKAN_ENGINE CEngine
{
public:
	CEngine();
	CEngine(const CEngine&) = delete;
	CEngine& operator=(const CEngine&) = delete;
	~CEngine();
	void initialize(const SEngineInitConfig& config);
	void destroy();

	void update(float newTime);
	void handleKeyInput(EKey key, int scancode, int action, int mods);
	void handleMouseMove(double x, double y);
	void handleScroll(double x, double y);
	void setWindowResolution(int w, int h);
	void renderRayTracingFrame(bool bUpdateBufferFrame);
	void renderSceneFrame(bool bUpdateBufferFrame, glm::uvec2 mouseCoord);
	void resizeSwapchain(int w, int h);

	const en_scene::CScene& getScene() const;
	void setSimulationWork(bool bWork);

private:
	void initializeScene(const SEngineInitConfig& config);
	void initializeRayTracing(const SEngineInitConfig& config);
	SFrameRenderData buildFrameRenderData(glm::uvec2 mouseCoord);
	void fillCameraData(SFrameRenderData& data, glm::uvec2 mouseCoord);
	void setPlayerAnimation(std::string animation, bool bLoop = true);
	void updateJump(float deltaTime);
	void initializeRenderObjects();
	void initializePhysicsWorld();
	void syncPlayerTransformWithPhysics();
	void syncCamera();
	void addDefaultTexture();
	SComputeInit descriptorCompute();
	void uploadTexture();
	void uploadMesh();
	void prepareInterface(SFrameRenderData& frameData);
private:
	uint64_t m_camera_1{common::cInvalidID};
	uint64_t m_camera_2{common::cInvalidID};
	bool m_bSimulation{true};
	CSceneComponentSystem m_sceneComponents;
	CInputSystem m_inputSystem;
	physic_en::CPhysicsEngine m_physicsEngine;
	CRenderSystem m_renderSystem;

	SInterfaceResources m_interfaceResources;
	STextureId m_defaultTexture;
	CAnimationSystem m_animationSystem{m_sceneComponents};
	CWorld m_world{m_sceneComponents};
	en_vk::CSelectSystem m_selectSystem;
	std::unique_ptr<interface_menu::CInterfaceSystem> m_pInterfaceSystem;
	float m_deltaOrientation{0};

	// Jump state: the jump animation runs from a Space press until landing.
	bool m_bJumping{false};
	bool m_bJumpLeftGround{false};
	float m_jumpTime{0.0f};
	std::string m_moveAction{"Idle"};
};