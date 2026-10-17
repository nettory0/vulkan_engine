#pragma once
#include "asset_loader_export.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace YAML
{
class Node;
}

namespace asset
{

struct SLoadModel
{
	std::string m_model{"default"};
	std::vector <std::string> m_texture{"default"};
	glm::vec3 m_position{0., 0., 0.};
	glm::vec3 m_scale{1., 1., 1.};
	glm::vec4 m_rotate{1., 1., 1., 0};
};

struct SLoadPrimitive
{
	glm::vec3 m_position{0., 0., 0.};
	glm::vec3 m_scale{1., 1., 1.};
	glm::vec4 m_rotate{1., 1., 1., 0};
};

struct SReadResult
{
	std::vector<SLoadModel> m_aAnimatedModel;
	std::vector<SLoadModel> m_aLevel;
	SLoadModel m_aPlayer;
};

class ASSET_MODULE CAssetLoader
{
public:
	void load();
	SReadResult load2();
private:
	glm::vec3 readVec3(const YAML::Node& node);
	glm::vec4 readVec4(const YAML::Node& node);
	SLoadModel readNode(const YAML::Node& node);
};

class ASSET_MODULE CAssetWriter
{
public:
	void save();
};

}
