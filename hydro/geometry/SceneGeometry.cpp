#include "SceneGeometry.h"
#include "GlobalParams.h"

SceneGeometry::SceneGeometry(unsigned int num_col,
                             unsigned int num_row) : _num_col(num_col),
                                                     _num_row(num_row)
{
    _size_cell = 100 / WORLD_SCALE;

    generateSceneGeometry();
}


void SceneGeometry::generateSceneGeometry() {

    unsigned int idx_rect = 0;

    for (unsigned int iy = 0; iy != _num_row + 1; ++iy) {

        for (unsigned int ix = 0; ix != _num_col + 1; ++ix) {
            unsigned int idx = iy * (_num_row  + 1)+ ix;

            vertex_t vertex;

            _verticies.push_back(vertex_t( QVector3D(ix * _size_cell, iy * _size_cell, 0), QVector4D(0.0, 0.0, 0.0, 0.2) ) );

            if ( iy == _num_row || ix == _num_col ) {
                continue;
            }

            if ( idx % 2 == 0 ) {
                _indices.push_back(idx + _num_row + 1);
                _indices.push_back(idx);
                _indices.push_back(idx + 1);
                _indices.push_back(idx + _num_row + 1);
                _indices.push_back(idx + 1);
                _indices.push_back(idx + _num_row + 2);
            }
        }
    }

}

void SceneGeometry::drawScene(QOpenGLShaderProgram *program) {
    //glPolygonMode(GL_BACK, GL_FILL);

    drawTriangles( program, _verticies, _indices );
}
