#pragma once
#include "en_text_system.h"
#include "asset_loader.h"
#include "en_animation_structs.h"
#include "en_scene_component.h"

namespace en_scene
{
class CNodeBase;
}

namespace asset
{
class CAssetLoader;
struct SLoadModel;
}

class CSceneComponentSystem;

class CWorld
{
public:
	explicit CWorld(CSceneComponentSystem& scene);

	void loadLevel();
	void spawnPlayer(const asset::SLoadModel& playerModel);
public:
	CTextSystem m_textSystem;
	asset::CAssetLoader m_loader;

private:
	void initDrawElements3d();
	void initDrawElements2d();
	void addWord(uint64_t pNodeParent, float fontHeight, std::string word, glm::vec3 color, glm::vec3 offset);
	void addInitPlayer(const asset::SLoadModel& playerModel);
	void addPrimitives(const std::vector<asset::SLoadPrimitive>& aPrimitives);
	void addSkybox(vk_structs::CUniformBase* textureDefault);
	void addModelsLevel(const std::vector<asset::SLoadModel>& aModels);
	void addModelsAnimated(const std::vector<asset::SLoadModel>& aModels);
	uint64_t addModel(uint64_t pNodeParent, asset::SLoadModel modelPath1, bool bLevel, bool bAnimated);
	std::map<int, STextureId> loadTexture(std::vector<commons_struct::STexture>& aTexture, const std::vector<std::string>& aTextureIn);
	std::map<int, SMaterialId> loadMaterial(std::vector<en_model::SMaterial2>& aMaterial);
private:
	CSceneComponentSystem& m_scene;


};
