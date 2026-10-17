/*
 * Based on Poly2Tri (https://github.com/jhasse/poly2tri)
 * Copyright (c) 2009-2018, Poly2Tri Contributors. All rights reserved.
 * Licensed under the BSD 3-Clause License, see third_party_licenses/poly2tri-LICENSE.txt
 *
 * Modified by nettory0, 2026: renamed types, adapted for glyph triangulation.
 */
#include "tg_shapes.h"
#include <iostream>

namespace p2t
{

CTriangle::CTriangle(CPoint& a, CPoint& b, CPoint& c)
{
	points_[0] = &a; points_[1] = &b; points_[2] = &c;
	neighbors_[0] = NULL; neighbors_[1] = NULL; neighbors_[2] = NULL;
	constrained_edge[0] = constrained_edge[1] = constrained_edge[2] = false;
	delaunay_edge[0] = delaunay_edge[1] = delaunay_edge[2] = false;
	interior_ = false;
}

void CTriangle::MarkNeighbor(CPoint* p1, CPoint* p2, CTriangle* t)
{
	if((p1 == points_[2] && p2 == points_[1]) || (p1 == points_[1] && p2 == points_[2]))
		neighbors_[0] = t;
	else if((p1 == points_[0] && p2 == points_[2]) || (p1 == points_[2] && p2 == points_[0]))
		neighbors_[1] = t;
	else if((p1 == points_[0] && p2 == points_[1]) || (p1 == points_[1] && p2 == points_[0]))
		neighbors_[2] = t;
	else
		assert(0);
}

void CTriangle::MarkNeighbor(CTriangle& t)
{
	if(t.Contains(points_[1], points_[2]))
	{
		neighbors_[0] = &t;
		t.MarkNeighbor(points_[1], points_[2], this);
	}
	else if(t.Contains(points_[0], points_[2]))
	{
		neighbors_[1] = &t;
		t.MarkNeighbor(points_[0], points_[2], this);
	}
	else if(t.Contains(points_[0], points_[1]))
	{
		neighbors_[2] = &t;
		t.MarkNeighbor(points_[0], points_[1], this);
	}
}

void CTriangle::Clear()
{
	CTriangle* t;
	for(int i = 0; i < 3; i++)
	{
		t = neighbors_[i];
		if(t != NULL)
		{
			t->ClearNeighbor(this);
		}
	}
	ClearNeighbors();
	points_[0] = points_[1] = points_[2] = NULL;
}

void CTriangle::ClearNeighbor(const CTriangle* triangle)
{
	if(neighbors_[0] == triangle)
	{
		neighbors_[0] = NULL;
	}
	else if(neighbors_[1] == triangle)
	{
		neighbors_[1] = NULL;
	}
	else
	{
		neighbors_[2] = NULL;
	}
}

void CTriangle::ClearNeighbors()
{
	neighbors_[0] = NULL;
	neighbors_[1] = NULL;
	neighbors_[2] = NULL;
}

void CTriangle::ClearDelunayEdges()
{
	delaunay_edge[0] = delaunay_edge[1] = delaunay_edge[2] = false;
}

CPoint* CTriangle::OppositePoint(CTriangle& t, const CPoint& p)
{
	CPoint* cw = t.PointCW(p);
	return PointCW(*cw);
}

void CTriangle::Legalize(CPoint& point)
{
	points_[1] = points_[0];
	points_[0] = points_[2];
	points_[2] = &point;
}

void CTriangle::Legalize(CPoint& opoint, CPoint& npoint)
{
	if(&opoint == points_[0])
	{
		points_[1] = points_[0];
		points_[0] = points_[2];
		points_[2] = &npoint;
	}
	else if(&opoint == points_[1])
	{
		points_[2] = points_[1];
		points_[1] = points_[0];
		points_[0] = &npoint;
	}
	else if(&opoint == points_[2])
	{
		points_[0] = points_[2];
		points_[2] = points_[1];
		points_[1] = &npoint;
	}
	else
	{
		assert(0);
	}
}

int CTriangle::Index(const CPoint* p)
{
	if(p == points_[0])
	{
		return 0;
	}
	else if(p == points_[1])
	{
		return 1;
	}
	else if(p == points_[2])
	{
		return 2;
	}
	assert(0);
	return -1;
}

int CTriangle::EdgeIndex(const CPoint* p1, const CPoint* p2)
{
	if(points_[0] == p1)
	{
		if(points_[1] == p2)
		{
			return 2;
		}
		else if(points_[2] == p2)
		{
			return 1;
		}
	}
	else if(points_[1] == p1)
	{
		if(points_[2] == p2)
		{
			return 0;
		}
		else if(points_[0] == p2)
		{
			return 2;
		}
	}
	else if(points_[2] == p1)
	{
		if(points_[0] == p2)
		{
			return 1;
		}
		else if(points_[1] == p2)
		{
			return 0;
		}
	}
	return -1;
}

void CTriangle::MarkConstrainedEdge(int index)
{
	constrained_edge[index] = true;
}

void CTriangle::MarkConstrainedEdge(CEdge& edge)
{
	MarkConstrainedEdge(edge.p, edge.q);
}

void CTriangle::MarkConstrainedEdge(CPoint* p, CPoint* q)
{
	if((q == points_[0] && p == points_[1]) || (q == points_[1] && p == points_[0]))
	{
		constrained_edge[2] = true;
	}
	else if((q == points_[0] && p == points_[2]) || (q == points_[2] && p == points_[0]))
	{
		constrained_edge[1] = true;
	}
	else if((q == points_[1] && p == points_[2]) || (q == points_[2] && p == points_[1]))
	{
		constrained_edge[0] = true;
	}
}

CPoint* CTriangle::PointCW(const CPoint& point)
{
	if(&point == points_[0])
	{
		return points_[2];
	}
	else if(&point == points_[1])
	{
		return points_[0];
	}
	else if(&point == points_[2])
	{
		return points_[1];
	}
	assert(0);
	return NULL;
}

CPoint* CTriangle::PointCCW(const CPoint& point)
{
	if(&point == points_[0])
	{
		return points_[1];
	}
	else if(&point == points_[1])
	{
		return points_[2];
	}
	else if(&point == points_[2])
	{
		return points_[0];
	}
	assert(0);
	return NULL;
}

CTriangle* CTriangle::NeighborCW(const CPoint& point)
{
	if(&point == points_[0])
	{
		return neighbors_[1];
	}
	else if(&point == points_[1])
	{
		return neighbors_[2];
	}
	return neighbors_[0];
}

CTriangle* CTriangle::NeighborCCW(const CPoint& point)
{
	if(&point == points_[0])
	{
		return neighbors_[2];
	}
	else if(&point == points_[1])
	{
		return neighbors_[0];
	}
	return neighbors_[1];
}

bool CTriangle::GetConstrainedEdgeCCW(const CPoint& p)
{
	if(&p == points_[0])
	{
		return constrained_edge[2];
	}
	else if(&p == points_[1])
	{
		return constrained_edge[0];
	}
	return constrained_edge[1];
}

bool CTriangle::GetConstrainedEdgeCW(const CPoint& p)
{
	if(&p == points_[0])
	{
		return constrained_edge[1];
	}
	else if(&p == points_[1])
	{
		return constrained_edge[2];
	}
	return constrained_edge[0];
}

void CTriangle::SetConstrainedEdgeCCW(const CPoint& p, bool ce)
{
	if(&p == points_[0])
	{
		constrained_edge[2] = ce;
	}
	else if(&p == points_[1])
	{
		constrained_edge[0] = ce;
	}
	else
	{
		constrained_edge[1] = ce;
	}
}

void CTriangle::SetConstrainedEdgeCW(const CPoint& p, bool ce)
{
	if(&p == points_[0])
	{
		constrained_edge[1] = ce;
	}
	else if(&p == points_[1])
	{
		constrained_edge[2] = ce;
	}
	else
	{
		constrained_edge[0] = ce;
	}
}

bool CTriangle::GetDelunayEdgeCCW(const CPoint& p)
{
	if(&p == points_[0])
	{
		return delaunay_edge[2];
	}
	else if(&p == points_[1])
	{
		return delaunay_edge[0];
	}
	return delaunay_edge[1];
}

bool CTriangle::GetDelunayEdgeCW(const CPoint& p)
{
	if(&p == points_[0])
	{
		return delaunay_edge[1];
	}
	else if(&p == points_[1])
	{
		return delaunay_edge[2];
	}
	return delaunay_edge[0];
}

void CTriangle::SetDelunayEdgeCCW(const CPoint& p, bool e)
{
	if(&p == points_[0])
	{
		delaunay_edge[2] = e;
	}
	else if(&p == points_[1])
	{
		delaunay_edge[0] = e;
	}
	else
	{
		delaunay_edge[1] = e;
	}
}

void CTriangle::SetDelunayEdgeCW(const CPoint& p, bool e)
{
	if(&p == points_[0])
	{
		delaunay_edge[1] = e;
	}
	else if(&p == points_[1])
	{
		delaunay_edge[2] = e;
	}
	else
	{
		delaunay_edge[0] = e;
	}
}

CTriangle& CTriangle::NeighborAcross(const CPoint& opoint)
{
	if(&opoint == points_[0])
	{
		return *neighbors_[0];
	}
	else if(&opoint == points_[1])
	{
		return *neighbors_[1];
	}
	return *neighbors_[2];
}

void CTriangle::DebugPrint()
{
	using namespace std;
	cout << points_[0]->x() << "," << points_[0]->y() << " ";
	cout << points_[1]->x() << "," << points_[1]->y() << " ";
	cout << points_[2]->x() << "," << points_[2]->y() << endl;
}

}