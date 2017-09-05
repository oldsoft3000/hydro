#ifndef FIXTUREPAINTER_H
#define FIXTUREPAINTER_H

#include <QtOpenGL>
#include <Box2D/Box2D.h>

class FixturePainter
{
public:
    FixturePainter();

    static void paintFixture(QPainter *painter, b2Body*  body, const QColor& color);
    static void paintCircle(QPainter *painter, const QVector2D& point, double radius, const QColor& color);
private:
    static void paintFixtureCircle(QPainter *painter, b2Body*  body, const QColor& color);
    static void paintFixturePolygon(QPainter *painter, b2Body*  body, const QColor& color);
};

#endif // FIXTUREPAINTER_H
