#ifndef FIXTUREPAINTERGL_H
#define FIXTUREPAINTERGL_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

class FixturePainterGl : protected QOpenGLFunctions
{
public:
    FixturePainterGl();
};

#endif // FIXTUREPAINTERGL_H
