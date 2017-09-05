#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cmath>
#include <limits>
#include <QVector3D>

#include "Triangulate.h"


static const float EPSILON=0.0000000001f;

float Triangulate::Area(const QVector3D* contour,
                        unsigned short stripe,
                        const std::vector<unsigned short> &contour_idxs)
{

  int n = contour_idxs.size();

  float A=0.0f;

  for(int p=n-1,q=0; q<n; p=q++)
  {

    A+= ((QVector3D*)((unsigned char*)contour + contour_idxs[p] * stripe ))->x() *
        ((QVector3D*)((unsigned char*)contour + contour_idxs[q] * stripe ))->y() -
        ((QVector3D*)((unsigned char*)contour + contour_idxs[q] * stripe ))->x() *
        ((QVector3D*)((unsigned char*)contour + contour_idxs[p] * stripe ))->y();
  }
  return A*0.5f;
}

   /*
     InsideTriangle decides if a point P is Inside of the triangle
     defined by A, B, C.
   */
bool Triangulate::InsideTriangle(float Ax, float Ay,
                      float Bx, float By,
                      float Cx, float Cy,
                      float Px, float Py)

{
  float ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
  float cCROSSap, bCROSScp, aCROSSbp;

  ax = Cx - Bx;  ay = Cy - By;
  bx = Ax - Cx;  by = Ay - Cy;
  cx = Bx - Ax;  cy = By - Ay;
  apx= Px - Ax;  apy= Py - Ay;
  bpx= Px - Bx;  bpy= Py - By;
  cpx= Px - Cx;  cpy= Py - Cy;

  aCROSSbp = ax*bpy - ay*bpx;
  cCROSSap = cx*apy - cy*apx;
  bCROSScp = bx*cpy - by*cpx;

  return (aCROSSbp > 0) && (bCROSScp > 0) && (cCROSSap > 0);
};

bool Triangulate::Snip(const QVector3D* contour,
                       unsigned short stripe,
                       const std::vector<unsigned short> &contour_idxs,
                       int u,int v,int w,int n,int *V)
{
  int p;
  float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

  Ax = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[u]] * stripe ))->x();
  Ay = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[u]] * stripe ))->y();

  Bx = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[v]] * stripe ))->x();
  By = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[v]] * stripe ))->y();

  Cx = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[w]] * stripe ))->x();
  Cy = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[w]] * stripe ))->y();

  if ( EPSILON > std::fabs(((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) {
      return false;
  }

  for (p=0;p<n;p++)
  {
    if( (p == u) || (p == v) || (p == w) ) continue;
    Px = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[p]] * stripe ))->x();
    Py = ((QVector3D*)((unsigned char*)contour + contour_idxs[V[p]] * stripe ))->y();
    if (InsideTriangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py)) {
        return false;
    }

  }

  return true;
}

bool Triangulate::Process(const QVector3D* contour,
                          unsigned short stripe,
                          const std::vector<unsigned short> &contour_idxs,
                          std::vector<unsigned short> &result)
{
  /* allocate and initialize list of Vertices in polygon */

  int n = contour_idxs.size();
  if ( n < 3 ) return false;

  int *V = new int[n];

  /* we want a counter-clockwise polygon in V */

  if ( 0.0f < Area(contour, stripe, contour_idxs) )
    for (int v=0; v<n; v++) V[v] = v;
  else
    for(int v=0; v<n; v++) V[v] = (n-1)-v;

  int nv = n;

  /*  remove nv-2 Vertices, creating 1 triangle every time */
  int count = 2*nv;   /* error detection */

  for(int m=0, v=nv-1; nv>2; )
  {
    /* if we loop, it is probably a non-simple polygon */
    if (0 >= (count--))
    {
      //** Triangulate: ERROR - probable bad polygon!
      return false;
    }

    /* three consecutive vertices in current polygon, <u,v,w> */
    int u = v  ; if (nv <= u) u = 0;     /* previous */
    v = u+1; if (nv <= v) v = 0;     /* new v    */
    int w = v+1; if (nv <= w) w = 0;     /* next     */

    if ( Snip(contour,stripe, contour_idxs,u,v,w,nv,V) )
    {
      int a,b,c,s,t;

      /* true names of the vertices */
      a = V[u]; b = V[v]; c = V[w];

      /* output Triangle */
      /*result.push_back( contour[a] );
      result.push_back( contour[b] );
      result.push_back( contour[c] );*/

      result.push_back( contour_idxs[a] );
      result.push_back( contour_idxs[b] );
      result.push_back( contour_idxs[c] );

      m++;

      /* remove v from remaining polygon */
      for(s=v,t=v+1;t<nv;s++,t++) V[s] = V[t]; nv--;

      /* resest error detection counter */
      count = 2*nv;
    }
  }



  delete V;

  return true;
}

