#ifndef SNIPPETAREA_H
#define SNIPPETAREA_H

#include <vector>
#include <QVector3D>

class OutlineNormal;

class SnippetArea {
public:
    struct Cell {
        std::vector<unsigned int>   data_idxs;
        unsigned int                frame_counter;
        bool                        marked;

        Cell() { frame_counter = 0; marked = false; data_idxs.reserve(16); }
    };

public:
    SnippetArea(int num_x, int num_y, float width_cell);

    void initialize();
    void addSnippet(unsigned int frame_counter,
                    unsigned int idx_data,
                    std::vector<unsigned int>& cell_idxs,
                    const QVector3D& vertex_0,
                    const QVector3D& vertex_1);
    void setTop(float top);
    void setLeft(float left);
    Cell* getCell(unsigned int idx);
    Cell* getCell(float x, float y);
    int coordsToIdx(float x, float y);
    void idxToCoords(int idx, int& idx_x, int& idx_y);
    bool getSnippetIdxs(QVector3D vertex_0,
                        QVector3D vertex_1,
                        std::vector<unsigned int>& cell_idxs);
    void visualize(std::vector<QVector3D>& vertex_buffer, unsigned int frame_counter);
    void clear();
private:
    float   _top;
    float   _left;
    int     _num_x;
    int     _num_y;
    float   _width_cell;

    std::vector<Cell>           _cells;
};

#endif // SNIPPETAREA_H
