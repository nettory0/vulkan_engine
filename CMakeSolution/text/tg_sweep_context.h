#pragma once

#include <list>
#include <vector>
#include <cstddef>
#include "tg_shapes.h"
#include "tg_sweep.h"

namespace p2t
{

// Inital triangle factor, seed triangle will extend 30% of
// PointSet width to both left and right.
const double kAlpha = 0.3;

struct CPoint;
class CTriangle;
struct SNode;
struct CEdge;
class CAdvancingFront;

class CSweepContext
{
public:

	/// Constructor
	CSweepContext(const std::vector<CPoint*>& polyline);
	/// Destructor
	~CSweepContext();

	void set_head(CPoint* p1);

	CPoint* head() const;

	void set_tail(CPoint* p1);

	CPoint* tail() const;

	size_t point_count() const;

	SNode& LocateNode(const CPoint& point);

	void RemoveNode(SNode* node);

	void CreateAdvancingFront(const std::vector<SNode*>& nodes);

	/// Try to map a node to all sides of this triangle that don't have a neighbor
	void MapTriangleToNodes(CTriangle& t);

	void AddToMap(CTriangle* triangle);

	CPoint* GetPoint(size_t index);

	CPoint* GetPoints();

	void RemoveFromMap(CTriangle* triangle);

	void AddHole(const std::vector<CPoint*>& polyline);

	void AddPoint(CPoint* point);

	CAdvancingFront* front() const;

	void MeshClean(CTriangle& triangle);

	std::vector<CTriangle*>& GetTriangles();
	std::list<CTriangle*>& GetMap();

	std::vector<CEdge*> edge_list;

	struct SBasin
	{
		SNode* left_node;
		SNode* bottom_node;
		SNode* right_node;
		double width;
		bool left_highest;

		SBasin(): left_node(NULL), bottom_node(NULL), right_node(NULL), width(0.0), left_highest(false)
		{
		}

		void Clear()
		{
			left_node = NULL;
			bottom_node = NULL;
			right_node = NULL;
			width = 0.0;
			left_highest = false;
		}
	};

	struct EdgeEvent
	{
		CEdge* constrained_edge;
		bool right;

		EdgeEvent(): constrained_edge(NULL), right(false)
		{
		}
	};

	SBasin basin;
	EdgeEvent edge_event;

private:

	friend class CSweep;

	std::vector<CTriangle*> m_aTrngVector;
	std::list<CTriangle*> m_aTrngList;
	std::vector<CPoint*> m_aPoints;

	// Advancing front
	CAdvancingFront* m_front;
	// head point used with advancing front
	CPoint* head_;
	// tail point used with advancing front
	CPoint* tail_;

	SNode* af_head_, * af_middle_, * af_tail_;

	void InitTriangulation();
	void InitEdges(const std::vector<CPoint*>& polyline);

};

inline CAdvancingFront* CSweepContext::front() const
{
	return m_front;
}

inline size_t CSweepContext::point_count() const
{
	return m_aPoints.size();
}

inline void CSweepContext::set_head(CPoint* p1)
{
	head_ = p1;
}

inline CPoint* CSweepContext::head() const
{
	return head_;
}

inline void CSweepContext::set_tail(CPoint* p1)
{
	tail_ = p1;
}

inline CPoint* CSweepContext::tail() const
{
	return tail_;
}


struct SNode;

// Advancing front node
struct SNode
{
	CPoint* point;
	CTriangle* triangle;

	SNode* next;
	SNode* prev;

	double value;

	SNode(CPoint& p): point(&p), triangle(NULL), next(NULL), prev(NULL), value(p.x())
	{
	}

	SNode(CPoint& p, CTriangle& t): point(&p), triangle(&t), next(NULL), prev(NULL), value(p.x())
	{
	}

};

// Advancing front
class CAdvancingFront
{
public:

	CAdvancingFront(SNode& head, SNode& tail);
	// Destructor
	~CAdvancingFront();

	SNode* head();
	void set_head(SNode* node);
	SNode* tail();
	void set_tail(SNode* node);
	SNode* search();
	void set_search(SNode* node);

	/// Locate insertion point along advancing front
	SNode* LocateNode(double x);

	SNode* LocatePoint(const CPoint* point);

private:

	SNode* head_, * tail_, * search_node_;

	SNode* FindSearchNode(double x);
};

inline SNode* CAdvancingFront::head()
{
	return head_;
}
inline void CAdvancingFront::set_head(SNode* node)
{
	head_ = node;
}

inline SNode* CAdvancingFront::tail()
{
	return tail_;
}
inline void CAdvancingFront::set_tail(SNode* node)
{
	tail_ = node;
}

inline SNode* CAdvancingFront::search()
{
	return search_node_;
}

inline void CAdvancingFront::set_search(SNode* node)
{
	search_node_ = node;
}


class CCDT
{
public:

	CCDT(const std::vector<CPoint*>& polyline);
	~CCDT();
	void AddHole(const std::vector<CPoint*>& polyline);
	void Triangulate();
	std::vector<CTriangle*> GetTriangles();
	std::list<CTriangle*> GetMap();

private:
	CSweepContext m_sweepContext;
	CSweep m_seep;

};

}

