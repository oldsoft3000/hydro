#ifndef FLUIDTRIANGLES_H
#define FLUIDTRIANGLES_H

#include "FluidGeometry.h"
#include <vector>
#include <functional>
#include <cmath>

class FluidGeometry;
typedef std::shared_ptr<FluidGeometry> FluidGeometryPtr;
class ColorGenerator;
typedef std::shared_ptr<ColorGenerator> ColorGeneratorPtr;

const float epsilon_compare = 0.0001;

class FluidTriangles
{
    typedef std::vector<unsigned int> edges_t;

    struct path_data_t {
        edges_t edges;
    };

    struct sector_t {
        sector_t( OutlineNormals::iterator i_r, OutlineNormals::iterator i_l) {
            inormal_r = i_r;
            inormal_l = i_l;
        }

        OutlineNormals::iterator inormal_r;
        OutlineNormals::iterator inormal_l;
    };

    typedef std::vector<sector_t> sectors_t;

    typedef std::vector<GLushort> contour_idxs_t;
    typedef std::vector<contour_idxs_t> contours_idxs_t;
    typedef std::pair<contour_idxs_t::iterator, contour_idxs_t::iterator> contour_iterators_t;
    typedef std::vector<contour_iterators_t> vector_contour_iterators_t;

public:
    FluidTriangles(FluidGeometry& fluid_geometry);

    void triangulate();
private:
    void processContour(contour_idxs_t& contour_idxs, const QColor& color);
    void processSector(int idx_row, const sector_t& sector);
    void processUnclosed();

    static void pushIdxToContour(contour_idxs_t& contour_idxs, GLushort idx);

    OutlineNormals::iterator findFirstNormal(unsigned int idx_row);
    OutlineNormals::iterator findNextSectors(unsigned int idx_row, OutlineNormals::iterator inormal, sectors_t& sectors);
    OutlineNormals::iterator findNearestNormal(unsigned int idx_row, OutlineNormals::iterator inormal_0, bool forward);
    int getOffsetFromEnd(int idx_row, OutlineNormals::iterator inormal);
    void addToContour(unsigned int idx_row, int idx_normal, contour_idxs_t& contour_idxs);

private:
    FluidGeometry&              _fg;
    std::vector<path_data_t>    _path_data;
    ColorGeneratorPtr           _color_generator;

};

#endif // FLUIDTRIANGLES_H
