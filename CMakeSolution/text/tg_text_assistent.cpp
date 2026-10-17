#include "tg_text_assistent.h"
#include <string>
#include "tg_font2OBJ.h"
#include FT_FREETYPE_H
#include FT_OUTLINE_H
namespace text
{

CTextAssistent::~CTextAssistent()
{}

bool CTextAssistent::init2(const char* fontFilePath)
{
	auto error = FT_Init_FreeType(&m_library);
	if(error)
	{
		printf("FT_Init_FreeType failed\n");
		return false;
	}

	error = FT_New_Face(m_library, fontFilePath, 0, &m_pFace);
	if(error == FT_Err_Unknown_File_Format)
	{
		printf("Unsupported font format: %s\n", fontFilePath);
		return false;
	}
	else if(error)
	{
		printf("Failed to load font: %s\n", fontFilePath);
		return false;
	}

	printf("Loaded font %s, height %d\n", fontFilePath, m_pFace->height);
	return true;
}
const character::CMeshText* CTextAssistent::getChar(char32_t code)
{
	if(m_pFace)
	{
		if(auto it = m_aMesh.find(code); it != m_aMesh.end())
			return &it->second;

		if(auto t = FT_Get_Char_Index(m_pFace, code))
		{
			auto& mesh = m_aMesh[code];
			if(!addCharacter(mesh, m_pFace, code, m_bezierSteps))
			{
				m_aMesh.erase(code);
				return nullptr;
			}
			return &mesh;
		}
	}
	return nullptr;
}
}