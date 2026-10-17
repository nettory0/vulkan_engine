/*
 * Based on FTGL FTVectoriser (https://github.com/frankheckenbach/ftgl)
 * Copyright (C) 2001-2004 Henry Maddocks and FTGL contributors
 * Licensed under the MIT License, see third_party_licenses/FTGL-COPYING.txt
 *
 * Modified by nettory0, 2026: renamed types, adapted for glyph mesh generation.
 */
#pragma once
#include <list>
#include <vector>
#include <array>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace ftgl
{

const double FTGL_FRONT_FACING = 1.0;
const double FTGL_BACK_FACING = -1.0;

class CPoint
{
public:
	CPoint()
	{
		aValues[0] = 0;
		aValues[1] = 0;
		aValues[2] = 0;
	}
	CPoint(const double x, const double y, const double z = 0)
	{
		aValues[0] = x;
		aValues[1] = y;
		aValues[2] = z;
	}
	inline CPoint(const FT_Vector& ft_vector)
	{
		aValues[0] = ft_vector.x;
		aValues[1] = ft_vector.y;
		aValues[2] = 0;
	}
	CPoint Normalise();
	CPoint& operator += (const CPoint& point)
	{
		aValues[0] += point.aValues[0];
		aValues[1] += point.aValues[1];
		aValues[2] += point.aValues[2];

		return *this;
	}
	CPoint operator + (const CPoint& point) const
	{
		CPoint temp;
		temp.aValues[0] = aValues[0] + point.aValues[0];
		temp.aValues[1] = aValues[1] + point.aValues[1];
		temp.aValues[2] = aValues[2] + point.aValues[2];

		return temp;
	}
	CPoint& operator -= (const CPoint& point)
	{
		aValues[0] -= point.aValues[0];
		aValues[1] -= point.aValues[1];
		aValues[2] -= point.aValues[2];

		return *this;
	}
	CPoint operator - (const CPoint& point) const
	{
		CPoint temp;
		temp.aValues[0] = aValues[0] - point.aValues[0];
		temp.aValues[1] = aValues[1] - point.aValues[1];
		temp.aValues[2] = aValues[2] - point.aValues[2];

		return temp;
	}
	CPoint operator * (double multiplier) const
	{
		CPoint temp;
		temp.aValues[0] = aValues[0] * multiplier;
		temp.aValues[1] = aValues[1] * multiplier;
		temp.aValues[2] = aValues[2] * multiplier;

		return temp;
	}
	friend CPoint operator * (double multiplier, CPoint& point)
	{
		return point * multiplier;
	}
	friend double operator * (CPoint& a, CPoint& b)
	{
		return a.aValues[0] * b.aValues[0]
			+ a.aValues[1] * b.aValues[1]
			+ a.aValues[2] * b.aValues[2];
	}
	CPoint operator ^ (const CPoint& point)
	{
		CPoint temp;
		temp.aValues[0] = aValues[1] * point.aValues[2] - aValues[2] * point.aValues[1];
		temp.aValues[1] = aValues[2] * point.aValues[0] - aValues[0] * point.aValues[2];
		temp.aValues[2] = aValues[0] * point.aValues[1] - aValues[1] * point.aValues[0];
		return temp;
	}
	friend bool operator == (const CPoint& a, const CPoint& b);
	friend bool operator != (const CPoint& a, const CPoint& b);
	/*
	operator const double* () const
	{
		return aValues.begin();
	}
	*/
	void X(double x)
	{
		aValues[0] = x;
	};
	void Y(double y)
	{
		aValues[1] = y;
	};
	void Z(double z)
	{
		aValues[2] = z;
	};
	double X() const
	{
		return aValues[0];
	};
	double Y() const
	{
		return aValues[1];
	};
	double Z() const
	{
		return aValues[2];
	};
	float Xf() const
	{
		return static_cast<float>(aValues[0]);
	};
	float Yf() const
	{
		return static_cast<float>(aValues[1]);
	};
	float Zf() const
	{
		return static_cast<float>(aValues[2]);
	};

private:
	std::array<double, 3> aValues;
};

class CLoop
{
public:
	CLoop(FT_Vector* contour, unsigned char* pointTags, unsigned int numberOfPoints, unsigned short bezierSteps);
	CLoop() = default;
	~CLoop();

	const CPoint& GetPoint(size_t index) const
	{
		return pointList[index];
	}
	size_t PointCount() const
	{
		return pointList.size();
	}
	void SetParity(int parity);
	bool GetDirection() const
	{
		return clockwise;
	}

	// True if this loop lies inside `big` - used to pair a hole contour with
	// the outer contour it cuts into. A bounding-box check alone is only a
	// heuristic and gives wrong answers for non-convex outlines, so it is
	// used here purely as a cheap reject before the real point-in-polygon test.
	bool IsInside(const CLoop* big) const
	{
		if(pointList.empty() || big->pointList.empty())
			return false;

		if(minx < big->minx || miny < big->miny || maxx > big->maxx || maxy > big->maxy)
			return false;

		return big->ContainsPoint(pointList[0]);
	}

	void Reverse();
public:
	float minx{65000.0f}, miny{65000.0f}, maxx{-65000.0f}, maxy{-65000.0f};

private:
	void AddPoint(CPoint point);
	void AddOutsetPoint(CPoint point);
	void evaluateQuadraticCurve(CPoint, CPoint, CPoint, unsigned short);
	void evaluateCubicCurve(CPoint, CPoint, CPoint, CPoint, unsigned short);
	CPoint ComputeOutsetPoint(CPoint a, CPoint b, CPoint c);
	// Even-odd (ray casting) point-in-polygon test against this loop's boundary.
	bool ContainsPoint(const CPoint& p) const;

private:
	std::vector<CPoint> pointList;
	std::vector<CPoint> outsetPointList;
	std::vector<CPoint> frontPointList;
	std::vector<CPoint> backPointList;
	bool clockwise;
};



class CVectoriser
{
public:
	CVectoriser(const FT_Outline outline, unsigned short bezierSteps, bool reverse);
	virtual ~CVectoriser();
	size_t PointCount();
	size_t ContourCount() const
	{
		return contourList.size();
	}
	const CLoop* const GetContour(size_t index) const;
	const std::vector<CLoop>& GetContour() const;
	size_t ContourSize(int c) const
	{
		return contourList[c].PointCount();
	}
	int ContourFlag() const
	{
		return contourFlag;
	}

private:
	void ProcessContours(unsigned short bezierSteps, bool reverse, int count);
	std::vector<CLoop> contourList;
	int contourFlag;
	FT_Outline outline;
};

}

