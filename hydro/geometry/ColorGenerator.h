#ifndef COLORGENERATOR_H
#define COLORGENERATOR_H

#include <QVector4D>

class ColorGenerator
{
public:
    ColorGenerator();
    QVector4D getColor();
private:
    unsigned int _cc;
};

#endif // COLORGENERATOR_H
