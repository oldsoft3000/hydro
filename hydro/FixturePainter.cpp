#include "FixturePainter.h"
#include "GlobalParams.h"

FixturePainter::FixturePainter()
{
}

void FixturePainter::paintFixture(QPainter *painter, b2Body* body, const QColor& color) {
    b2Fixture* cur_fixture = body->GetFixtureList();
    while (cur_fixture != NULL) {
        switch  ( cur_fixture->GetShape()->GetType() ) {
        case b2Shape::e_circle:
            paintFixtureCircle(painter, body, color);
            break;
        case b2Shape::e_polygon:
            paintFixturePolygon(painter, body, color);
            break;
        default:
            break;
        }
        cur_fixture = cur_fixture->GetNext();
    }

}

void FixturePainter::paintCircle(QPainter *painter, const QVector2D& point, double radius, const QColor& color) {
    //QPen pen;
    //pen.setWidth(1);
    //pen.setColor(color);

    painter->setBrush( QBrush( color ) );
    painter->setPen( Qt::NoPen );

    float32 x = point.x() * WORLD_SCALE;
    float32 y = point.y() * WORLD_SCALE;
    float32 r = radius * WORLD_SCALE;
    painter->drawEllipse(QPointF(x, y), r, r);
}

void FixturePainter::paintFixtureCircle(QPainter *painter, b2Body* body, const QColor& color) {
    //QPen pen;
    //pen.setWidth(1);
    //pen.setColor(color);

    painter->setBrush( QBrush( color ) );
    painter->setPen( Qt::NoPen );

    float32 x = body->GetPosition().x * WORLD_SCALE;
    float32 y = body->GetPosition().y * WORLD_SCALE;
    float32 r = body->GetFixtureList()->GetShape()->m_radius * WORLD_SCALE;
    painter->drawEllipse(QPointF(x, y), r, r);
}

void FixturePainter::paintFixturePolygon(QPainter *painter, b2Body* body, const QColor& color) {
    QPen pen;
    pen.setWidth(1);
    pen.setColor(color);

    painter->setPen(pen);

    float32 x = body->GetPosition().x * WORLD_SCALE;
    float32 y = body->GetPosition().y * WORLD_SCALE;
    float32 angle = body->GetAngle();
    const b2PolygonShape *shape = static_cast<b2PolygonShape*>(body->GetFixtureList()->GetShape());
    float32 hx = shape->GetVertex(1).x * WORLD_SCALE;
    float32 hy = shape->GetVertex(2).y * WORLD_SCALE;
    QRectF r(x-hx, y-hy, 2*hx, 2*hy);
    painter->save();
    painter->translate(r.center());
    painter->rotate(angle*180/b2_pi);
    painter->translate(-r.center());
    painter->drawRect(r);
    painter->restore();
}
