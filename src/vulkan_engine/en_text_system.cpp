#include "en_text_system.h"

std::vector<commons_struct::SVertex> CTextSystem::genWordMesh(std::string word, glm::vec3 color, glm::vec3 offset, float fontHeight)
{
	const float aspectRatio = 1920. / 1080.;
	std::vector<commons_struct::SVertex> verticesMesh;
	for(auto s : word)
	{
		if(auto* pMesh = m_textAssistant.getChar(s))
		{
			for(auto ind : pMesh->indices)
			{
				verticesMesh.push_back({});
				auto& back = verticesMesh.back();
				back.pos.x = pMesh->vertices[ind - 1].m_pos.x * fontHeight;
				back.pos.y = pMesh->vertices[ind - 1].m_pos.y * fontHeight * aspectRatio;
				back.pos.z = 0;
				back.pos += glm::vec4(offset, 0);
				back.color = glm::vec4(color, 0);
			}
			offset.x += pMesh->m_offset * fontHeight;
		}
	}
	return verticesMesh;
}