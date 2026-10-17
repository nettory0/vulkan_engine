#include <algorithm>
#include "tg_sweep_context.h"
#include "tg_sweep.h"

namespace p2t {

CSweepContext::CSweepContext(const std::vector<CPoint*>& polyline) : m_aPoints(polyline),
  m_front(0),
  head_(0),
  tail_(0),
  af_head_(0),
  af_middle_(0),
  af_tail_(0)
{
  InitEdges(m_aPoints);
}

void CSweepContext::AddHole(const std::vector<CPoint*>& polyline)
{
  InitEdges(polyline);
  for(unsigned int i = 0; i < polyline.size(); i++) {
    m_aPoints.push_back(polyline[i]);
  }
}

void CSweepContext::AddPoint(CPoint* point) {
  m_aPoints.push_back(point);
}

std::vector<CTriangle*> &CSweepContext::GetTriangles()
{
  return m_aTrngVector;
}

std::list<CTriangle*> &CSweepContext::GetMap()
{
  return m_aTrngList;
}

void CSweepContext::InitTriangulation()
{
  double xmax(m_aPoints[0]->x()), xmin(m_aPoints[0]->x());
  double ymax(m_aPoints[0]->y()), ymin(m_aPoints[0]->y());

  // Calculate bounds.
  for (unsigned int i = 0; i < m_aPoints.size(); i++) {
    CPoint& p = *m_aPoints[i];
    if (p.x() > xmax)
      xmax = p.x();
    if (p.x() < xmin)
      xmin = p.x();
    if (p.y() > ymax)
      ymax = p.y();
    if (p.y() < ymin)
      ymin = p.y();
  }

  double dx = kAlpha * (xmax - xmin);
  double dy = kAlpha * (ymax - ymin);
  head_ = new CPoint(xmax + dx, ymin - dy);
  tail_ = new CPoint(xmin - dx, ymin - dy);

  // Sort points along y-axis
  std::sort(m_aPoints.begin(), m_aPoints.end(), cmp);

}

void CSweepContext::InitEdges(const std::vector<CPoint*>& polyline)
{
  size_t num_points = polyline.size();
  for (size_t i = 0; i < num_points; i++) {
    size_t j = i < num_points - 1 ? i + 1 : 0;
    edge_list.push_back(new CEdge(*polyline[i], *polyline[j]));
  }
}

CPoint* CSweepContext::GetPoint(size_t index)
{
  return m_aPoints[index];
}

void CSweepContext::AddToMap(CTriangle* triangle)
{
  m_aTrngList.push_back(triangle);
}

SNode& CSweepContext::LocateNode(const CPoint& point)
{
  // TODO implement search tree
  return *m_front->LocateNode(point.x());
}

void CSweepContext::CreateAdvancingFront(const std::vector<SNode*>& nodes)
{

  (void) nodes;
  // Initial triangle
  CTriangle* triangle = new CTriangle(*m_aPoints[0], *tail_, *head_);

  m_aTrngList.push_back(triangle);

  af_head_ = new SNode(*triangle->GetPoint(1), *triangle);
  af_middle_ = new SNode(*triangle->GetPoint(0), *triangle);
  af_tail_ = new SNode(*triangle->GetPoint(2));
  m_front = new CAdvancingFront(*af_head_, *af_tail_);

  // TODO: More intuitive if head is middles next and not previous?
  //       so swap head and tail
  af_head_->next = af_middle_;
  af_middle_->next = af_tail_;
  af_middle_->prev = af_head_;
  af_tail_->prev = af_middle_;
}

void CSweepContext::RemoveNode(SNode* node)
{
  delete node;
}

void CSweepContext::MapTriangleToNodes(CTriangle& t)
{
  for (int i = 0; i < 3; i++) {
    if (!t.GetNeighbor(i)) {
      SNode* n = m_front->LocatePoint(t.PointCW(*t.GetPoint(i)));
      if (n)
        n->triangle = &t;
    }
  }
}

void CSweepContext::RemoveFromMap(CTriangle* triangle)
{
  m_aTrngList.remove(triangle);
}

void CSweepContext::MeshClean(CTriangle& triangle)
{
  std::vector<CTriangle *> triangles;
  triangles.push_back(&triangle);

  while(!triangles.empty()){
	CTriangle *t = triangles.back();
	triangles.pop_back();

    if (t != NULL && !t->IsInterior()) {
      t->IsInterior(true);
      m_aTrngVector.push_back(t);
      for (int i = 0; i < 3; i++) {
        if (!t->constrained_edge[i])
          triangles.push_back(t->GetNeighbor(i));
      }
    }
  }
}

CSweepContext::~CSweepContext()
{

    // Clean up memory

    delete head_;
    delete tail_;
    delete m_front;
    delete af_head_;
    delete af_middle_;
    delete af_tail_;

    typedef std::list<CTriangle*> type_list;

    for(type_list::iterator iter = m_aTrngList.begin(); iter != m_aTrngList.end(); ++iter) {
        CTriangle* ptr = *iter;
        delete ptr;
    }

     for(unsigned int i = 0; i < edge_list.size(); i++) {
        delete edge_list[i];
    }

}

CAdvancingFront::CAdvancingFront(SNode& head, SNode& tail)
{
    head_ = &head;
    tail_ = &tail;
    search_node_ = &head;
}

SNode* CAdvancingFront::LocateNode(double x)
{
    SNode* node = search_node_;

    if(x < node->value)
    {
        while((node = node->prev) != NULL)
        {
            if(x >= node->value)
            {
                search_node_ = node;
                return node;
            }
        }
    }
    else
    {
        while((node = node->next) != NULL)
        {
            if(x < node->value)
            {
                search_node_ = node->prev;
                return node->prev;
            }
        }
    }
    return NULL;
}

SNode* CAdvancingFront::FindSearchNode(double x)
{
    (void)x; // suppress compiler warnings "unused parameter 'x'"
    // TODO: implement BST index
    return search_node_;
}

SNode* CAdvancingFront::LocatePoint(const CPoint* point)
{
    const double px = point->x();
    SNode* node = FindSearchNode(px);
    const double nx = node->point->x();

    if(px == nx)
    {
        if(point != node->point)
        {
            // We might have two nodes with same x value for a short time
            if(point == node->prev->point)
            {
                node = node->prev;
            }
            else if(point == node->next->point)
            {
                node = node->next;
            }
            else
            {
                assert(0);
            }
        }
    }
    else if(px < nx)
    {
        while((node = node->prev) != NULL)
        {
            if(point == node->point)
            {
                break;
            }
        }
    }
    else
    {
        while((node = node->next) != NULL)
        {
            if(point == node->point)
                break;
        }
    }
    if(node) search_node_ = node;
    return node;
}

CAdvancingFront::~CAdvancingFront()
{
}


CCDT::CCDT(const std::vector<CPoint*>& polyline):m_sweepContext(polyline)
{
}

void CCDT::AddHole(const std::vector<CPoint*>& polyline)
{
    m_sweepContext.AddHole(polyline);
}

void CCDT::Triangulate()
{
    m_seep.Triangulate(m_sweepContext);
}

std::vector<p2t::CTriangle*> CCDT::GetTriangles()
{
    return m_sweepContext.GetTriangles();
}

std::list<p2t::CTriangle*> CCDT::GetMap()
{
    return m_sweepContext.GetMap();
}

CCDT::~CCDT()
{
}

}
