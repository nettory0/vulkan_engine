#include <stdexcept>
#include "tg_sweep.h"
#include "tg_sweep_context.h"
#include "tg_utils.h"

namespace p2t
{

void CSweep::Triangulate(CSweepContext& tcx)
{
	tcx.InitTriangulation();
	tcx.CreateAdvancingFront(nodes_);
	SweepPoints(tcx);
	FinalizationPolygon(tcx);
}

void CSweep::SweepPoints(CSweepContext& tcx)
{
	for(size_t i = 1; i < tcx.point_count(); i++)
	{
		CPoint& point = *tcx.GetPoint(i);
		SNode* node = &PointEvent(tcx, point);
		for(unsigned int i = 0; i < point.edge_list.size(); i++)
		{
			EdgeEvent(tcx, point.edge_list[i], node);
		}
	}
}

void CSweep::FinalizationPolygon(CSweepContext& tcx)
{
	// Get an Internal triangle to start with
	CTriangle* t = tcx.front()->head()->next->triangle;
	CPoint* p = tcx.front()->head()->next->point;
	while(!t->GetConstrainedEdgeCW(*p))
	{
		t = t->NeighborCCW(*p);
	}

	tcx.MeshClean(*t);
}

SNode& CSweep::PointEvent(CSweepContext& tcx, CPoint& point)
{
	auto& node = tcx.LocateNode(point);
	auto& new_node = NewFrontTriangle(tcx, point, node);


	if(point.x() <= node.point->x() + EPSILON)
	{
		Fill(tcx, node);
	}

	FillAdvancingFront(tcx, new_node);
	return new_node;
}

void CSweep::EdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node)
{
	tcx.edge_event.constrained_edge = edge;
	tcx.edge_event.right = (edge->p->x() > edge->q->x());

	if(IsEdgeSideOfTriangle(*node->triangle, *edge->p, *edge->q))
	{
		return;
	}

	FillEdgeEvent(tcx, edge, node);
	EdgeEvent(tcx, *edge->p, *edge->q, node->triangle, *edge->q);
}

void CSweep::EdgeEvent(CSweepContext& tcx, CPoint& ep, CPoint& eq, CTriangle* triangle, CPoint& point)
{
	if(IsEdgeSideOfTriangle(*triangle, ep, eq))
	{
		return;
	}

	CPoint* p1 = triangle->PointCCW(point);
	EOrientation o1 = Orient2d(eq, *p1, ep);
	if(o1 == eCOLLINEAR)
	{
		if(triangle->Contains(&eq, p1))
		{
			triangle->MarkConstrainedEdge(&eq, p1);

			tcx.edge_event.constrained_edge->q = p1;
			triangle = &triangle->NeighborAcross(point);
			EdgeEvent(tcx, ep, *p1, triangle, *p1);
		}
		else
		{
			std::runtime_error("EdgeEvent - collinear points not supported");
			assert(0);
		}
		return;
	}

	CPoint* p2 = triangle->PointCW(point);
	EOrientation o2 = Orient2d(eq, *p2, ep);
	if(o2 == eCOLLINEAR)
	{
		if(triangle->Contains(&eq, p2))
		{
			triangle->MarkConstrainedEdge(&eq, p2);

			tcx.edge_event.constrained_edge->q = p2;
			triangle = &triangle->NeighborAcross(point);
			EdgeEvent(tcx, ep, *p2, triangle, *p2);
		}
		else
		{
			std::runtime_error("EdgeEvent - collinear points not supported");
			assert(0);
		}
		return;
	}

	if(o1 == o2)
	{

		if(o1 == eCW)
		{
			triangle = triangle->NeighborCCW(point);
		}
		else
		{
			triangle = triangle->NeighborCW(point);
		}
		EdgeEvent(tcx, ep, eq, triangle, point);
	}
	else
	{

		FlipEdgeEvent(tcx, ep, eq, triangle, point);
	}
}

bool CSweep::IsEdgeSideOfTriangle(CTriangle& triangle, CPoint& ep, CPoint& eq)
{
	const int index = triangle.EdgeIndex(&ep, &eq);

	if(index != -1)
	{
		triangle.MarkConstrainedEdge(index);
		CTriangle* t = triangle.GetNeighbor(index);
		if(t)
		{
			t->MarkConstrainedEdge(&ep, &eq);
		}
		return true;
	}
	return false;
}

SNode& CSweep::NewFrontTriangle(CSweepContext& tcx, CPoint& point, SNode& node)
{
	CTriangle* triangle = new CTriangle(point, *node.point, *node.next->point);

	triangle->MarkNeighbor(*node.triangle);
	tcx.AddToMap(triangle);

	SNode* new_node = new SNode(point);
	nodes_.push_back(new_node);

	new_node->next = node.next;
	new_node->prev = &node;
	node.next->prev = new_node;
	node.next = new_node;

	if(!Legalize(tcx, *triangle))
	{
		tcx.MapTriangleToNodes(*triangle);
	}

	return *new_node;
}

void CSweep::Fill(CSweepContext& tcx, SNode& node)
{
	CTriangle* triangle = new CTriangle(*node.prev->point, *node.point, *node.next->point);

	// TODO: should copy the constrained_edge value from neighbor triangles
	//       for now constrained_edge values are copied during the legalize
	triangle->MarkNeighbor(*node.prev->triangle);
	triangle->MarkNeighbor(*node.triangle);

	tcx.AddToMap(triangle);

	// Update the advancing front
	node.prev->next = node.next;
	node.next->prev = node.prev;

	// If it was legalized the triangle has already been mapped
	if(!Legalize(tcx, *triangle))
	{
		tcx.MapTriangleToNodes(*triangle);
	}

}

void CSweep::FillAdvancingFront(CSweepContext& tcx, SNode& n)
{

	// Fill right holes
	SNode* node = n.next;

	while(node->next)
	{
		// if HoleAngle exceeds 90 degrees then break.
		if(LargeHole_DontFill(node)) break;
		Fill(tcx, *node);
		node = node->next;
	}

	// Fill left holes
	node = n.prev;

	while(node->prev)
	{
		// if HoleAngle exceeds 90 degrees then break.
		if(LargeHole_DontFill(node)) break;
		Fill(tcx, *node);
		node = node->prev;
	}

	// Fill right basins
	if(n.next && n.next->next)
	{
		const double angle = BasinAngle(n);
		if(angle < PI_3div4)
		{
			FillBasin(tcx, n);
		}
	}
}

// True if HoleAngle exceeds 90 degrees.
bool CSweep::LargeHole_DontFill(const SNode* node) const
{

	const SNode* nextNode = node->next;
	const SNode* prevNode = node->prev;
	if(!AngleExceeds90Degrees(node->point, nextNode->point, prevNode->point))
		return false;

	// Check additional points on front.
	const SNode* next2Node = nextNode->next;
	// "..Plus.." because only want angles on same side as point being added.
	if((next2Node != NULL) && !AngleExceedsPlus90DegreesOrIsNegative(node->point, next2Node->point, prevNode->point))
		return false;

	const SNode* prev2Node = prevNode->prev;
	// "..Plus.." because only want angles on same side as point being added.
	if((prev2Node != NULL) && !AngleExceedsPlus90DegreesOrIsNegative(node->point, nextNode->point, prev2Node->point))
		return false;

	return true;
}

bool CSweep::AngleExceeds90Degrees(const CPoint* origin, const CPoint* pa, const CPoint* pb) const
{
	const double angle = Angle(origin, pa, pb);
	return ((angle > PI_div2) || (angle < -PI_div2));
}

bool CSweep::AngleExceedsPlus90DegreesOrIsNegative(const CPoint* origin, const CPoint* pa, const CPoint* pb) const
{
	const double angle = Angle(origin, pa, pb);
	return (angle > PI_div2) || (angle < 0);
}

double CSweep::Angle(const CPoint* origin, const CPoint* pa, const CPoint* pb) const
{
	/* Complex plane
	 * ab = cosA +i*sinA
	 * ab = (ax + ay*i)(bx + by*i) = (ax*bx + ay*by) + i(ax*by-ay*bx)
	 * atan2(y,x) computes the principal value of the argument function
	 * applied to the complex number x+iy
	 * Where x = ax*bx + ay*by
	 *       y = ax*by - ay*bx
	 */
	const double px = origin->x();
	const double py = origin->y();
	const double ax = pa->x() - px;
	const double ay = pa->y() - py;
	const double bx = pb->x() - px;
	const double by = pb->y() - py;
	const double x = ax * by - ay * bx;
	const double y = ax * bx + ay * by;
	return atan2(x, y);
}

double CSweep::BasinAngle(const SNode& node) const
{
	const double ax = node.point->x() - node.next->next->point->x();
	const double ay = node.point->y() - node.next->next->point->y();
	return atan2(ay, ax);
}

double CSweep::HoleAngle(const SNode& node) const
{
	/* Complex plane
	 * ab = cosA +i*sinA
	 * ab = (ax + ay*i)(bx + by*i) = (ax*bx + ay*by) + i(ax*by-ay*bx)
	 * atan2(y,x) computes the principal value of the argument function
	 * applied to the complex number x+iy
	 * Where x = ax*bx + ay*by
	 *       y = ax*by - ay*bx
	 */
	const double ax = node.next->point->x() - node.point->x();
	const double ay = node.next->point->y() - node.point->y();
	const double bx = node.prev->point->x() - node.point->x();
	const double by = node.prev->point->y() - node.point->y();
	return atan2(ax * by - ay * bx, ax * bx + ay * by);
}

bool CSweep::Legalize(CSweepContext& tcx, CTriangle& t)
{
	// To legalize a triangle we start by finding if any of the three edges
	// violate the Delaunay condition
	for(int i = 0; i < 3; i++)
	{
		if(t.delaunay_edge[i])
			continue;

		CTriangle* ot = t.GetNeighbor(i);

		if(ot)
		{
			CPoint* p = t.GetPoint(i);
			CPoint* op = ot->OppositePoint(t, *p);
			int oi = ot->Index(op);

			// If this is a Constrained Edge or a Delaunay Edge(only during recursive legalization)
			// then we should not try to legalize
			if(ot->constrained_edge[oi] || ot->delaunay_edge[oi])
			{
				t.constrained_edge[i] = ot->constrained_edge[oi];
				continue;
			}

			bool inside = Incircle(*p, *t.PointCCW(*p), *t.PointCW(*p), *op);

			if(inside)
			{
				// Lets mark this shared edge as Delaunay
				t.delaunay_edge[i] = true;
				ot->delaunay_edge[oi] = true;

				// Lets rotate shared edge one vertex CW to legalize it
				RotateTrianglePair(t, *p, *ot, *op);

				// We now got one valid Delaunay Edge shared by two triangles
				// This gives us 4 new edges to check for Delaunay

				// Make sure that triangle to node mapping is done only one time for a specific triangle
				bool not_legalized = !Legalize(tcx, t);
				if(not_legalized)
				{
					tcx.MapTriangleToNodes(t);
				}

				not_legalized = !Legalize(tcx, *ot);
				if(not_legalized)
					tcx.MapTriangleToNodes(*ot);

				// Reset the Delaunay edges, since they only are valid Delaunay edges
				// until we add a new triangle or point.
				// XXX: need to think about this. Can these edges be tried after we
				//      return to previous recursive level?
				t.delaunay_edge[i] = false;
				ot->delaunay_edge[oi] = false;

				// If triangle have been legalized no need to check the other edges since
				// the recursive legalization will handles those so we can end here.
				return true;
			}
		}
	}
	return false;
}

bool CSweep::Incircle(const CPoint& pa, const CPoint& pb, const CPoint& pc, const CPoint& pd) const
{
	const double adx = pa.x() - pd.x();
	const double ady = pa.y() - pd.y();
	const double bdx = pb.x() - pd.x();
	const double bdy = pb.y() - pd.y();

	const double adxbdy = adx * bdy;
	const double bdxady = bdx * ady;
	const double oabd = adxbdy - bdxady;

	if(oabd <= 0)
		return false;

	const double cdx = pc.x() - pd.x();
	const double cdy = pc.y() - pd.y();

	const double cdxady = cdx * ady;
	const double adxcdy = adx * cdy;
	const double ocad = cdxady - adxcdy;

	if(ocad <= 0)
		return false;

	const double bdxcdy = bdx * cdy;
	const double cdxbdy = cdx * bdy;

	const double alift = adx * adx + ady * ady;
	const double blift = bdx * bdx + bdy * bdy;
	const double clift = cdx * cdx + cdy * cdy;

	const double det = alift * (bdxcdy - cdxbdy) + blift * ocad + clift * oabd;

	return det > 0;
}

void CSweep::RotateTrianglePair(CTriangle& t, CPoint& p, CTriangle& ot, CPoint& op) const
{
	CTriangle* n1, * n2, * n3, * n4;
	n1 = t.NeighborCCW(p);
	n2 = t.NeighborCW(p);
	n3 = ot.NeighborCCW(op);
	n4 = ot.NeighborCW(op);

	bool ce1, ce2, ce3, ce4;
	ce1 = t.GetConstrainedEdgeCCW(p);
	ce2 = t.GetConstrainedEdgeCW(p);
	ce3 = ot.GetConstrainedEdgeCCW(op);
	ce4 = ot.GetConstrainedEdgeCW(op);

	bool de1, de2, de3, de4;
	de1 = t.GetDelunayEdgeCCW(p);
	de2 = t.GetDelunayEdgeCW(p);
	de3 = ot.GetDelunayEdgeCCW(op);
	de4 = ot.GetDelunayEdgeCW(op);

	t.Legalize(p, op);
	ot.Legalize(op, p);

	// Remap delaunay_edge
	ot.SetDelunayEdgeCCW(p, de1);
	t.SetDelunayEdgeCW(p, de2);
	t.SetDelunayEdgeCCW(op, de3);
	ot.SetDelunayEdgeCW(op, de4);

	// Remap constrained_edge
	ot.SetConstrainedEdgeCCW(p, ce1);
	t.SetConstrainedEdgeCW(p, ce2);
	t.SetConstrainedEdgeCCW(op, ce3);
	ot.SetConstrainedEdgeCW(op, ce4);

	// Remap neighbors
	// XXX: might optimize the markNeighbor by keeping track of
	//      what side should be assigned to what neighbor after the
	//      rotation. Now mark neighbor does lots of testing to find
	//      the right side.
	t.ClearNeighbors();
	ot.ClearNeighbors();
	if(n1) ot.MarkNeighbor(*n1);
	if(n2) t.MarkNeighbor(*n2);
	if(n3) t.MarkNeighbor(*n3);
	if(n4) ot.MarkNeighbor(*n4);
	t.MarkNeighbor(ot);
}

void CSweep::FillBasin(CSweepContext& tcx, SNode& node)
{
	if(Orient2d(*node.point, *node.next->point, *node.next->next->point) == eCCW)
	{
		tcx.basin.left_node = node.next->next;
	}
	else
	{
		tcx.basin.left_node = node.next;
	}

	// Find the bottom and right node
	tcx.basin.bottom_node = tcx.basin.left_node;
	while(tcx.basin.bottom_node->next
		&& tcx.basin.bottom_node->point->y() >= tcx.basin.bottom_node->next->point->y())
	{
		tcx.basin.bottom_node = tcx.basin.bottom_node->next;
	}
	if(tcx.basin.bottom_node == tcx.basin.left_node)
	{
		// No valid basin
		return;
	}

	tcx.basin.right_node = tcx.basin.bottom_node;
	while(tcx.basin.right_node->next
		&& tcx.basin.right_node->point->y() < tcx.basin.right_node->next->point->y())
	{
		tcx.basin.right_node = tcx.basin.right_node->next;
	}
	if(tcx.basin.right_node == tcx.basin.bottom_node)
	{
		// No valid basins
		return;
	}

	tcx.basin.width = tcx.basin.right_node->point->x() - tcx.basin.left_node->point->x();
	tcx.basin.left_highest = tcx.basin.left_node->point->y() > tcx.basin.right_node->point->y();

	FillBasinReq(tcx, tcx.basin.bottom_node);
}

void CSweep::FillBasinReq(CSweepContext& tcx, SNode* node)
{
	// if shallow stop filling
	if(IsShallow(tcx, *node))
	{
		return;
	}

	Fill(tcx, *node);

	if(node->prev == tcx.basin.left_node && node->next == tcx.basin.right_node)
	{
		return;
	}
	else if(node->prev == tcx.basin.left_node)
	{
		EOrientation o = Orient2d(*node->point, *node->next->point, *node->next->next->point);
		if(o == eCW)
		{
			return;
		}
		node = node->next;
	}
	else if(node->next == tcx.basin.right_node)
	{
		EOrientation o = Orient2d(*node->point, *node->prev->point, *node->prev->prev->point);
		if(o == eCCW)
		{
			return;
		}
		node = node->prev;
	}
	else
	{
		// Continue with the neighbor node with lowest Y value
		if(node->prev->point->y() < node->next->point->y())
		{
			node = node->prev;
		}
		else
		{
			node = node->next;
		}
	}

	FillBasinReq(tcx, node);
}

bool CSweep::IsShallow(CSweepContext& tcx, SNode& node)
{
	double height;

	if(tcx.basin.left_highest)
	{
		height = tcx.basin.left_node->point->y() - node.point->y();
	}
	else
	{
		height = tcx.basin.right_node->point->y() - node.point->y();
	}

	// if shallow stop filling
	if(tcx.basin.width > height)
	{
		return true;
	}
	return false;
}

void CSweep::FillEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node)
{
	if(tcx.edge_event.right)
	{
		FillRightAboveEdgeEvent(tcx, edge, node);
	}
	else
	{
		FillLeftAboveEdgeEvent(tcx, edge, node);
	}
}

void CSweep::FillRightAboveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node)
{
	while(node->next->point->x() < edge->p->x())
	{
		// Check if next node is below the edge
		if(Orient2d(*edge->q, *node->next->point, *edge->p) == eCCW)
		{
			FillRightBelowEdgeEvent(tcx, edge, *node);
		}
		else
		{
			node = node->next;
		}
	}
}

void CSweep::FillRightBelowEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node)
{
	if(node.point->x() < edge->p->x())
	{
		if(Orient2d(*node.point, *node.next->point, *node.next->next->point) == eCCW)
		{
			// Concave
			FillRightConcaveEdgeEvent(tcx, edge, node);
		}
		else
		{
			// Convex
			FillRightConvexEdgeEvent(tcx, edge, node);
			// Retry this one
			FillRightBelowEdgeEvent(tcx, edge, node);
		}
	}
}

void CSweep::FillRightConcaveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node)
{
	Fill(tcx, *node.next);
	if(node.next->point != edge->p)
	{
		// Next above or below edge?
		if(Orient2d(*edge->q, *node.next->point, *edge->p) == eCCW)
		{
			// Below
			if(Orient2d(*node.point, *node.next->point, *node.next->next->point) == eCCW)
			{
				// Next is concave
				FillRightConcaveEdgeEvent(tcx, edge, node);
			}
			else
			{
				// Next is convex
			}
		}
	}

}

void CSweep::FillRightConvexEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node)
{
	// Next concave or convex?
	if(Orient2d(*node.next->point, *node.next->next->point, *node.next->next->next->point) == eCCW)
	{
		// Concave
		FillRightConcaveEdgeEvent(tcx, edge, *node.next);
	}
	else
	{
		// Convex
		// Next above or below edge?
		if(Orient2d(*edge->q, *node.next->next->point, *edge->p) == eCCW)
		{
			// Below
			FillRightConvexEdgeEvent(tcx, edge, *node.next);
		}
		else
		{
			// Above
		}
	}
}

void CSweep::FillLeftAboveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode* node)
{
	while(node->prev->point->x() > edge->p->x())
	{
		// Check if next node is below the edge
		if(Orient2d(*edge->q, *node->prev->point, *edge->p) == eCW)
		{
			FillLeftBelowEdgeEvent(tcx, edge, *node);
		}
		else
		{
			node = node->prev;
		}
	}
}

void CSweep::FillLeftBelowEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node)
{
	if(node.point->x() > edge->p->x())
	{
		if(Orient2d(*node.point, *node.prev->point, *node.prev->prev->point) == eCW)
		{
			// Concave
			FillLeftConcaveEdgeEvent(tcx, edge, node);
		}
		else
		{
			// Convex
			FillLeftConvexEdgeEvent(tcx, edge, node);
			// Retry this one
			FillLeftBelowEdgeEvent(tcx, edge, node);
		}
	}
}

void CSweep::FillLeftConvexEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node)
{
	// Next concave or convex?
	if(Orient2d(*node.prev->point, *node.prev->prev->point, *node.prev->prev->prev->point) == eCW)
	{
		// Concave
		FillLeftConcaveEdgeEvent(tcx, edge, *node.prev);
	}
	else
	{
		// Convex
		// Next above or below edge?
		if(Orient2d(*edge->q, *node.prev->prev->point, *edge->p) == eCW)
		{
			// Below
			FillLeftConvexEdgeEvent(tcx, edge, *node.prev);
		}
		else
		{
			// Above
		}
	}
}

void CSweep::FillLeftConcaveEdgeEvent(CSweepContext& tcx, CEdge* edge, SNode& node)
{
	Fill(tcx, *node.prev);
	if(node.prev->point != edge->p)
	{
		// Next above or below edge?
		if(Orient2d(*edge->q, *node.prev->point, *edge->p) == eCW)
		{
			// Below
			if(Orient2d(*node.point, *node.prev->point, *node.prev->prev->point) == eCW)
			{
				// Next is concave
				FillLeftConcaveEdgeEvent(tcx, edge, node);
			}
			else
			{
				// Next is convex
			}
		}
	}

}

void CSweep::FlipEdgeEvent(CSweepContext& tcx, CPoint& ep, CPoint& eq, CTriangle* t, CPoint& p)
{
	CTriangle& ot = t->NeighborAcross(p);
	CPoint& op = *ot.OppositePoint(*t, p);

	if(InScanArea(p, *t->PointCCW(p), *t->PointCW(p), op))
	{
		// Lets rotate shared edge one vertex CW
		RotateTrianglePair(*t, p, ot, op);
		tcx.MapTriangleToNodes(*t);
		tcx.MapTriangleToNodes(ot);

		if(p == eq && op == ep)
		{
			if(eq == *tcx.edge_event.constrained_edge->q && ep == *tcx.edge_event.constrained_edge->p)
			{
				t->MarkConstrainedEdge(&ep, &eq);
				ot.MarkConstrainedEdge(&ep, &eq);
				Legalize(tcx, *t);
				Legalize(tcx, ot);
			}
			else
			{
				// XXX: I think one of the triangles should be legalized here?
			}
		}
		else
		{
			EOrientation o = Orient2d(eq, op, ep);
			t = &NextFlipTriangle(tcx, (int)o, *t, ot, p, op);
			FlipEdgeEvent(tcx, ep, eq, t, p);
		}
	}
	else
	{
		CPoint& newP = NextFlipPoint(ep, eq, ot, op);
		FlipScanEdgeEvent(tcx, ep, eq, *t, ot, newP);
		EdgeEvent(tcx, ep, eq, t, p);
	}
}

CTriangle& CSweep::NextFlipTriangle(CSweepContext& tcx, int o, CTriangle& t, CTriangle& ot, CPoint& p, CPoint& op)
{
	if(o == eCCW)
	{
		// ot is not crossing edge after flip
		int edge_index = ot.EdgeIndex(&p, &op);
		ot.delaunay_edge[edge_index] = true;
		Legalize(tcx, ot);
		ot.ClearDelunayEdges();
		return t;
	}

	// t is not crossing edge after flip
	int edge_index = t.EdgeIndex(&p, &op);

	t.delaunay_edge[edge_index] = true;
	Legalize(tcx, t);
	t.ClearDelunayEdges();
	return ot;
}

CPoint& CSweep::NextFlipPoint(CPoint& ep, CPoint& eq, CTriangle& ot, CPoint& op)
{
	EOrientation o2d = Orient2d(eq, op, ep);
	if(o2d == eCW)
	{
		// Right
		return *ot.PointCCW(op);
	}
	else if(o2d == eCCW)
	{
		// Left
		return *ot.PointCW(op);
	}
	throw std::runtime_error("[Unsupported] Opposing point on constrained edge");
}

void CSweep::FlipScanEdgeEvent(CSweepContext& tcx, CPoint& ep, CPoint& eq, CTriangle& flip_triangle,
	CTriangle& t, CPoint& p)
{
	CTriangle& ot = t.NeighborAcross(p);
	CPoint& op = *ot.OppositePoint(t, p);

	if(InScanArea(eq, *flip_triangle.PointCCW(eq), *flip_triangle.PointCW(eq), op))
	{
		FlipEdgeEvent(tcx, eq, op, &ot, op);
	}
	else
	{
		CPoint& newP = NextFlipPoint(ep, eq, ot, op);
		FlipScanEdgeEvent(tcx, ep, eq, flip_triangle, ot, newP);
	}
}

CSweep::~CSweep()
{
	for(size_t i = 0; i < nodes_.size(); i++)
	{
		delete nodes_[i];
	}

}

}

