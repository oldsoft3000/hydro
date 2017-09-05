#include "Interpolator.h"
#include <QDebug>

double Interpolator::calcLinear(double x) {
    double k = (_y1 - _y0) / (_x1 - _x0);
    double y = k * (x - _x0) + _y0;
    return y;
}

double Interpolator::calcQuad(double x) {
    double k = (_y1 - _y0) / ((_x1 - _x0) * (_x1 - _x0));
    double y = k * (x - _x0) * (x - _x0) + _y0;
    return y;
}

double Interpolator::calc(double x, QEasingCurve::Type easing_type) {
    QEasingCurve easing(easing_type);

    double t = (x - _x0) / (_x1 - _x0);

    double v = easing.valueForProgress(t);

    double y = v * (_y1 - _y0) + _y0;

    return y;
}



