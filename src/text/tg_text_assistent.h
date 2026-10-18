#pragma once
#include <ft2build.h>
#include <vector>
#include <map>
#include "tg_mesh.h"
#include "tg_text_export.h"

typedef struct FT_LibraryRec_* FT_Library;
typedef struct FT_FaceRec_* FT_Face;

namespace character
{
struct CMeshText;
}

namespace text
{
class TEXT_GEN CTextAssistent
{
public:
	CTextAssistent() = default;
	~CTextAssistent();
	bool init2(const char* fontFilePath = "models/fonts/LiberationSans-Regular.ttf");
	const character::CMeshText* getChar(char32_t);
	FT_Library  m_library{nullptr};
	FT_Face m_pFace{nullptr};
	std::map<char32_t, character::CMeshText> m_aMesh;
private:
	const unsigned short m_bezierSteps{2};
};
}
