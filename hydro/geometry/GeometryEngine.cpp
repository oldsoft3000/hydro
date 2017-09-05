#include "GeometryEngine.h"

GeometryEngine::GeometryEngine() : _indicies(QOpenGLBuffer::IndexBuffer)
{
    initializeOpenGLFunctions();

    _verticies.create();
    _indicies.create();

    _verticies.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _indicies.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    if (!_program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/vshader_passthrough.glsl")) {
        qDebug() << _program.log();
    }
    if (!_program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fshader_simpletexture.glsl")) {
        qDebug() << _program.log();
    }
    if (!_program.link()) {
        qDebug() << _program.log();
    }

    _fbo = std::make_shared<QGLFramebufferObject>( QSize(1024, 1024) );
}

GeometryEngine::~GeometryEngine()
{
    _verticies.destroy();
    _indicies.destroy();
}

void GeometryEngine::drawPolyLine(QOpenGLShaderProgram *program, const vertex_data_t& vertex_data) {
    program->bind();
    _verticies.bind();
    _verticies.allocate(vertex_data.data(), vertex_data.size() * sizeof(vertex_t));

    int vertex_Location = program->attributeLocation("vertex");
    program->enableAttributeArray(vertex_Location);
    program->setAttributeBuffer(vertex_Location, GL_FLOAT, offsetof(vertex_t, vertex), 3, sizeof(vertex_t));

    int color_location = program->attributeLocation("color");
    program->enableAttributeArray(color_location);
    program->setAttributeBuffer(color_location, GL_FLOAT, offsetof(vertex_t, color), 4, sizeof(vertex_t));

    glDrawArrays(GL_LINES, 0, vertex_data.size());

    program->release();
    _verticies.release();
}

void GeometryEngine::drawTriangles( QOpenGLShaderProgram *program, const vertex_data_t& vertex_data, const index_data_t& index_data ) {
    //_fbo->bind();

    glEnable( GL_BLEND );
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

     program->bind();
    _verticies.bind();
    _verticies.allocate( vertex_data.data(), vertex_data.size() * sizeof(vertex_t) );

    int vertex_location = program->attributeLocation("vertex");
    program->enableAttributeArray(vertex_location);
    program->setAttributeBuffer(vertex_location, GL_FLOAT, offsetof(vertex_t, vertex), 3, sizeof(vertex_t));

    int normal_location = program->attributeLocation("normal");
    program->enableAttributeArray(normal_location);
    program->setAttributeBuffer(normal_location, GL_FLOAT, offsetof(vertex_t, normal), 3, sizeof(vertex_t));

    int color_location = program->attributeLocation("color");
    program->enableAttributeArray(color_location);
    program->setAttributeBuffer(color_location, GL_FLOAT, offsetof(vertex_t, color), 4, sizeof(vertex_t));

    _indicies.bind();
    _indicies.allocate( index_data.data(), index_data.size() * sizeof(GLushort) );

    glDrawElements( GL_TRIANGLES, index_data.size(), GL_UNSIGNED_SHORT, 0 );

    program->release();
    _verticies.release();
    _indicies.release();

    //_fbo->release();
    //drawFBO();
}


void GeometryEngine::drawTriangles( QOpenGLShaderProgram *program, const vertex_data_t& vertex_data ) {
    _fbo->bind();

    program->bind();
    _verticies.bind();
    _verticies.allocate( vertex_data.data(), vertex_data.size() * sizeof(vertex_t) );

    int vertex_location = program->attributeLocation("vertex");
    program->enableAttributeArray(vertex_location);
    program->setAttributeBuffer(vertex_location, GL_FLOAT, offsetof(vertex_t, vertex), 3, sizeof(vertex_t));

    int normal_location = program->attributeLocation("normal");
    program->enableAttributeArray(normal_location);
    program->setAttributeBuffer(normal_location, GL_FLOAT, offsetof(vertex_t, normal), 3, sizeof(vertex_t));

    int color_location = program->attributeLocation("color");
    program->enableAttributeArray(color_location);
    program->setAttributeBuffer(color_location, GL_FLOAT, offsetof(vertex_t, color), 4, sizeof(vertex_t));

    glDrawArrays( GL_TRIANGLES, 0, vertex_data.size() );


    program->release();
    _verticies.release();
    _indicies.release();
    _fbo->release();
}


void GeometryEngine::drawFBO() {

    glBindTexture( GL_TEXTURE_2D, _fbo->texture() );
    _program.setUniformValue("renderedTexture", 0);

    vertex_data_t data;

    data.push_back(vertex_t(QVector3D( 0, 0, 0 )));
    data.push_back(vertex_t(QVector3D( 1, 0, 0 )));
    data.push_back(vertex_t(QVector3D( 1, 1, 0 )));

    _program.bind();
    _verticies.bind();
    _verticies.allocate( data.data(), data.size() * sizeof(vertex_t) );

    int vertex_location = _program.attributeLocation("vertex");
    _program.enableAttributeArray(vertex_location);
    _program.setAttributeBuffer(vertex_location, GL_FLOAT, offsetof(vertex_t, vertex), 3, sizeof(vertex_t));

    glDrawArrays( GL_TRIANGLES, 0, data.size() );

    _program.release();
    _verticies.release();

}
