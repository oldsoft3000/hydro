#include "CommonUtils.h"

void bvec2qvec(const b2Vec2& vec1, QVector2D& vec2) {
    vec2.setX(vec1.x);
    vec2.setY(vec1.y);
}

void qvec2bvec(const QVector2D& vec1, b2Vec2& vec2) {
    vec2.Set(vec1.x(), vec1.y());
}

void qvec2bvec(const QVector4D& vec1, b2Vec2& vec2) {
    vec2.Set(vec1.x(), vec1.y());
}
