/*
 * Based on Poly2Tri (https://github.com/jhasse/poly2tri)
 * Copyright (c) 2009-2018, Poly2Tri Contributors. All rights reserved.
 * Licensed under the BSD 3-Clause License, see third_party_licenses/poly2tri-LICENSE.txt
 *
 * Modified by nettory0, 2026: renamed types, adapted for glyph triangulation.
 */
#pragma once
// Otherwise #defines like M_PI are undeclared under Visual Studio
#define _USE_MATH_DEFINES

#include <exception>
#include <math.h>
#include "tg_shapes.h"
// C99 removes M_PI from math.h
#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

namespace p2t 
{

const double PI_3div4 = 3 * M_PI / 4;
const double PI_div2 = 1.57079632679489661923;
const double EPSILON = 1e-12;

enum EOrientation { eCW, eCCW, eCOLLINEAR };

EOrientation Orient2d(const CPoint& pa, const CPoint& pb, const CPoint& pc)
{
  double detleft = (pa.x() - pc.x()) * (pb.y() - pc.y());
  double detright = (pa.y() - pc.y()) * (pb.x() - pc.x());
  double val = detleft - detright;
  if (val > -EPSILON && val < EPSILON) {
    return eCOLLINEAR;
  } else if (val > 0) {
    return eCCW;
  }
  return eCW;
}


bool InScanArea(const CPoint& pa, const CPoint& pb, const CPoint& pc, const CPoint& pd)
{
  double oadb = (pa.x() - pb.x())*(pd.y() - pb.y()) - (pd.x() - pb.x())*(pa.y() - pb.y());
  if (oadb >= -EPSILON) {
    return false;
  }

  double oadc = (pa.x() - pc.x())*(pd.y() - pc.y()) - (pd.x() - pc.x())*(pa.y() - pc.y());
  if (oadc <= EPSILON) {
    return false;
  }
  return true;
}

}

