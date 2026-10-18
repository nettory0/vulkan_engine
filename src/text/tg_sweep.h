/*
 * Based on Poly2Tri (https://github.com/jhasse/poly2tri)
 * Copyright (c) 2009-2018, Poly2Tri Contributors. All rights reserved.
 * Licensed under the BSD 3-Clause License, see third_party_licenses/poly2tri-LICENSE.txt
 *
 * Modified by nettory0, 2026: renamed types, adapted for glyph triangulation.
 */
#pragma once
#include <vector>

namespace p2t
{

class CSweepContext;
struct SNode;
struct CPoint;
struct CEdge;
class CTriangle;

class CSweep
{
public:
	void Triangulate(CSweepContext& tcx);
	~CSweep();

private:
	void SweepPoints(CSweepContext& tcx);
	SNode& PointEvent(CSweepContext& tcx, CPoint& point);
	void EdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node);
	void EdgeEvent(CSweepContext& tcx, CPoint& ep, CPoint& eq, CTriangle* triangle, CPoint& point);
	SNode& NewFrontTriangle(CSweepContext& tcx, CPoint& point, SNode& node);
	void Fill(CSweepContext& tcx, SNode& node);
	bool Legalize(CSweepContext& tcx, CTriangle& t);
	bool Incircle(const CPoint& pa, const CPoint& pb, const CPoint& pc, const CPoint& pd) const;
	void RotateTrianglePair(CTriangle& t, CPoint& p, CTriangle& ot, CPoint& op) const;
	void FillAdvancingFront(CSweepContext& tcx, SNode& n);
	bool LargeHole_DontFill(const SNode* node) const;
	bool AngleExceeds90Degrees(const CPoint* origin, const CPoint* pa, const CPoint* pb) const;
	bool AngleExceedsPlus90DegreesOrIsNegative(const CPoint* origin, const CPoint* pa, const CPoint* pb) const;
	double Angle(const CPoint* origin, const CPoint* pa, const CPoint* pb) const;
	double HoleAngle(const SNode& node) const;
	double BasinAngle(const SNode& node) const;
	void FillBasin(CSweepContext& tcx, SNode& node);
	void FillBasinReq(CSweepContext& tcx, SNode* node);
	bool IsShallow(CSweepContext& tcx, SNode& node);
	bool IsEdgeSideOfTriangle(CTriangle& triangle, CPoint& ep, CPoint& eq);
	void FillEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node);
	void FillRightAboveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node);
	void FillRightBelowEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node);
	void FillRightConcaveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node);
	void FillRightConvexEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node);
	void FillLeftAboveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node);
	void FillLeftBelowEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node);
	void FillLeftConcaveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node);
	void FillLeftConvexEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node);
	void FlipEdgeEvent(CSweepContext& tcx, CPoint& ep, CPoint& eq, CTriangle* t, CPoint& p);
	CTriangle& NextFlipTriangle(CSweepContext& tcx, int o, CTriangle& t, CTriangle& ot, CPoint& p, CPoint& op);
	CPoint& NextFlipPoint(CPoint& ep, CPoint& eq, CTriangle& ot, CPoint& op);

	void FlipScanEdgeEvent(CSweepContext& tcx, CPoint& ep, CPoint& eq, CTriangle& flip_triangle, CTriangle& t, CPoint& p);

	void FinalizationPolygon(CSweepContext& tcx);

	std::vector<SNode*> nodes_;

};

}