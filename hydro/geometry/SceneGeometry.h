#ifndef SCENEGEOMETRY_H
#define SCENEGEOMETRY_H

#include "GeometryEngine.h"

class SceneGeometry :  public GeometryEngine {
public:
    SceneGeometry(unsigned int num_col,
                  unsigned int num_row);
    void drawScene(QOpenGLShaderProgram *program);
private:
    void generateSceneGeometry();
private:
    unsigned int    _num_col;
    unsigned int    _num_row;
    float           _size_cell;

    vertex_data_t       _verticies;
    index_data_t        _indices;
};

#endif // SCENEGEOMETRY_H
