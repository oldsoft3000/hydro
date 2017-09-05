#include "MathUtils.h"


bool CentripetalAcceleration::getCircleByPoints(const QVector2D& point_1,
                            const QVector2D& point_2,
                            const QVector2D& point_3,
                            QVector2D& center) {

    if (point_2 == point_1 || point_3 == point_2) {
        return false;
    }

    double m_1 = (point_2.y() - point_1.y()) / (point_2.x() - point_1.x());
    double m_2 = (point_3.y() - point_2.y()) / (point_3.x() - point_2.x());

    double x = (m_1 * m_2 * (point_1.y() - point_3.y()) + m_2 * (point_1.x() + point_2.x()) - m_1 * (point_2.x() + point_3.x())) / (2 * (m_2 - m_1));
    double y = (-1 / m_1) * (x - (point_1.x() + point_2.x()) / 2) + (point_1.y() + point_2.y()) / 2;

    center.setX(x);
    center.setY(y);

    return true;
}

bool CentripetalAcceleration::update(const QVector2D& point) {
    if (_trace.empty()) {
        _trace.push_back(point);
    } else {
        QVector2D vec_points = _trace.back() - point;
        const double distance_points = 1;

        if (vec_points.lengthSquared() > distance_points) {
            _trace.push_back(point);
        }
    }

    //_trace.push_back(point);

    if (_trace.size() < 3) {
        return false;
    } else {
        if (_trace.size() > 3) {
            _trace.pop_front();
        }

        QVector2D center;

        _last_center = _center;

        if (getCircleByPoints(_trace[0], _trace[1], _trace[2], center)) {
            _center = center;
            return true;
        } else {
            _last_center = _center = _trace.back();
            return false;
        }

        //qDebug() << "LastCenter " << _center << " " << _trace[0] << " " << _trace[1] << " " << _trace[2];
    }
}

bool CentripetalAcceleration::getCenter(QVector2D& center) const {
    if (isValidVector(_center) && _trace.size() >= 3) {
        center = _center;
        return true;
    } else {
        return false;
    }
}

bool CentripetalAcceleration::getLastCenter(QVector2D& center) const {
    if (isValidVector(_last_center) && _trace.size() >= 3) {
        center = _last_center;
        return true;
    } else {
        return false;
    }
}

double CentripetalAcceleration::calcCentripetalAcceleration(double speed, const QVector2D& point_center, const QVector2D& point_body) {
    double radius = point_center.distanceToPoint(point_body);
    double W = speed / radius;
    const double koef = 0.01;
    double force = W * W * radius * koef;

    return  force;
}
