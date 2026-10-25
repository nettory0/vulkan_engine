#pragma once
#include "vk_base.h"
#include "en_graphics_pipeline.h"
#include "en_context.h"
#include "en_ray_tracing.h"
#include "en_descriptor_manager.h"
#include "en_draw_object.h"
#include "en_common_utils.h"
#include "en_render_resource.h"
#include "co_types.h"
#include "interface_types.h"

namespace en_model
{
struct SVertexData;
}

struct VULKAN_ENGINE SUniformStructure
{
	SUniformStructure();
	glm::mat4 view{1};
	glm::mat4 projection{1};
	glm::vec4 m_cameraPosition = {0, 0, 0, 1};
	alignas(16) uint32_t frameCount;
};

struct SConfig
{
	glm::mat4 transform;
	int matrixCount;
};

struct SRenderObject2D
{
	common::entityId64 m_id;
	uint32_t m_meshId{0};
};

struct SRenderObject3D
{
	common::entityId64 m_id;
	uint32_t m_meshId{0};
	glm::mat4 model;
};

struct SRenderSkinnedObject3D
{
	common::entityId64 m_id;
	uint32_t m_meshId{0};
	glm::mat4 model;
	std::vector<glm::mat4> bones;
};

struct SComputeInfo
{
	glm::uvec2 m_mouseCoord{0};
	glm::vec2 m_screenSize{0};
	glm::vec3 m_cameraPos{0};
	uint32_t m_totalTriangles{0};
	common::entityId64 m_computeEntity{0};
};

struct SInterface
{
	vk_structs::CDescriptorBufferResource* m_pDescriptor{nullptr};
	vk_structs::CBuffer* m_pBuffer{nullptr};
	en_vk::SDraw* m_drawInfo;
	glm::vec2 m_offset{0,0};
};

struct SFrameRenderData
{
	std::vector<common::entityId64> m_aSelected;
	common::entityId64 m_highlighted{0};
	SComputeInfo m_computeInfo;
	en_vk_draw::SUniformBufferObject m_camera;
	std::vector<SRenderObject2D> m_a2dMeshes;
	std::vector<SRenderObject2D> m_a2dMeshesUI;
	std::vector<SRenderObject3D> m_aStaticMeshes;
	std::vector<SRenderSkinnedObject3D> m_aSkinnedMeshes;
	std::vector<SInterface> m_aInterface;
	common::entityId64 m_cameraId{0};
};

class CPipelineSystem
{
public:
	void createPipeRayTracing(VkDevice device);
	void createPipe3d(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details);
	void clear();
public:
	en_vk::CPipelineManager m_graphicPipelineManager;
	en_vk::CPipelineLayoutManager m_graphicPipelineLayoutManager;
};


class VULKAN_ENGINE CGpuUploadSystem
{
public:
	CGpuUploadSystem();
	void initialize(en_vk::CRenderResources* resources, en_vk::CVulkanContext* context, vk_structs::CCommandPool* commandPool);
	void uploadMesh(uint32_t meshId, const en_model::SVertexData& dataSource);
	void uploadTexture(uint32_t textureId, const std::vector<unsigned char>& pixels, int texWidth, int texHeight, int texChannels);
	uint32_t uploadTextureRt(uint32_t textureId, const VkFormat& format, VkImageLayout imageLayout);

private:
	en_vk::CRenderResources* m_resources{nullptr};
	en_vk::CVulkanContext* m_context{nullptr};
	vk_structs::CCommandPool* m_commandPool{nullptr};
};

struct SUiContent
{
	vk_structs::CBuffer m_vertex;
	vk_structs::CTexture m_texture;
	vk_structs::CDescriptorBufferResource m_descriptor;
};

class VULKAN_ENGINE CRenderSystem
{
public:
	CRenderSystem();
	~CRenderSystem();
	void renderFrame(const SFrameRenderData& data, bool bUpdateBufferFrame);
	uint32_t rayPick(const SFrameRenderData& data);
	void initializeRaster(VkInstance instance, VkSurfaceKHR surface);
	void initializeRayTracing(VkInstance instance, VkSurfaceKHR surface, const std::vector<uint32_t>& indexVertex, const std::vector<en_vk::SVertexRT>& points);

	void clear();

	void recreateSwapchain();
	void destroy();
	void initializeDescriptors(const SComputeInit& initCompute, common::entityId64 camera, en_vk_utils::EPipelineLayoutType type, size_t bufferSize);
	void renderRayTracingFrame(bool bUpdateBufferFrame, size_t sizeUbo, const void* pDataUbo);
	void addEntityDescriptorSetInterface(common::entityId64 idObject, en_vk_utils::EPipelineLayoutType layout, uint32_t textureId);
	void addEntityDescriptorSetStatic(common::entityId64 idObject, en_vk_utils::EPipelineLayoutType layout, uint32_t textureId);
	void addEntityDescriptorSetAnimated(common::entityId64 idObject, en_vk_utils::EPipelineLayoutType layout, size_t bufferSize2, uint32_t textureId);
	void getMeshInfo(uint32_t meshId, SComputeInit& result, SMeshDesc& meshInfo);
	uint32_t getTriangleSize(uint32_t meshId);
	CGpuUploadSystem& uploadSystem();
	void createTexture(vk_structs::CTexture& texture, const commons_struct::STexture& textureSource);
	void createGeometryBuffer(vk_structs::CBuffer& buffer, const commons_struct::SIndexedVx& m_pGeometry);
	void createDescriptor(vk_structs::CDescriptorBufferResource& descriptor, const vk_structs::CTexture& texture);
	void waitIdle(); //TODO: remove
private:
	void updateUbo(const SFrameRenderData& data, int frame);
	void drawSkinnedMeshes(vk_base::CCommandBuffer& command, const int counter, const SFrameRenderData& frameInfo);
	void beginRendering(vk_base::CCommandBuffer& command, VkRect2D renderArea, vk_structs::SRenderTargets& framebuffer, uint32_t imageIndex);
	void endRendering(vk_base::CCommandBuffer& command, vk_structs::SRenderTargets& framebuffer, uint32_t imageIndex);
	void rayTracingDescriptors(const std::vector<uint32_t>& indexVertex, const std::vector<en_vk::SVertexRT>& points);
	void drawLayer2d(vk_base::CCommandBuffer& command, const int counter, const SFrameRenderData& frameInfo);
	void drawLayer2dMenu(vk_base::CCommandBuffer& command, const int counter, const SFrameRenderData& frameInfo);
	void drawStaticMeshes(vk_base::CCommandBuffer& command, const int counter, common::entityId64 selectMeshId, const std::vector<common::entityId64>& aSelected, const SFrameRenderData& frameInfo);
	void recordMeshDrawCommands(vk_base::CCommandBuffer& command, const VkPipelineLayout& pipeline, const std::vector<VkDescriptorBufferBindingInfoEXT>& aDescriptors, uint32_t meshId);
	void createSecond(bool bRt);
	void addTextureDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorType descriptorType, uint32_t textureId, common::entityId64 id, uint32_t index);
	void addBufferDescriptor(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorType descriptorType, uint32_t uniformBufferSize2, common::entityId64 id, uint32_t index);
private:
	en_vk::CVulkanContext m_context;
	CPipelineSystem m_pipelineManager;
	en_vk::CRenderResources m_renderResources;
	vk_structs::CCommandPool m_commandPool;
	en_vk::CSwapChainFrameSystem m_frameSystem;
	en_vk::CRayTracingSystem m_rayTracing{m_context};
	en_vk::CComputeSystem m_computeSystem;
	CGpuUploadSystem m_uploadSystem;
	uint32_t m_currentFrameInd{0};
	bool m_bNeedUpdateFrame{true};
	bool m_bNeedUpdate{false};
	bool m_bFramebufferResized{false};
	bool m_bRayTracing{false};
	std::map<int, std::map<void*, SUiContent>> m_interfaceBuffer;

};