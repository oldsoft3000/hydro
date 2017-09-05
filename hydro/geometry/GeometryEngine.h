#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QGLFramebufferObject>
#include <memory>

class GeometryEngine : protected QOpenGLFunctions
{
public:
    struct __attribute__((__packed__)) vertex_t {
        vertex_t(const QVector3D& v = QVector3D(),
                 const QVector4D& c = QVector4D(),
                 const QVector3D& n = QVector3D(),
                 const QVector2D& t = QVector2D()) : vertex(v),
                                                     color(c),
                                                     normal(n),
                                                     texcoord(t)

        {
        }

        QVector3D   vertex;
        QVector4D   color;
        QVector3D   normal;
        QVector2D   texcoord;

    };

typedef std::vector<vertex_t>   vertex_data_t;
typedef std::vector<GLushort>   index_data_t;

public:
    GeometryEngine();
    ~GeometryEngine();

    void drawPolyLine( QOpenGLShaderProgram *program, const vertex_data_t& vertex_data );
    void drawTriangles( QOpenGLShaderProgram *program, const vertex_data_t& vertex_data, const index_data_t& index_data );
    void drawTriangles( QOpenGLShaderProgram *program, const vertex_data_t& vertex_data );
private:
    void drawFBO();
private:
    QOpenGLBuffer   _verticies;
    QOpenGLBuffer   _indicies;

    std::shared_ptr<QGLFramebufferObject>    _fbo;
    QOpenGLShaderProgram    _program;
};


#endif // GEOMETRYENGINE_H
