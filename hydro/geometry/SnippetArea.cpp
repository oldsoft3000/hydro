#include "SnippetArea.h"
#include "FluidGeometry.h"
#define MEASURES
#ifdef MEASURES
#include "Ticks.h"
#endif
#include "Game.h"
#include "GlobalParams.h"

void SnippetArea::addSnippet(unsigned int frame_counter,
                unsigned int idx_data,
                std::vector<unsigned int>& cell_idxs,
                const QVector3D& vertex_0,
                const QVector3D& vertex_1) {
#ifdef MEASURES
//    Game::instance().getMeasures()->startTimer(MEASURE_ADD_SNIPPET_0);
#endif
    if (getSnippetIdxs(vertex_0, vertex_1, cell_idxs)) {
#ifdef MEASURES
//    Game::instance().getMeasures()->pauseTimer(MEASURE_ADD_SNIPPET_0);
#endif
#ifdef MEASURES
//    Game::instance().getMeasures()->startTimer(MEASURE_ADD_SNIPPET_1);
#endif
        for (std::vector<unsigned int>::iterator iidx_cell = cell_idxs.begin(); iidx_cell != cell_idxs.end(); ++iidx_cell) {
            if (*iidx_cell < _cells.size()) {
                Cell& cell = _cells[*iidx_cell];

                if (cell.frame_counter != frame_counter) {
                    cell.frame_counter = frame_counter;
                    cell.data_idxs.clear();
                }

                if (std::find(cell.data_idxs.begin(), cell.data_idxs.end(), idx_data) == cell.data_idxs.end()) {
                    cell.data_idxs.push_back(idx_data);
                }
            }
        }
#ifdef MEASURES
 //   Game::instance().getMeasures()->pauseTimer(MEASURE_ADD_SNIPPET_1);
#endif

    }
}
