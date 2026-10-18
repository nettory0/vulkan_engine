/*
 * Based on FTGL FTVectoriser (https://github.com/frankheckenbach/ftgl)
 * Copyright (C) 2001-2004 Henry Maddocks and FTGL contributors
 * Licensed under the MIT License, see third_party_licenses/FTGL-COPYING.txt
 *
 * Modified by nettory0, 2026: renamed types, adapted for glyph mesh generation.
 */
#include <cmath>
#include "tg_vectoriser.h"


namespace ftgl
{

CVectoriser::CVectoriser(const FT_Outline ol, unsigned short bezierSteps, bool reverse): contourList(0), contourFlag(0)
{
	outline = ol;
	contourFlag = outline.flags;

	ProcessContours(bezierSteps, reverse, outline.n_contours);
}

CVectoriser::~CVectoriser()
{
}

void CVectoriser::ProcessContours(unsigned short bezierSteps, bool reverse, int count)
{
	short contourLength = 0;
	short startIndex = 0;
	short endIndex = 0;

	contourList.resize(count);

	for(int i = 0; i < count; ++i)
	{
		FT_Vector* pointList = &outline.points[startIndex];
		unsigned char* tagList = &outline.tags[startIndex];

		endIndex = outline.contours[i];
		contourLength = (endIndex - startIndex) + 1;

		contourList[i] = CLoop(pointList, tagList, contourLength, bezierSteps);
		if(reverse)
			contourList[i].Reverse();

		startIndex = endIndex + 1;
	}

	return; 
}

size_t CVectoriser::PointCount()
{
	size_t s = 0;
	for(size_t c = 0; c < ContourCount(); ++c)
	{
		s += contourList[c].PointCount();
	}

	return s;
}

const CLoop* const CVectoriser::GetContour(size_t index) const
{
	return (index < ContourCount()) ? &(contourList[index]) : nullptr;
}

const std::vector<CLoop>& CVectoriser::GetContour() const
{
	return contourList;
}

bool operator == (const CPoint& a, const CPoint& b)
{
	return((a.aValues[0] == b.aValues[0]) && (a.aValues[1] == b.aValues[1]) && (a.aValues[2] == b.aValues[2]));
}


bool operator != (const CPoint& a, const CPoint& b)
{
	return((a.aValues[0] != b.aValues[0]) || (a.aValues[1] != b.aValues[1]) || (a.aValues[2] != b.aValues[2]));
}


CPoint CPoint::Normalise()
{
	double norm = sqrt(aValues[0] * aValues[0]
		+ aValues[1] * aValues[1]
		+ aValues[2] * aValues[2]);
	if(norm == 0.0)
	{
		return *this;
	}

	CPoint temp(aValues[0] / norm, aValues[1] / norm, aValues[2] / norm);
	return temp;
}

void CLoop::AddPoint(CPoint point)
{
	if(pointList.empty() || (point != pointList[pointList.size() - 1]
		&& point != pointList[0]))
	{
		pointList.push_back(point);
	}

	if(minx > point.X())
		minx = point.X();

	if(miny > point.Y())
		miny = point.Y();

	if(maxx < point.X())
		maxx = point.X();

	if(maxy < point.Y())
		maxy = point.Y();
}

void CLoop::AddOutsetPoint(CPoint point)
{
	outsetPointList.push_back(point);
}

void CLoop::evaluateQuadraticCurve(CPoint A, CPoint B, CPoint C, unsigned short bezierSteps)
{
	for(unsigned int i = 1; i < bezierSteps; i++)
	{
		float t = static_cast<float>(i) / bezierSteps;

		CPoint U = (1.0f - t) * A + t * B;
		CPoint V = (1.0f - t) * B + t * C;

		AddPoint((1.0f - t) * U + t * V);
	}
}


void CLoop::evaluateCubicCurve(CPoint A, CPoint B, CPoint C, CPoint D, unsigned short bezierSteps)
{
	for(unsigned int i = 0; i < bezierSteps; i++)
	{
		float t = static_cast<float>(i) / bezierSteps;

		CPoint U = (1.0f - t) * A + t * B;
		CPoint V = (1.0f - t) * B + t * C;
		CPoint W = (1.0f - t) * C + t * D;

		CPoint M = (1.0f - t) * U + t * V;
		CPoint N = (1.0f - t) * V + t * W;

		AddPoint((1.0f - t) * M + t * N);
	}
}


// This function is a bit tricky. Given a path ABC, it returns the
// coordinates of the outset point facing B on the left at a distance
// of 64.0.
//                                         M
//                            - - - - - - X
//                             ^         / '
//                             | 64.0   /   '
//  X---->-----X     ==>    X--v-------X     '
// A          B \          A          B \   .>'
//               \                       \<'  64.0
//                \                       \                  .
//                 \                       \                 .
//                C X                     C X
//
CPoint CLoop::ComputeOutsetPoint(CPoint A, CPoint B, CPoint C)
{
	CPoint ba = (A - B).Normalise();
	CPoint bc = C - B;
	CPoint tmp(bc.X() * -ba.X() + bc.Y() * -ba.Y(), bc.X() * ba.Y() + bc.Y() * -ba.X());

	double norm = sqrt(tmp.X() * tmp.X() + tmp.Y() * tmp.Y());
	double dist = sqrt((norm - tmp.X()) / (norm + tmp.X()));
	tmp.X(tmp.Y() < 0.0 ? dist : -dist);
	tmp.Y(1);

	return CPoint(tmp.X() * -ba.X() + tmp.Y() * ba.Y(), tmp.X() * -ba.Y() + tmp.Y() * -ba.X());
}

bool CLoop::ContainsPoint(const CPoint& p) const
{
	bool inside = false;
	size_t n = pointList.size();
	for(size_t i = 0, j = n - 1; i < n; j = i++)
	{
		const CPoint& pi = pointList[i];
		const CPoint& pj = pointList[j];

		if(((pi.Y() > p.Y()) != (pj.Y() > p.Y())) &&
			(p.X() < (pj.X() - pi.X()) * (p.Y() - pi.Y()) / (pj.Y() - pi.Y()) + pi.X()))
		{
			inside = !inside;
		}
	}
	return inside;
}

void CLoop::Reverse()
{
	size_t size = PointCount();

	for(size_t i = 0; i < size / 2; i++)
	{
		CPoint tmp = pointList[i];
		pointList[i] = pointList[size - 1 - i];
		pointList[size - 1 - i] = tmp;
	}
	clockwise = !clockwise;

	CPoint vOutset;
	for(size_t i = 0; i < size; i++)
	{
		size_t prev, cur, next;

		prev = (i + size - 1) % size;
		cur = i;
		next = (i + size + 1) % size;

		vOutset = ComputeOutsetPoint(GetPoint(prev), GetPoint(cur), GetPoint(next));
		AddOutsetPoint(vOutset);
	}
}

void CLoop::SetParity(int parity)
{
	size_t size = PointCount();
	CPoint vOutset;

	if(((parity & 1) && clockwise) || (!(parity & 1) && !clockwise))
	{
		for(size_t i = 0; i < size / 2; i++)
		{
			CPoint tmp = pointList[i];
			pointList[i] = pointList[size - 1 - i];
			pointList[size - 1 - i] = tmp;
		}

		clockwise = !clockwise;
	}

	for(size_t i = 0; i < size; i++)
	{
		size_t prev, cur, next;

		prev = (i + size - 1) % size;
		cur = i;
		next = (i + size + 1) % size;

		vOutset = ComputeOutsetPoint(GetPoint(prev), GetPoint(cur), GetPoint(next));
		AddOutsetPoint(vOutset);
	}
}


CLoop::CLoop(FT_Vector* contour, unsigned char* tags, unsigned int n, unsigned short bezierSteps)
{
	CPoint prev, cur(contour[(n - 1) % n]), next(contour[0]);

	for(unsigned int i = 0; i < n; i++)
	{
		prev = cur;
		cur = next;
		next = CPoint(contour[(i + 1) % n]);

		if(n < 2 || FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_On)
		{
			AddPoint(cur);
		}
		else if(FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Conic)
		{
			CPoint prev2 = prev, next2 = next;

			if(FT_CURVE_TAG(tags[(i - 1 + n) % n]) == FT_Curve_Tag_Conic)
			{
				prev2 = (cur + prev) * 0.5;
				AddPoint(prev2);
			}

			if(FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Conic)
			{
				next2 = (cur + next) * 0.5;
			}

			evaluateQuadraticCurve(prev2, cur, next2, bezierSteps);
		}
		else if(FT_CURVE_TAG(tags[i]) == FT_Curve_Tag_Cubic
			&& FT_CURVE_TAG(tags[(i + 1) % n]) == FT_Curve_Tag_Cubic)
		{
			evaluateCubicCurve(prev, cur, next, CPoint(contour[(i + 2) % n]), bezierSteps);
		}
	}

	double signedArea = 0.0;

	prev = CPoint(contour[n - 1]);
	for(unsigned int i = 0; i < n; i++)
	{
		cur = CPoint(contour[i]);

		double area = cur.X() * prev.Y() - cur.Y() * prev.X();
		signedArea += area;

		prev = cur;
	}
	clockwise = signedArea > 0.0f;
}

CLoop::~CLoop()
{
	pointList.clear();
	outsetPointList.clear();
	frontPointList.clear();
	backPointList.clear();
}


}
