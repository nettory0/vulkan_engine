#include "en_graphics_pipeline.h"
#include "co_geom_primitives.h"
#include "en_gl_utils.h"
#include "en_animation_structs.h"
#include "en_gl_descriptor_manager.h"

namespace en_vk
{

void CPipelineManager::clear()
{
	for(auto& pipe : m_aPipelines)
		pipe.destroy();
}
void CPipelineManager::createRayTracingPipeline(VkDevice device, VkPipelineLayout layout)
{
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> rayTracingShaderGroupCreateInfoList = {
{vk_init::rayTracingShaderGroupCreateInfoKHR(nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR, VK_SHADER_UNUSED_KHR, 0, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, nullptr)},
{vk_init::rayTracingShaderGroupCreateInfoKHR(nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 1, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, nullptr)},
{vk_init::rayTracingShaderGroupCreateInfoKHR(nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 2, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, nullptr)},
{vk_init::rayTracingShaderGroupCreateInfoKHR(nullptr, VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, 3, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, nullptr)}
	};

	vk_structs::CPipelineBuilder configRt{device};
	configRt.setRayTracingShaderGroupCreateInfoList(rayTracingShaderGroupCreateInfoList);
	configRt.addShaderStageCreateInfo(0, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, en_vk_utils::readFile2({"shaders/RayTracing/shader.rchit.spv"}), "main", nullptr);
	configRt.addShaderStageCreateInfo(0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, en_vk_utils::readFile2({"shaders/RayTracing/shader.rgen.spv"}), "main", nullptr);
	configRt.addShaderStageCreateInfo(0, VK_SHADER_STAGE_MISS_BIT_KHR, en_vk_utils::readFile2({"shaders/RayTracing/shader.rmiss.spv"}), "main", nullptr);
	configRt.addShaderStageCreateInfo(0, VK_SHADER_STAGE_MISS_BIT_KHR, en_vk_utils::readFile2({"shaders/RayTracing/shader.shadow.rmiss.spv"}), "main", nullptr);
	configRt.setLayout(layout);//

	getPipeline(en_vk_utils::EPipelineType::eRt).createRayTracing(device, configRt.initPipelineCreateInfoRT());
}
void CPipelineManager::createPipeline3D(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout)
{
	VkVertexInputAttributeDescription vertexInputAttributeDescription{vk_init::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, pos))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription1{vk_init::vertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, normal))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription2{vk_init::vertexInputAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, color))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription3{vk_init::vertexInputAttributeDescription(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(commons_struct::SVertex, texCoord))};

	vk_structs::CPipelineBuilder config3d{device};
	config3d.addShaderStageCreateInfo(0, VK_SHADER_STAGE_VERTEX_BIT, en_vk_utils::readFile2({"shaders/shader.vert.spv"}), "main", nullptr); //shader3d //"shaders/shader.vert.spv"
	config3d.addShaderStageCreateInfo(0, VK_SHADER_STAGE_FRAGMENT_BIT, en_vk_utils::readFile2({"shaders/shader.frag.spv"}), "main", nullptr);
	config3d.addAttributeDescription(vertexInputAttributeDescription);
	config3d.addAttributeDescription(vertexInputAttributeDescription1);
	config3d.addAttributeDescription(vertexInputAttributeDescription2);
	config3d.addAttributeDescription(vertexInputAttributeDescription3);
	config3d.setBindingDescription(0, sizeof(commons_struct::SVertex), VK_VERTEX_INPUT_RATE_VERTEX);
	config3d.set(details, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
	config3d.setRenderPass(renderPass);
	config3d.setLayout(layout);// );

	VkFormat colorFormat = details.m_swapChainConfig.m_surfaceFormat.format;
	VkFormat depthFormat = details.m_supportedFormat;

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &colorFormat;
	renderingInfo.depthAttachmentFormat = depthFormat;



	getPipeline(en_vk_utils::EPipelineType::e3d).create(device, config3d.initPipelineCreateInfo(&renderingInfo));//&renderingInfo
}

void CPipelineManager::createPipeline2D(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout)
{

	VkVertexInputAttributeDescription vertexInputAttributeDescription{vk_init::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, pos))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription1{vk_init::vertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, normal))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription2{vk_init::vertexInputAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, color))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription3{vk_init::vertexInputAttributeDescription(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(commons_struct::SVertex, texCoord))};


	vk_structs::CPipelineBuilder config2dLines{device};
	config2dLines.addShaderStageCreateInfo(0, VK_SHADER_STAGE_VERTEX_BIT, en_vk_utils::readFile2({"shaders/2d/shader1.vert.spv"}), "main", nullptr);
	config2dLines.addShaderStageCreateInfo(0, VK_SHADER_STAGE_FRAGMENT_BIT, en_vk_utils::readFile2({"shaders/2d/shader1.frag.spv"}), "main", nullptr);
	config2dLines.addAttributeDescription(vertexInputAttributeDescription);
	config2dLines.addAttributeDescription(vertexInputAttributeDescription1);
	config2dLines.addAttributeDescription(vertexInputAttributeDescription2);
	config2dLines.addAttributeDescription(vertexInputAttributeDescription3);
	config2dLines.setBindingDescription(0, sizeof(commons_struct::SVertex), VK_VERTEX_INPUT_RATE_VERTEX);
	config2dLines.set(details, msaaSamples, VK_PRIMITIVE_TOPOLOGY_LINE_LIST, false);
	config2dLines.setRenderPass(renderPass);
	config2dLines.setLayout(layout);

	VkFormat colorFormat = details.m_swapChainConfig.m_surfaceFormat.format;
	VkFormat depthFormat = details.m_supportedFormat;

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &colorFormat;
	renderingInfo.depthAttachmentFormat = depthFormat;


	getPipeline(en_vk_utils::EPipelineType::e2dLines).create(device, config2dLines.initPipelineCreateInfo(&renderingInfo));//&renderingInfo


	vk_structs::CPipelineBuilder config2d{device};
	config2d.addShaderStageCreateInfo(0, VK_SHADER_STAGE_VERTEX_BIT, en_vk_utils::readFile2({"shaders/2d/shader1.vert.spv"}), "main", nullptr);
	config2d.addShaderStageCreateInfo(0, VK_SHADER_STAGE_FRAGMENT_BIT, en_vk_utils::readFile2({"shaders/2d/shader1.frag.spv"}), "main", nullptr);
	config2d.addAttributeDescription(vertexInputAttributeDescription);
	config2d.addAttributeDescription(vertexInputAttributeDescription1);
	config2d.addAttributeDescription(vertexInputAttributeDescription2);
	config2d.addAttributeDescription(vertexInputAttributeDescription3);
	config2d.setBindingDescription(0, sizeof(commons_struct::SVertex), VK_VERTEX_INPUT_RATE_VERTEX);
	config2d.set(details, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
	config2d.setRenderPass(renderPass);
	config2d.setLayout(layout);// m_aPipelineLayout[en_vk::EPipelineLayout::e2d].layout());

	getPipeline(en_vk_utils::EPipelineType::e2d).create(device, config2d.initPipelineCreateInfo(&renderingInfo));//&renderingInfo

}

void CPipelineManager::createPipeline2DInterface(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout)
{

	VkVertexInputAttributeDescription vertexInputAttributeDescription{vk_init::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, pos))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription1{vk_init::vertexInputAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, normal))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription2{vk_init::vertexInputAttributeDescription(2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(commons_struct::SVertex, color))};
	VkVertexInputAttributeDescription vertexInputAttributeDescription3{vk_init::vertexInputAttributeDescription(3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(commons_struct::SVertex, texCoord))};


	VkFormat colorFormat = details.m_swapChainConfig.m_surfaceFormat.format;
	VkFormat depthFormat = details.m_supportedFormat;

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &colorFormat;
	renderingInfo.depthAttachmentFormat = depthFormat;

	vk_structs::CPipelineBuilder config2d{device};
	config2d.addShaderStageCreateInfo(0, VK_SHADER_STAGE_VERTEX_BIT, en_vk_utils::readFile2({"shaders/ui/shader_interface.vert.spv"}), "main", nullptr);
	config2d.addShaderStageCreateInfo(0, VK_SHADER_STAGE_FRAGMENT_BIT, en_vk_utils::readFile2({"shaders/ui/shader_interface.frag.spv"}), "main", nullptr);
	config2d.addAttributeDescription(vertexInputAttributeDescription);
	config2d.addAttributeDescription(vertexInputAttributeDescription1);
	config2d.addAttributeDescription(vertexInputAttributeDescription2);
	config2d.addAttributeDescription(vertexInputAttributeDescription3);
	config2d.setBindingDescription(0, sizeof(commons_struct::SVertex), VK_VERTEX_INPUT_RATE_VERTEX);
	config2d.set(details, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, true);
	config2d.setRenderPass(renderPass);
	config2d.setLayout(layout);// m_aPipelineLayout[en_vk::EPipelineLayout::e2d].layout());

	getPipeline(en_vk_utils::EPipelineType::e2dInterface).create(device, config2d.initPipelineCreateInfo(&renderingInfo));//&renderingInfo

}

void CPipelineManager::createPipeline3DAnimated(VkDevice device, VkSampleCountFlagBits msaaSamples, VkRenderPass renderPass, const vk_structs::SSupportDetails& details, VkPipelineLayout layout)
{

	vk_structs::CPipelineBuilder config3dAnimated{device};
	config3dAnimated.addShaderStageCreateInfo(0, VK_SHADER_STAGE_VERTEX_BIT, en_vk_utils::readFile2({"shaders/anim_model.vert.spv"}), "main", nullptr);
	config3dAnimated.addShaderStageCreateInfo(0, VK_SHADER_STAGE_FRAGMENT_BIT, en_vk_utils::readFile2({"shaders/anim_model.frag.spv"}), "main", nullptr);
	config3dAnimated.addAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(en_model::SVertexBone, pos));
	config3dAnimated.addAttributeDescription(1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(en_model::SVertexBone, normal));
	config3dAnimated.addAttributeDescription(2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(en_model::SVertexBone, tex));
	config3dAnimated.addAttributeDescription(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(en_model::SVertexBone, color));
	config3dAnimated.addAttributeDescription(4, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(en_model::SVertexBone, tangent));
	config3dAnimated.addAttributeDescription(5, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(en_model::SVertexBone, bitangent));
	config3dAnimated.addAttributeDescription(6, 0, VK_FORMAT_R32G32B32A32_SINT, offsetof(en_model::SVertexBone, m_boneIDs));
	config3dAnimated.addAttributeDescription(7, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(en_model::SVertexBone, m_weights));
	config3dAnimated.setBindingDescription(0, sizeof(en_model::SVertexBone), VK_VERTEX_INPUT_RATE_VERTEX);
	config3dAnimated.set(details, msaaSamples, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
	config3dAnimated.setRenderPass(renderPass);
	config3dAnimated.setLayout(layout);// m_aPipelineLayout[en_vk::EPipelineLayout::e3dAnimation].layout());


	VkFormat colorFormat = details.m_swapChainConfig.m_surfaceFormat.format;
	VkFormat depthFormat = details.m_supportedFormat;

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachmentFormats = &colorFormat;
	renderingInfo.depthAttachmentFormat = depthFormat;

	getPipeline(en_vk_utils::EPipelineType::e3dAnimation).create(device, config3dAnimated.initPipelineCreateInfo(&renderingInfo)); //&renderingInfo
}
void CPipelineManager::createPipelinesCompute(VkDevice device, VkPipelineLayout layout)
{

	vk_structs::CPipelineBuilder configCompute{device};
	configCompute.addShaderStageCreateInfo(0, VK_SHADER_STAGE_COMPUTE_BIT, en_vk_utils::readFile2({"shaders/Compute/shader.comp.spv"}), "main", nullptr);
	configCompute.setLayout(layout);//m_aPipelineLayout[en_vk::EPipelineLayout::eCompute].layout());
	getPipeline(en_vk_utils::EPipelineType::eCompute).createCompute(device, configCompute.initPipelineCreateInfoCompute());
}

VkPipeline CPipelineManager::pipeline(en_vk_utils::EPipelineType pipe) const
{
	return m_aPipelines[static_cast<size_t>(pipe)].m_pipeline.handle();
}

vk_structs::SPipeline& CPipelineManager::getPipeline(en_vk_utils::EPipelineType type)
{
	return m_aPipelines[static_cast<size_t>(type)];
}

const vk_structs::CPipelineLayout& CPipelineLayoutManager::getLayout(en_vk_utils::EPipelineLayoutType type)
{
	return m_aPipelineLayout[static_cast<size_t>(type)];
}

void CPipelineLayoutManager::clear()
{
	for(auto& layout : m_aPipelineLayout)
		layout.destroy();
}

void CPipelineLayoutManager::createRayTracingLayout(VkDevice device)
{
	vk_structs::CPipelineLayoutBuilder layoutConfig;
	layoutConfig.addDescriptorSetLayoutBinding(0, 0, VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr);
	layoutConfig.addDescriptorSetLayoutBinding(0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr);
	layoutConfig.addDescriptorSetLayoutBinding(0, 2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr);
	layoutConfig.addDescriptorSetLayoutBinding(0, 3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr);
	layoutConfig.addDescriptorSetLayoutBinding(0, 4, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, nullptr);

	layoutConfig.addDescriptorSetLayoutBinding(1, 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr);
	layoutConfig.addDescriptorSetLayoutBinding(1, 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, nullptr);
	layoutConfig.setLayoutCreateInfo();

	m_aPipelineLayout[static_cast<size_t>(en_vk_utils::EPipelineLayoutType::eRt)].create(device, layoutConfig.m_aLayoutInfo, layoutConfig.m_aPushConstant);
	

	for(auto& [index, descriptorType] : layoutConfig.m_mapDescriptorType)
		m_aPipelineLayoutSchema[{en_vk_utils::EPipelineLayoutType::eRt, index}] = descriptorType;

	getDescriptorSetLayouts();
}

void CPipelineLayoutManager::createGraphicsPipelineLayouts(VkDevice device)
{
	constexpr int max_buffer_size = 1024;
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = vk_init::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr);
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding2 = vk_init::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr);
	//VkDescriptorSetLayoutBinding descriptorSetLayoutBinding3 = vk_init::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingCompute0 = vk_init::descriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_buffer_size, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingCompute1 = vk_init::descriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_buffer_size, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingCompute2 = vk_init::descriptorSetLayoutBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_buffer_size, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingCompute3 = vk_init::descriptorSetLayoutBinding(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindingCompute4 = vk_init::descriptorSetLayoutBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, max_buffer_size, VK_SHADER_STAGE_COMPUTE_BIT, nullptr);

	vk_structs::CPipelineLayoutBuilder layoutConfig3d;
	layoutConfig3d.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4));
	layoutConfig3d.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBinding);
	layoutConfig3d.addDescriptorSetLayoutBinding(1, descriptorSetLayoutBinding2);
	layoutConfig3d.setLayoutCreateInfo();

	vk_structs::CPipelineLayoutBuilder layoutConfig3dAnimated;
	layoutConfig3dAnimated.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4));
	layoutConfig3dAnimated.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBinding);
	layoutConfig3dAnimated.addDescriptorSetLayoutBinding(1, descriptorSetLayoutBinding2);
	layoutConfig3dAnimated.addDescriptorSetLayoutBinding(2, descriptorSetLayoutBinding);
	layoutConfig3dAnimated.setLayoutCreateInfo();

	vk_structs::CPipelineLayoutBuilder layoutConfig2d;
	layoutConfig2d.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBinding);
	layoutConfig2d.addDescriptorSetLayoutBinding(1, descriptorSetLayoutBinding2);
	layoutConfig2d.setLayoutCreateInfo();

	vk_structs::CPipelineLayoutBuilder layoutConfig2dInterface;
	layoutConfig2dInterface.addPushConstant(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec4));
	layoutConfig2dInterface.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBinding2);
	layoutConfig2dInterface.setLayoutCreateInfo();

	vk_structs::CPipelineLayoutBuilder layoutConfigCompute;
	layoutConfigCompute.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBindingCompute0);
	layoutConfigCompute.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBindingCompute1);
	layoutConfigCompute.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBindingCompute2);
	layoutConfigCompute.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBindingCompute3);
	layoutConfigCompute.addDescriptorSetLayoutBinding(0, descriptorSetLayoutBindingCompute4);
	layoutConfigCompute.setLayoutCreateInfo();

	m_aPipelineLayout[static_cast<size_t>(en_vk_utils::EPipelineLayoutType::e3d)].create(device, layoutConfig3d.m_aLayoutInfo, layoutConfig3d.m_aPushConstant);
	m_aPipelineLayout[static_cast<size_t>(en_vk_utils::EPipelineLayoutType::e3dAnimation)].create(device, layoutConfig3dAnimated.m_aLayoutInfo, layoutConfig3dAnimated.m_aPushConstant);
	m_aPipelineLayout[static_cast<size_t>(en_vk_utils::EPipelineLayoutType::e2d)].create(device, layoutConfig2d.m_aLayoutInfo, layoutConfig2d.m_aPushConstant);
	m_aPipelineLayout[static_cast<size_t>(en_vk_utils::EPipelineLayoutType::e2dInterface)].create(device, layoutConfig2dInterface.m_aLayoutInfo, layoutConfig2dInterface.m_aPushConstant);
	m_aPipelineLayout[static_cast<size_t>(en_vk_utils::EPipelineLayoutType::eCompute)].create(device, layoutConfigCompute.m_aLayoutInfo, layoutConfigCompute.m_aPushConstant);


	for(auto& [index, descriptorType] : layoutConfig3d.m_mapDescriptorType)
		m_aPipelineLayoutSchema[{en_vk_utils::EPipelineLayoutType::e3d, index}] = descriptorType;

	for(auto& [index, descriptorType] : layoutConfig3dAnimated.m_mapDescriptorType)
		m_aPipelineLayoutSchema[{en_vk_utils::EPipelineLayoutType::e3dAnimation, index}] = descriptorType;

	for(auto& [index, descriptorType] : layoutConfig2d.m_mapDescriptorType)
		m_aPipelineLayoutSchema[{en_vk_utils::EPipelineLayoutType::e2d, index}] = descriptorType;

	for(auto& [index, descriptorType] : layoutConfig2dInterface.m_mapDescriptorType)
		m_aPipelineLayoutSchema[{en_vk_utils::EPipelineLayoutType::e2dInterface, index}] = descriptorType;

	for(auto& [index, descriptorType] : layoutConfigCompute.m_mapDescriptorType)
		m_aPipelineLayoutSchema[{en_vk_utils::EPipelineLayoutType::eCompute, index}] = descriptorType;


	getDescriptorSetLayouts();
}

const std::map<en_vk_utils::SDescriptorLayoutIndex, vk_structs::SDescriptorSetLayoutInfo>& CPipelineLayoutManager::getDescriptorSetLayoutsSchema() const
{
	return m_aPipelineLayoutSchema;
}

void CPipelineLayoutManager::getDescriptorSetLayouts()
{
	size_t counterPipelineLayout{0};
	for(auto& pipelineLayout : m_aPipelineLayout)
	{
		uint32_t counterLayout{0};
		for(auto& setLayout : pipelineLayout.descriptorSet().setLayout())
		{
			m_aPipelineLayoutSchema[{static_cast<en_vk_utils::EPipelineLayoutType>(counterPipelineLayout), counterLayout}].m_layout = setLayout.handle();
			counterLayout++;
		}
		counterPipelineLayout++;
	}
}

}