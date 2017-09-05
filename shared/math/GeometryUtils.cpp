#include "GeometryUtils.h"

float calcAngle(float x, float y) {
    float result = 0;
    if (y == 0) {
        if (x >= 0) {
            result = 0;
        } else {
            result = M_PI;
        }
    } else if (x == 0) {
        if (y == 0) {
            result = 0;
        } else if (y > 0) {
            result = M_PI / 2.0;
        } else {
            result = 3 * M_PI / 2.0;
        }
    } else {
        result = atan(y / x);
        if ((x < 0 && y > 0) ||
            (x < 0 && y < 0)) {
            result = result + M_PI;
        } else if (x > 0 && y < 0) {
            result = result + 2 * M_PI;
        }
    }
    return result;
}

float calcDiamondAngle( float x, float y ) {
    if ( y >= 0 ) {
        return ( x >= 0 ? y / ( x + y ) : 1 - x / ( -x + y ) );
    } else {
        return ( x < 0 ? 2 - y / ( -x - y ) : 3 + x / ( x- y ) );
    }
}

float calcAngle(const b2Vec2& v1, const b2Vec2& v2) {
    float32 cosa = b2Dot(v1, v2) / (v1.Length() * v2.Length());
    return acos(cosa);
}

bool isPointAtSnippet(const QVector3D& point_1,
                      const QVector3D& point_2,
                      const QVector3D& point_3,
                      float& koef) {
    const float eps = std::numeric_limits<float>::epsilon();

    float koef_x;
    float koef_y;

    float X = point_2.x() - point_1.x();
    float Y = point_2.y() - point_1.y();

    if (X == 0 && Y == 0) {
        return false;
    }

    if (X == 0) {
        koef_x = (point_3.y() - point_1.y()) / Y;
    } else {
        koef_x = (point_3.x() - point_1.x()) / X;
    }

    if (Y == 0) {
        koef_y = (point_3.x() - point_1.x()) / X;
    } else {
        koef_y = (point_3.y() - point_1.y()) / Y;
    }

    if (std::abs(koef_x - koef_y) < eps) {
        koef = koef_x;
        return true;
    } else {
        return false;
    }
}

bool isSnippetsIntersected(const QVector3D& point_1,
                           const QVector3D& point_2,
                           const QVector3D& point_3,
                           const QVector3D& point_4) {

    QVector3D v_1 = QVector3D::crossProduct(point_4 - point_3, point_3 - point_1);
    QVector3D v_2 = QVector3D::crossProduct(point_4 - point_3, point_3 - point_2);
    QVector3D v_3 = QVector3D::crossProduct(point_2 - point_1, point_3 - point_1);
    QVector3D v_4 = QVector3D::crossProduct(point_2 - point_1, point_4 - point_1);

    if (v_1.z() * v_2.z() < 0 && v_3.z() * v_4.z() < 0) {
        return true;
    } else {
        return false;
    }

}

bool getSnippetsInstersection(const QVector3D& point_1,
                              const QVector3D& point_2,
                              const QVector3D& point_3,
                              const QVector3D& point_4,
                              QVector3D& point) {

    /*float u_2 = ( point_4.y() - point_3.y() ) * ( point_2.x() - point_1.x() ) - ( point_4.x() - point_3.x() ) * ( point_2.y() - point_1.y() );
    if (u_2 == 0) {
        return false;
    }
    float u_1 = ( point_4.x() - point_3.x() ) * ( point_1.y() - point_3.y() ) - ( point_4.y() - point_3.y() ) * ( point_1.x() - point_3.x() );


    float u = u_1 / u_2;

    float x = point_1.x() + u * ( point_2.x() - point_1.x() );
    float y = point_1.y() + u * ( point_2.y() - point_1.y() );

    float x1 = std::min(point_1.x(), point_2.x());
    float x2 = std::max(point_1.x(), point_2.x());
    float x3 = std::min(point_3.x(), point_4.x());
    float x4 = std::max(point_3.x(), point_4.x());
    float y1 = std::min(point_1.y(), point_2.y());
    float y2 = std::max(point_1.y(), point_2.y());
    float y3 = std::min(point_3.y(), point_4.y());
    float y4 = std::max(point_3.y(), point_4.y());

    if ( x >= x1 &&
         x <= x2 &&
         x >= x3 &&
         x <= x4 &&
         y >= y1 &&
         y <= y2 &&
         y >= y3 &&
         y <= y4 ) {

        point.setX(x);
        point.setY(y);

        return true;
    } else {
        return false;
    }*/

    QVector3D dir1 = point_2 - point_1;
    QVector3D dir2 = point_4 - point_3;

    //считаем уравнения прямых проходящих через отрезки
    float a1 = -dir1.y();
    float b1 = +dir1.x();
    float d1 = -(a1*point_1.x() + b1*point_1.y());

    float a2 = -dir2.y();
    float b2 = +dir2.x();
    float d2 = -(a2*point_3.x() + b2*point_3.y());

    //подставляем концы отрезков, для выяснения в каких полуплоскотях они
    float seg1_line2_start = a2*point_1.x() + b2*point_1.y() + d2;
    float seg1_line2_end = a2*point_2.x() + b2*point_2.y() + d2;

    float seg2_line1_start = a1*point_3.x() + b1*point_3.y() + d1;
    float seg2_line1_end = a1*point_4.x() + b1*point_4.y() + d1;

    //если концы одного отрезка имеют один знак, значит он в одной полуплоскости и пересечения нет.
    if (seg1_line2_start * seg1_line2_end >= 0 || seg2_line1_start * seg2_line1_end >= 0)
        return false;

    float u = seg1_line2_start / (seg1_line2_start - seg1_line2_end);
    point =  point_1 + u*dir1;

    return true;

};

bool isEqualPoints(const QVector3D* point_1, const QVector3D* point_2) {
    const float eps = std::numeric_limits<float>::epsilon();

    if (std::abs(point_1->x() - point_2->x()) < eps &&
        std::abs(point_1->y() - point_2->y()) < eps) {
        return true;
    } else {
        return false;
    }
}

bool isPointAfterSnippetEnd(const QVector3D& point_1,
                            const QVector3D& point_2,
                            const QVector3D& point_3) {
    const float eps = std::numeric_limits<float>::epsilon();

    float koef;

    if (isPointAtSnippet(point_1, point_2, point_3, koef)) {
        if (koef > 1.0 + eps) {
            return true;
        }
    }
    return false;
}

bool calcVerticalPoint(const QVector3D& A,
                       const QVector3D& B,
                       const QVector3D& C,
                       QVector3D& D,
                       float& koef) {
    QVector3D AB = B - A;
    QVector3D ABn = AB.normalized();
    QVector3D AC = C - A;

    float L = QVector3D::dotProduct(AC, ABn);

    QVector3D AD = L * ABn;
    D = A + AD;

    koef = L / AB.length();

    if (koef < 0 || koef > 1) {
        return false;
    } else {
        return true;
    }
}
