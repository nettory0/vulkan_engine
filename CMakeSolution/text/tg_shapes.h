#pragma once

#include <vector>
#include <array>
#include <cstddef>
#include <assert.h>
#include <cmath>

namespace p2t
{

struct CEdge;

struct CPoint
{

	std::array<double, 2> xY{0., 0.};

	CPoint()
	{
	}
	double& x(){return xY[0];};
	double& y(){return xY[1];};
	const double& x() const {return xY[0];};
	const double& y() const {return xY[1];};
	std::vector<CEdge*> edge_list;

	CPoint(double x, double y): xY{x, y}
	{
	}

	void set_zero()
	{
		x() = 0.0;
		y() = 0.0;
	}

	void set(double x_, double y_)
	{
		x() = x_;
		y() = y_;
	}

	CPoint operator -() const
	{
		CPoint v;
		v.set(-x(), -y());
		return v;
	}

	void operator +=(const CPoint& v)
	{
		x() += v.x();
		y() += v.y();
	}

	void operator -=(const CPoint& v)
	{
		x() -= v.x();
		y() -= v.y();
	}

	void operator *=(double a)
	{
		x() *= a;
		y() *= a;
	}

	double Length() const
	{
		return sqrt(x() * x() + y() * y());
	}

	double Normalize()
	{
		const double len = Length();
		x() /= len;
		y() /= len;
		return len;
	}

};

struct CEdge
{

	CPoint* p;
	CPoint* q;


	CEdge(CPoint& p1, CPoint& p2): p(&p1), q(&p2)
	{
		if(p1.y() > p2.y())
		{
			q = &p1;
			p = &p2;
		}
		else if(p1.y() == p2.y())
		{
			if(p1.x() > p2.x())
			{
				q = &p1;
				p = &p2;
			}
			else if(p1.x() == p2.x())
			{
				assert(false);
			}
		}

		q->edge_list.push_back(this);
	}
};


class CTriangle
{
public:


	CTriangle(CPoint& a, CPoint& b, CPoint& c);

	bool constrained_edge[3];
	bool delaunay_edge[3];

	CPoint* GetPoint(int index);
	CPoint* PointCW(const CPoint& point);
	CPoint* PointCCW(const CPoint& point);
	CPoint* OppositePoint(CTriangle& t, const CPoint& p);

	CTriangle* GetNeighbor(int index);
	void MarkNeighbor(CPoint* p1, CPoint* p2, CTriangle* t);
	void MarkNeighbor(CTriangle& t);

	void MarkConstrainedEdge(int index);
	void MarkConstrainedEdge(CEdge& edge);
	void MarkConstrainedEdge(CPoint* p, CPoint* q);

	int Index(const CPoint* p);
	int EdgeIndex(const CPoint* p1, const CPoint* p2);

	CTriangle* NeighborCW(const CPoint& point);
	CTriangle* NeighborCCW(const CPoint& point);
	bool GetConstrainedEdgeCCW(const CPoint& p);
	bool GetConstrainedEdgeCW(const CPoint& p);
	void SetConstrainedEdgeCCW(const CPoint& p, bool ce);
	void SetConstrainedEdgeCW(const CPoint& p, bool ce);
	bool GetDelunayEdgeCCW(const CPoint& p);
	bool GetDelunayEdgeCW(const CPoint& p);
	void SetDelunayEdgeCCW(const CPoint& p, bool e);
	void SetDelunayEdgeCW(const CPoint& p, bool e);

	bool Contains(const CPoint* p);
	bool Contains(const CEdge& e);
	bool Contains(const CPoint* p, const CPoint* q);
	void Legalize(CPoint& point);
	void Legalize(CPoint& opoint, CPoint& npoint);
	void Clear();
	void ClearNeighbor(const CTriangle* triangle);
	void ClearNeighbors();
	void ClearDelunayEdges();

	inline bool IsInterior();
	inline void IsInterior(bool b);

	CTriangle& NeighborAcross(const CPoint& opoint);

	void DebugPrint();

private:


	CPoint* points_[3];
	CTriangle* neighbors_[3];


	bool interior_;
};

inline bool cmp(const CPoint* a, const CPoint* b)
{
	if(a->y() < b->y())
	{
		return true;
	}
	else if(a->y() == b->y())
	{
		if(a->x() < b->x())
		{
			return true;
		}
	}
	return false;
}


inline CPoint operator +(const CPoint& a, const CPoint& b)
{
	return CPoint(a.x() + b.x(), a.y() + b.y());
}


inline CPoint operator -(const CPoint& a, const CPoint& b)
{
	return CPoint(a.x() - b.x(), a.y() - b.y());
}


inline CPoint operator *(double s, const CPoint& a)
{
	return CPoint(s * a.x(), s * a.y());
}

inline bool operator ==(const CPoint& a, const CPoint& b)
{
	return a.x() == b.x() && a.y() == b.y();
}

inline bool operator !=(const CPoint& a, const CPoint& b)
{
	return !(a.x() == b.x()) && !(a.y() == b.y());
}

inline double Dot(const CPoint& a, const CPoint& b)
{
	return a.x() * b.x() + a.y() * b.y();
}

inline double Cross(const CPoint& a, const CPoint& b)
{
	return a.x() * b.y() - a.y() * b.x();
}

inline CPoint Cross(const CPoint& a, double s)
{
	return CPoint(s * a.y(), -s * a.x());
}

inline CPoint Cross(double s, const CPoint& a)
{
	return CPoint(-s * a.y(), s * a.x());
}

inline CPoint* CTriangle::GetPoint(int index)
{
	return points_[index];
}

inline CTriangle* CTriangle::GetNeighbor(int index)
{
	return neighbors_[index];
}

inline bool CTriangle::Contains(const CPoint* p)
{
	return p == points_[0] || p == points_[1] || p == points_[2];
}

inline bool CTriangle::Contains(const CEdge& e)
{
	return Contains(e.p) && Contains(e.q);
}

inline bool CTriangle::Contains(const CPoint* p, const CPoint* q)
{
	return Contains(p) && Contains(q);
}

inline bool CTriangle::IsInterior()
{
	return interior_;
}

inline void CTriangle::IsInterior(bool b)
{
	interior_ = b;
}

}
