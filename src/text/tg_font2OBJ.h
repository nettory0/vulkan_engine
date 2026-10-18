#pragma once
#include <math.h>
#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_FREETYPE_H
#include <vector>
#include <iostream>
#include <fstream>
#include <codecvt>
#include <locale>

#include "tg_vectoriser.h"
#include "tg_shapes.h"
#include "tg_sweep_context.h"
#include "tg_mesh.h"


//using namespace p2t;
//using namespace ftgl;

std::vector<p2t::CPoint*> triangulateContour(const ftgl::CLoop* contour)
{
	std::vector<p2t::CPoint*> polyline;
	for(size_t p = 0; p < contour->PointCount(); ++p)
	{
		auto& point = contour->GetPoint(p);
		polyline.push_back(new p2t::CPoint(point.X(), point.Y()));
	}
	return polyline;
}

ftgl::CVectoriser getVectoriser(FT_GlyphSlot& glyph, char32_t ch, unsigned short bezierSteps)
{
	FT_Orientation f = FT_Outline_Get_Orientation(&glyph->outline);

	ftgl::CVectoriser v(glyph->outline, bezierSteps, f == true);

	return v;
}

bool addCharacter(character::CMeshText& mesh, FT_Face face, char32_t ch, unsigned short bezierSteps)
{
	FT_UInt curCharIndex = FT_Get_Char_Index(face, ch);
	if(FT_Load_Glyph(face, curCharIndex, FT_LOAD_DEFAULT))
		return false;

	if(face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
		return false;

	const float h = face->height;
	ftgl::CVectoriser vectoriser = getVectoriser(face->glyph, ch, bezierSteps);

	character::CVertexText v1, v2, v3;
	for(auto& loop1: vectoriser.GetContour())
	{
		if(loop1.GetDirection())
		{
			std::vector<p2t::CPoint*> polyline = triangulateContour(&loop1);
			p2t::CCDT cdt(polyline);
			std::vector<std::vector<p2t::CPoint*>> holePolylines;
			for(auto& loop2 : vectoriser.GetContour())
			{
				if(&loop2 != &loop1 && !loop2.GetDirection() && loop2.IsInside(&loop1))
				{
					std::vector<p2t::CPoint*> pl = triangulateContour(&loop2);
					cdt.AddHole(pl);
					holePolylines.push_back(pl);
				}
			}

			cdt.Triangulate();
			std::vector<p2t::CTriangle*> ts = cdt.GetTriangles();
			for(auto* ot: ts)
			{

				v1.m_pos.x = ot->GetPoint(0)->x() / h;
				v1.m_pos.y = ot->GetPoint(0)->y() / h;
				v1.m_pos.z = 0;
				v2.m_pos.x = ot->GetPoint(1)->x() / h;
				v2.m_pos.y = ot->GetPoint(1)->y() / h;
				v2.m_pos.z = 0;
				v3.m_pos.x = ot->GetPoint(2)->x() / h;
				v3.m_pos.y = ot->GetPoint(2)->y() / h;
				v3.m_pos.z = 0;
				mesh.addTriangle(v1, v2, v3);

				v1.m_pos.x = ot->GetPoint(0)->x() / h;
				v1.m_pos.y = ot->GetPoint(0)->y() / h;
				v1.m_pos.z = 0;
				v2.m_pos.x = ot->GetPoint(1)->x() / h;
				v2.m_pos.y = ot->GetPoint(1)->y() / h;
				v2.m_pos.z = 0;
				v3.m_pos.x = ot->GetPoint(2)->x() / h;
				v3.m_pos.y = ot->GetPoint(2)->y() / h;
				v3.m_pos.z = 0;
				mesh.addTriangle(v3, v2, v1);
			}

			// p2t::CCDT/CSweepContext take ownership of the triangles/edges it builds,
			// but not of the CPoint*s we allocated in triangulateContour — free those here.
			for(auto* p : polyline)
				delete p;
			for(auto& pl : holePolylines)
				for(auto* p : pl)
					delete p;
		}
	}
	const float ofX = face->glyph->advance.x;
	mesh.m_offset = ofX / h;
	return true;
}
