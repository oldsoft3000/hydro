#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <cmath>
#include <Box2D/Box2D.h>
#include <QVector3D>

float calcAngle( float x, float y );
float calcDiamondAngle( float x, float y );
float calcAngle( const b2Vec2& v1, const b2Vec2& v2 );

bool getSnippetsInstersection(const QVector3D& point_1,
                              const QVector3D& point_2,
                              const QVector3D& point_3,
                              const QVector3D& point_4,
                              QVector3D& point);
bool isSnippetsIntersected(const QVector3D& point_1,
                           const QVector3D& point_2,
                           const QVector3D& point_3,
                           const QVector3D& point_4);

bool isPointAtSnippet(const QVector3D& point_1,
                      const QVector3D& point_2,
                      const QVector3D& point_3,
                      float& koef);

bool isEqualPoints(const QVector3D* point_1, const QVector3D* point_2);
bool isPointAfterSnippetEnd(const QVector3D& point_1,
                            const QVector3D& point_2,
                            const QVector3D& point_3);

bool calcVerticalPoint(const QVector3D& A,
                       const QVector3D& B,
                       const QVector3D& C,
                       QVector3D& D,
                       float& koef);

#endif // GEOMETRYUTILS_H
