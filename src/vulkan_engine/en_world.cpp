#include <array>
#include <iostream>
#include <set>
#include <unordered_map>
#include <fstream>
#include <numbers>
#include <ranges>
#include <codecvt>

#include "en_world.h"
#include "en_scene_component.h"

#include "vk_struct_builders.h"
#include "tg_mesh.h"
#include "co_geom_primitives.h"

#include "en_descriptor_manager.h"
#include "en_models_path.h"
#include "co_structs.h"

glm::mat4 createMatrix1(glm::vec3 pos, glm::vec3 scale, glm::vec3 rotate, float angle)
{
	float angleRad = glm::radians(angle);

	glm::vec3 rotationAxis = rotate;
	glm::mat4 result(1);

	result = glm::translate(result, pos);
	result = glm::rotate(result, angleRad, rotationAxis);
	result = glm::scale(result, scale);
	return result;
}

static commons_struct::SIndexedVx genIndexed1(const std::vector<commons_struct::SVertex>& vertexIn)
{
	commons_struct::SIndexedVx result;
	std::unordered_map<commons_struct::SVertex, uint32_t> aVertex;
	uint32_t counter{0};
	result.m_aIndices.reserve(vertexIn.size());
	result.m_aVx.reserve(vertexIn.size());
	for(const auto& vertex : vertexIn)
	{
		auto [pair, bRes] = aVertex.try_emplace(vertex, counter);
		if(bRes)
		{
			result.m_aVx.push_back(vertex);
			counter++;
		}
		result.m_aIndices.push_back(pair->second);
	}
	return result;
}

CWorld::CWorld(CSceneComponentSystem& scene): m_scene{scene}
{
	m_textSystem.m_textAssistant.init2();
}

void CWorld::loadLevel()
{
	initDrawElements3d();
	initDrawElements2d();
	m_scene.addCompute();
}
void CWorld::spawnPlayer(const asset::SLoadModel& playerModel)
{

}

void CWorld::addCubes()
{
	auto root = m_scene.m_scene.root();

	m_scene.addCube(root, glm::vec3(glm::vec3(0, 0, 0)), 0, {15, 1, 15});
	m_scene.addCube(root, glm::vec3(glm::vec3(31, 0, 0)), 0, {15, 1, 15});
}

void CWorld::addModelsAnimated(const std::vector<asset::SLoadModel>& aModels)
{
	auto pNodeModelParent = m_scene.addTransformNode(m_scene.m_scene.root(), glm::mat4(1), "Model");


	for(const auto& model : aModels)
		addModel(pNodeModelParent, model, false, true);
}

void CWorld::addModelsLevel(const std::vector<asset::SLoadModel>& aModels)
{
	auto pNodeLevelParent = m_scene.addTransformNode(m_scene.m_scene.root(), glm::mat4(1), "Level");

	for(const auto& model : aModels)
		addModel(pNodeLevelParent, model, true, false);
}

void CWorld::initDrawElements2d()
{
	float fontHeight{0.1};
	float fontHeight2{0.05};
	float fontHeight3{0.2};

	auto pNodeParent = m_scene.addNode(m_scene.m_scene.root(), "Words");
	addWord(pNodeParent, fontHeight, std::string{"Player"}, glm::vec3(1., 1., 1.), glm::vec3{-1 + 0.2 * fontHeight, -1 + 0.2 * fontHeight, 0});
	addWord(pNodeParent, fontHeight2, std::string{"Level"}, glm::vec3(1., 0.1, 0.1), glm::vec3{-1 + 0.2 * fontHeight, 1 - 4 * fontHeight2, 0});
	addWord(pNodeParent, fontHeight2, std::string{"Endurance"}, glm::vec3(1., 1., 1.), glm::vec3{-1 + 0.2 * fontHeight, 1 - 2 * fontHeight2, 0});
}

void CWorld::initDrawElements3d()
{
	asset::CAssetWriter writer;
	writer.save();
	auto aAnimatedModel = m_loader.load2();

	addModelsAnimated(aAnimatedModel.m_aAnimatedModel);
	addModelsLevel(aAnimatedModel.m_aLevel);
	//addSkybox(inTextureFloor, size);
	addInitPlayer(aAnimatedModel.m_aPlayer);
	addCubes();
	m_scene.addSphere(m_scene.m_scene.root(), {1.0f, 10.0f, 1.0f}, 0, 1.5);
}

uint64_t CWorld::addModel(uint64_t pNodeParent, asset::SLoadModel modelPath1, bool bLevel, bool bAnimated)
{
	auto pNodeModel = m_scene.addNode(pNodeParent, modelPath1.m_model);
	m_scene.componentManager().addComponent<en_model::SMatrixSource>(pNodeModel) = {modelPath1.m_position, modelPath1.m_scale, modelPath1.m_rotate};
	m_scene.componentManager().addComponent<std::unique_ptr<en_model::CModelScene>>(pNodeModel) = std::make_unique<en_model::CModelScene>();
	auto* pSceneModel = m_scene.componentManager().addComponent<std::unique_ptr<en_model::CModelScene>>(pNodeModel).get();

	std::map<int, SMeshId> multipleMesh;
	en_model::CModel newModel(modelPath1, bAnimated);

	int meshCounter{0};
	for(auto& mesh : newModel.m_aMeshes)
	{
		if(bAnimated)
		{
			multipleMesh[meshCounter] = m_scene.sceneResources().addAnimatedMesh(std::move(*static_cast<en_model::CMeshAnimated*>(mesh.release())));
		}
		else
		{
			multipleMesh[meshCounter] = m_scene.sceneResources().addStaticMesh(std::move(*static_cast<en_model::CMeshStatic*>(mesh.release())));
		}
		meshCounter++;
	}
	std::map<int, SMaterialId> pMaterial;
	std::map<int, STextureId> pTexture;
	
	
	pTexture = loadTexture(newModel.m_aTexture, newModel.m_paths.m_texture);
	pMaterial = loadMaterial(newModel.m_aMaterial);

	m_scene.traverseNodeMeshes(pNodeModel, *newModel.m_pRoot.get(), multipleMesh, pMaterial, pTexture, bAnimated ? false : bLevel, bAnimated);
	m_scene.componentManager().addComponent<glm::mat4>(pNodeModel) = createMatrix1(modelPath1.m_position, modelPath1.m_scale, glm::vec3(modelPath1.m_rotate), modelPath1.m_rotate.w);

	if(bAnimated)
		m_scene.componentManager().addComponent<std::unique_ptr<en_model::CAnimator>>(pNodeModel) = std::make_unique<en_model::CAnimator>(newModel.m_animator);

	pSceneModel->m_pRoot.reset(newModel.m_pRoot.release());
	return pNodeModel;
}

void CWorld::addSkybox(vk_structs::CUniformBase* textureDefault)
{
	/*
	m_aDrawSimpleObject[EDrawTypeObject::eSkyBox].resize(2);
	auto& skyBox0 = m_aDrawSimpleObject[EDrawTypeObject::eSkyBox][0];
	auto& skyBox1 = m_aDrawSimpleObject[EDrawTypeObject::eSkyBox][1];
	skyBox0 = std::make_unique<en_vk_draw::CSimpleDrawObject>();
	skyBox0->create(geom_primitives::plane(), m_context.logicalDevice().handle(), m_context.m_details);
	skyBox0->m_model = glm::translate(skyBox0->m_model, glm::vec3(50, 0, 0));
	skyBox0->m_model = glm::scale(skyBox0->m_model, {50, 50, 50});

	skyBox0->m_model = glm::rotate(skyBox0->m_model, std::numbers::pi_v<float> / 2, glm::vec3(0, 0, 1));
	skyBox0->m_model = glm::rotate(skyBox0->m_model, std::numbers::pi_v<float>, glm::vec3(0, 1, 0));

	skyBox1 = std::make_unique<en_vk_draw::CSimpleDrawObject>();
	skyBox1->create(geom_primitives::plane(), m_context.logicalDevice().handle(), m_context.m_details);
	skyBox1->m_model = glm::translate(skyBox1->m_model, glm::vec3(0, 0, -50));
	skyBox1->m_model = glm::scale(skyBox1->m_model, {50, 50, 50});

	skyBox1->m_model = glm::rotate(skyBox1->m_model, std::numbers::pi_v<float> / 2, glm::vec3(1, 0, 0));
	skyBox1->m_model = glm::rotate(skyBox1->m_model, -std::numbers::pi_v<float> / 2, glm::vec3(0, 1, 0));


	int texWidth, texHeight, texChannels;

	auto pix2 = en_vk_utils::loadTexture(texWidth, texHeight, texChannels, texture_PATH_2.c_str());
	auto* inTexture2 = m_descriptorManager.createUniformTexture(m_context.logicalDevice().handle(), m_context.m_details, m_context.physicalDevice().handle(), pix2, texWidth, texHeight, texChannels, m_context.m_commandPool, m_context.m_aQueue[en_vk::EQueue::eGraphic], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	auto pix3 = en_vk_utils::loadTexture(texWidth, texHeight, texChannels, texture_PATH_3.c_str());
	auto* inTexture3 = m_descriptorManager.createUniformTexture(m_context.logicalDevice().handle(), m_context.m_details, m_context.physicalDevice().handle(), pix3, texWidth, texHeight, texChannels, m_context.m_commandPool, m_context.m_aQueue[en_vk::EQueue::eGraphic], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	std::vector<vk_structs::CUniformBase*> aTexture{inTexture2, inTexture3};

	int counter{0};
	for(auto& dr : m_aDrawSimpleObject[EDrawTypeObject::eSkyBox])
	{
		vk_structs::CUniformBase* texture{aTexture[counter]};
		for(int frameInd = 0; frameInd < size; frameInd++)
		{
			auto* indBuffer = m_descriptorManager.createUniformBuffer(m_context.logicalDevice().handle(), m_context.m_details, sizeof(en_vk_draw::SUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, nullptr, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr);
			auto& descriptor = m_descriptorManager.addLayoutResourceDescriptor(&dr, frameInd, en_vk::EPipelineLayout::e3d, 0);
			descriptor.m_resourceIndex.push_back(indBuffer);

			auto& descriptor2 = m_descriptorManager.addLayoutResourceDescriptor(&dr, frameInd, en_vk::EPipelineLayout::e3d, 1);
			descriptor2.m_resourceIndex.push_back(texture);
		}
		counter++;
	}
	*/
}

void CWorld::addWord(uint64_t pNodeParent, float fontHeight, std::string word, glm::vec3 color, glm::vec3 offset)
{
	auto resMeshIndexed = genIndexed1(m_textSystem.genWordMesh(word, color, offset, fontHeight));
	if(!resMeshIndexed.m_aVx.empty())
		m_scene.addWord(pNodeParent, resMeshIndexed, word);
}

void CWorld::addInitPlayer(const asset::SLoadModel& playerModel)
{
	m_scene.m_player = m_scene.addNode(m_scene.m_scene.root(), "Player");// m_scene.addNode(m_sceneComponentSystem.m_scene.root()->id());
	m_scene.componentManager().addComponent<glm::mat4>(m_scene.m_player, glm::mat4(1));
	addModel(m_scene.m_player, playerModel, false, true);
}

std::map<int, STextureId> CWorld::loadTexture(std::vector<commons_struct::STexture>& aTexture, const std::vector <std::string>& aTextureIn)
{
	int textureCounter{0};
	std::map<int, STextureId> result;
	if(!aTexture.empty())
	{
		for(auto& texture : aTexture)
		{
			result[textureCounter] = m_scene.sceneResources().addTexture(std::move(texture));
			textureCounter++;
		}
	}
	else
	{
		if(aTextureIn[0] != "default")
		{
			for(const auto texture : aTextureIn)
			{
				commons_struct::STexture textureS;

				textureS.m_data = en_vk_utils::loadTexture(textureS.w, textureS.h, textureS.texChannels, texture.c_str());
				result[textureCounter] = m_scene.sceneResources().addTexture(std::move(textureS));
				textureCounter++;
			}
		}
	}

	return result;
}
std::map<int, SMaterialId> CWorld::loadMaterial(std::vector<en_model::SMaterial2>& aMaterial)
{
	std::map<int, SMaterialId> result;

	int materialCounter{0};
	for(auto& material : aMaterial)
	{
		result[materialCounter] = m_scene.sceneResources().addMaterial(std::move(material));
		materialCounter++;
	}
	return result;
}