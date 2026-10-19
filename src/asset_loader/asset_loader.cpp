#include "asset_loader.h"
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <fstream>

namespace asset
{

void CAssetLoader::load()
{

	YAML::Node config = YAML::LoadFile("config_saved.yaml");


	std::string appName = config["app_name"].as<std::string>();
	int width = config["window"]["width"].as<int>();
	int height = config["window"]["height"].as<int>();
	bool fullscreen = config["fullscreen"].as<bool>();

	std::cout << "App Name: " << appName << "\n";
	std::cout << "Window: " << width << "x" << height << "\n";
	std::cout << "Fullscreen: " << (fullscreen ? "Yes" : "No") << "\n";
}

SReadResult CAssetLoader::load2()
{
	SReadResult result;

	YAML::Node root = YAML::LoadFile("config_saved.yaml")["root"];

	if(YAML::Node modelSet = root["modelSet"])
	{
		for(auto it = modelSet.begin(); it != modelSet.end(); ++it)
			result.m_aAnimatedModel.push_back(readNode(it->second));
	}

	if(YAML::Node modelLevel = root["modelLevel"])
	{
		for(auto it = modelLevel.begin(); it != modelLevel.end(); ++it)
			result.m_aLevel.push_back(readNode(it->second));
	}

	if(YAML::Node nodePlayer = root["modelPlayer"])
	{
		for(auto it = nodePlayer.begin(); it != nodePlayer.end(); ++it)
			result.m_aPlayer = readNode(it->second);
	}

	YAML::Node primitiveSet = root["primitiveSet"];

	for(auto it = primitiveSet.begin(); it != primitiveSet.end(); ++it)
	{
		std::string key = it->first.as<std::string>();
		std::string value = it->second.as<std::string>();
	}
	return result;
}

void CAssetWriter::save()
{
	return;
	YAML::Node root;

	// modelSet
	root["root"]["modelSet"]["nodeA"]["path"] = "value1";
	root["root"]["modelSet"]["nodeA"]["texture"] = "value2";

	root["root"]["modelSet"]["nodeB"]["path"] = "value3";
	root["root"]["modelSet"]["nodeB"]["texture"] = "value4";

	// primitiveSet
	root["root"]["primitiveSet"]["nodeA1"].push_back(1.23);
	root["root"]["primitiveSet"]["nodeA1"].push_back(2.23);
	root["root"]["primitiveSet"]["nodeA1"].push_back(3.23);
	root["root"]["primitiveSet"]["nodeA2"].push_back(1.23);
	root["root"]["primitiveSet"]["nodeA2"].push_back(1.23);
	root["root"]["primitiveSet"]["nodeA2"].push_back(1.23);
	root["root"]["primitiveSet"]["nodeA3"].push_back(1.23);
	root["root"]["primitiveSet"]["nodeA3"].push_back(1.23);
	root["root"]["primitiveSet"]["nodeA3"].push_back(1.23);


	std::ofstream fout("config_saved.yaml");
	fout << root;
	fout.close();

}

glm::vec3 CAssetLoader::readVec3(const YAML::Node& node)
{
	glm::vec3 result{0, 0, 0};
	if(node.size() == 3)
	{
		result[0] = node[0].as<float>();
		result[1] = node[1].as<float>();
		result[2] = node[2].as<float>();
	}
	return result;
}

glm::vec4 CAssetLoader::readVec4(const YAML::Node& node)
{
	glm::vec4 result{0, 0, 0, 0};
	if(node.size() == 4)
	{
		result[0] = node[0].as<float>();
		result[1] = node[1].as<float>();
		result[2] = node[2].as<float>();
		result[3] = node[3].as<float>();
	}
	return result;
}
SLoadModel CAssetLoader::readNode(const YAML::Node& node)
{
	SLoadModel model;

	model.m_model = node["path"].as<std::string>();
	model.m_texture = node["texture"].as<std::vector<std::string>>();

	if(auto nodePosition = node["position"]; nodePosition && nodePosition.IsSequence() && nodePosition.size() == 3)
		model.m_position = readVec3(nodePosition);

	if(auto nodeScale = node["scale"]; nodeScale && nodeScale.IsSequence() && nodeScale.size() == 3)
		model.m_scale = readVec3(nodeScale);

	if(auto nodeRotate = node["rotate"]; nodeRotate && nodeRotate.IsSequence())
		model.m_rotate = readVec4(nodeRotate);

	return model;
}

}
