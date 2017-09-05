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
    SnippetArea(int num_x, int num_y, float width_cell) {
        _top = 0;
        _left = 0;
        _num_x = num_x;
        _num_y = num_y;
        _width_cell = width_cell;
    }

    void initialize() {
        _cells.resize(_num_x * _num_y);
    }

    void setTop(float top) {
        _top = top;
    }

    void setLeft(float left) {
        _left = left;
    }

    Cell* getCell(unsigned int idx) {
        if (idx < _cells.size()) {
            return &_cells[idx];
        } else {
            return nullptr;
        }
    }

    Cell* getCell(float x, float y) {
        unsigned int idx_x = (x - _left) / _width_cell;
        unsigned int idx_y = (y - _top) / _width_cell;
        unsigned int idx = idx_y *_num_x + idx_x;

        if (idx < _cells.size()) {
            return &_cells[idx];
        } else {
            return nullptr;
        }
    }

    int coordsToIdx(float x, float y) {
        unsigned int idx_x = (x - _left) / _width_cell;
        unsigned int idx_y = (y - _top) / _width_cell;
        unsigned int idx = idx_y *_num_x + idx_x;

        if (idx >= _cells.size()) {
            return -1;
        } else {
            return idx;
        }
    }

    void idxToCoords(int idx, int& idx_x, int& idx_y) {
        idx_y = idx / _num_x;
        idx_x = idx - idx_y * _num_x;
    }


    void addSnippet(unsigned int frame_counter,
                    unsigned int idx_data,
                    std::vector<unsigned int>& buffer_cell_idxs,
                    const QVector3D& vertex_0,
                    const QVector3D& vertex_1);

    bool getSnippetIdxs(QVector3D vertex_0,
                        QVector3D vertex_1,
                        std::vector<unsigned int>& cell_idxs) {

        //cell_idxs.clear();

        int idx_x_0 = (vertex_0.x() - _left) / _width_cell;
        int idx_y_0 = (vertex_0.y() - _top) / _width_cell;

        int idx_x_1 = (vertex_1.x() - _left) / _width_cell;
        int idx_y_1 = (vertex_1.y() - _top) / _width_cell;

        /*if (idx_x_0 < 0 || idx_x_0 >= _num_x ||
            idx_y_0 < 0 || idx_y_0 >= _num_y ||
            idx_x_1 < 0 || idx_x_1 >= _num_x ||
            idx_y_1 < 0 || idx_y_1 >= _num_y) {
            return false;
        }*/

        if (idx_x_0 < 0) {
            idx_x_0 = 0;
        } else if (idx_x_0 >= _num_x) {
            idx_x_0 = _num_x - 1;
        }

        if (idx_y_0 < 0) {
            idx_y_0 = 0;
        } else if (idx_y_0 >= _num_y) {
            idx_y_0 = _num_y - 1;
        }

        if (idx_x_1 < 0) {
            idx_x_1 = 0;
        } else if (idx_x_1 >= _num_x) {
            idx_x_1 = _num_x - 1;
        }

        if (idx_y_1 < 0) {
            idx_y_1 = 0;
        } else if (idx_y_1 >= _num_y) {
            idx_y_1 = _num_y - 1;
        }

        if (idx_x_0 == idx_x_1 && idx_y_0 == idx_y_1) {
            cell_idxs.push_back(idx_y_0 *_num_x + idx_x_0);
        }

        const float eps = 0.000001;

        int dx, dxx;
        int dy, dyy;

        if (idx_x_0 <= idx_x_1) {
            dx = 1;
            dxx = 1;
        } else {
            dx = -1;
            dxx = 0;
        }

        if (idx_y_0 <= idx_y_1) {
            dy = 1;
            dyy  = 1;
        } else {
            dy = -1;
            dyy = 0;
        }

        if (abs(idx_x_0 - idx_x_1) > abs(idx_y_0 - idx_y_1)) {
            int idx_straight_x_0 = idx_x_0;
            int idx_y = idx_y_0;

            if (idx_y_0 != idx_y_1) {
                for (; idx_y != idx_y_1 + dy; idx_y += dy) {
                    float y = _top + (idx_y + dyy) * _width_cell + eps * dy;
                    float x = (y - vertex_0.y()) * (vertex_1.x() - vertex_0.x()) / (vertex_1.y() - vertex_0.y()) + vertex_0.x();

                    int idx_x = (x - _left) / _width_cell;

                    if (idx_x != idx_straight_x_0) {
                        int idx = idx_straight_x_0;
                        for (; idx != idx_x + dx && idx != idx_x_1 + dx; idx += dx) {
                            if (idx >= 0) {
                                cell_idxs.push_back(idx_y *_num_x + idx);
                            }
                        }
                        idx_straight_x_0 = idx - dx;
                    } else {
                        cell_idxs.push_back(idx_y *_num_x + idx_x);
                    }

                }
            }

            if (idx_straight_x_0 != idx_x_1) {
                for (int idx = idx_straight_x_0; idx != idx_x_1 + dx; idx += dx) {
                    cell_idxs.push_back(idx_y_1 *_num_x + idx);
                }
            }

        } else {
            int idx_straight_y_0 = idx_y_0;
            int idx_x = idx_x_0;

            if (idx_x_0 != idx_x_1) {
                for (; idx_x != idx_x_1 + dx; idx_x += dx) {
                    float x = _left + (idx_x + dxx) * _width_cell + eps * dx;
                    float y = (x - vertex_0.x()) * (vertex_1.y() - vertex_0.y()) / (vertex_1.x() - vertex_0.x()) + vertex_0.y();

                    int idx_y = (y - _top) / _width_cell;

                    if (idx_y != idx_straight_y_0) {
                        int idx = idx_straight_y_0;
                        for (; idx != idx_y + dy && idx != idx_y_1 + dy; idx += dy) {
                            if (idx >= 0) {
                                cell_idxs.push_back(idx *_num_x + idx_x);
                            }
                        }
                        idx_straight_y_0 = idx - dy;
                    } else {
                        cell_idxs.push_back(idx_y *_num_x + idx_x);
                    }
                }
            }

            if (idx_straight_y_0 != idx_y_1) {
                for (int idx = idx_straight_y_0; idx != idx_y_1 + dy; idx += dy) {
                    cell_idxs.push_back(idx *_num_x + idx_x_1);
                }
            }
        }

        //Q_ASSERT(&_cells[cell_idxs.back()] == &_cells[idx_y_1 *_num_x + idx_x_1]);
        //Q_ASSERT(&_cells[cell_idxs.front()] == &_cells[idx_y_0 *_num_x + idx_x_0]);

        return cell_idxs.empty() == false;
    }


    void visualize(std::vector<QVector3D>& vertex_buffer, unsigned int frame_counter) {
        for (std::vector<Cell>::iterator i = _cells.begin(); i != _cells.end(); ++i) {
            if ( ( i->marked  ) && i->frame_counter == frame_counter) {
            //if ( ( i->marked  ) ) {
                int idx = i - _cells.begin();
                int idx_y = idx / _num_x;
                int idx_x = idx - idx_y * _num_x;

                vertex_buffer.push_back(QVector3D(_left + idx_x * _width_cell,
                                                  _top + idx_y * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + (idx_x + 1) * _width_cell,
                                                  _top + idx_y * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + (idx_x + 1) * _width_cell,
                                                  _top + idx_y * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + (idx_x + 1) * _width_cell,
                                                  _top + (idx_y + 1) * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + (idx_x + 1) * _width_cell,
                                                  _top + (idx_y + 1) * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + idx_x * _width_cell,
                                                  _top + (idx_y + 1) * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + idx_x * _width_cell,
                                                  _top + (idx_y + 1) * _width_cell,
                                                  0));
                vertex_buffer.push_back(QVector3D(_left + idx_x * _width_cell,
                                                  _top + idx_y * _width_cell,
                                                  0));

            }

        }


        vertex_buffer.push_back(QVector3D(_left,
                                          _top,
                                          0));
        vertex_buffer.push_back(QVector3D(_left + _num_x * _width_cell,
                                          _top,
                                          0));
        vertex_buffer.push_back(QVector3D(_left + _num_x * _width_cell,
                                          _top,
                                          0));
        vertex_buffer.push_back(QVector3D(_left + _num_x * _width_cell,
                                          _top + _num_y * _width_cell,
                                          0));
        vertex_buffer.push_back(QVector3D(_left + _num_x * _width_cell,
                                          _top + _num_y * _width_cell,
                                          0));
        vertex_buffer.push_back(QVector3D(_left,
                                          _top + _num_y * _width_cell,
                                          0));
        vertex_buffer.push_back(QVector3D(_left,
                                          _top + _num_y * _width_cell,
                                          0));
        vertex_buffer.push_back(QVector3D(_left,
                                          _top,
                                          0));

    }

    void clear() {
        for (std::vector<Cell>::iterator i = _cells.begin(); i != _cells.end(); ++i) {
            i->data_idxs.clear();
            i->marked = false;
        }
    }

private:
    float   _top;
    float   _left;
    int     _num_x;
    int     _num_y;
    float   _width_cell;

    std::vector<Cell>           _cells;
};

#endif // SNIPPETAREA_H
