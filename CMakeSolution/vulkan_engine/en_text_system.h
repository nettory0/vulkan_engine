#pragma once
#include <vector>
#include "tg_text_assistent.h"
#include "co_structs.h"
#include "en_vulkan_engine_export.h"

class VULKAN_ENGINE CTextSystem
{
public:
	CTextSystem() = default;
	~CTextSystem() = default;

	text::CTextAssistent m_textAssistant;
	std::vector<commons_struct::SVertex> genWordMesh(std::string word, glm::vec3 color, glm::vec3 offset, float fontHeight);
};