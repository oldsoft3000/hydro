#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include <QEasingCurve>

class Interpolator
{
    double _x0, _x1, _y0, _y1;
public:
    Interpolator(double x0,
                 double x1,
                 double y0,
                 double y1) : _x0(x0),
                              _x1(x1),
                              _y0(y0),
                              _y1(y1)  {}
    double calcLinear(double x);
    double calcQuad(double x);
    double calc(double x, QEasingCurve::Type easing_type);
};

#endif // INTERPOLATOR_H
