#include "ColorGenerator.h"

ColorGenerator::ColorGenerator() : _cc(0)
{

}



QVector4D ColorGenerator::getColor() {
    QVector4D color;

    _cc++;

    if ( _cc == 0 ) {
        color = QVector4D(1.0, 0.0, 0.0, 1.0);
    } else if ( _cc == 1 ) {
        color = QVector4D(0.0, 1.0, 0.0, 1.0);
    } else if ( _cc == 2 ) {
        color = QVector4D(0.0, 0.0, 1.0, 1.0);
    } else if ( _cc == 3 ) {
        color = QVector4D(0.0, 0.0, 0.0, 1.0);
    } else if ( _cc == 4 ) {
        color = QVector4D(0.0, 1.0, 1.0, 1.0);
        _cc = -1;
    }

    color = QVector4D(0.5, 0.5, 0.5, 1.0);

    return color;
}
