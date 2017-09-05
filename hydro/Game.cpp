#include "Game.h"

#include "DebugPanel.h"
#include "WorldView.h"
#include "WorldController.h"
#include "GlobalParams.h"
#ifdef MEASURES
#include "Ticks.h"
#endif
Game::Game() : QObject(nullptr),
               _world_view(nullptr) {
}

Game::~Game() {

}

void Game::initialize() {
#ifdef MEASURES
    _measures = std::make_shared<MeasureStats>("DEFAULT MEASURES", 1.0);
    _measures->init();
    _measures->createTimer(MEASURE_EMPTY, "MEASURE_EMPTY");
    _measures->createTimer(MEASURE_ADD_SNIPPET_0, "MEASURE_ADD_SNIPPET_0");
    _measures->createTimer(MEASURE_ADD_SNIPPET_1, "MEASURE_ADD_SNIPPET_1");
    _measures->createTimer(MEASURE_GROW_NORMAL_0, "MEASURE_GROW_NORMAL_0");
    _measures->createTimer(MEASURE_GROW_NORMAL_1, "MEASURE_GROW_NORMAL_1");
    _measures->createTimer(MEASURE_GROW_NORMAL_2, "MEASURE_GROW_NORMAL_2");
    _measures->createTimer(MEASURE_GROW_NORMALS, "MEASURE_GROW_NORMALS");
    _measures->createTimer(MEASURE_UPDATE_RADIAL, "MEASURE_UPDATE_RADIAL");
    _measures->createTimer(MEASURE_UPDATE_TANGEN, "MEASURE_UPDATE_TANGEN");
    _measures->createTimer(MEASURE_UPDATE_TANGEN_0, "MEASURE_UPDATE_TANGEN_0");
    _measures->createTimer(MEASURE_UPDATE_TANGEN_1, "MEASURE_UPDATE_TANGEN_1");
    _measures->createTimer(MEASURE_UPDATE_TANGEN_2, "MEASURE_UPDATE_TANGEN_2");
    _measures->createTimer(MEASURE_UPDATE_TANGEN_3, "MEASURE_UPDATE_TANGEN_3");
    _measures->createTimer(MEASURE_UPDATE_TANGEN_4, "MEASURE_UPDATE_TANGEN_4");
    _measures->createTimer(MEASURE_GENERATE_GEOMETRY, "MEASURE_GENERATE_GEOMETRY");
    _measures->createTimer(MEASURE_UPDATE_WORLD_VIEW, "MEASURE_UPDATE_WORLD_VIEW");
    _measures->createTimer(MEASURE_UPDATE_CONTROLLER, "MEASURE_UPDATE_CONTROLLER");
    _measures->createTimer(MEASURE_BEZIER_POINTS, "MEASURE_BEZIER_POINTS");
    _measures->createTimer(MEASURE_FILL_SECTORS, "MEASURE_FILL_SECTORS");
#endif

    _world_view = new WorldView();

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    //format.setSamples(4);
    format.setSwapInterval(1);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    _world_view->setFormat(format);

    _world_view->setFixedSize( QSize(WORLD_FIXED_WIDTH, WORLD_FIXED_HEIGHT) );
    _world_view->initialize();
    _world_view->show();


    b2Vec2 gravity(0.0f, 0.0f);

    _world_controller = std::make_shared<WorldController>(gravity, _world_view->width() / WORLD_SCALE, _world_view->height() / WORLD_SCALE);
    _world_controller->createBoundaries();
    _world_controller->createObstacles();

    /*DebugPanel debug_panel;

    debug_panel.setFixedSize( QSize(400, 400) );
    debug_panel.show();*/
    _timer_update.setInterval(0);

    _timer_nano.start();
    _timer_update.start();

    connect(&_timer_update, SIGNAL(timeout()), this, SLOT(update()));
}

void Game::update() {
    START_TIMER(MEASURE_UPDATE_CONTROLLER);
    _world_controller->update();
    PAUSE_TIMER(MEASURE_UPDATE_CONTROLLER);
    START_TIMER(MEASURE_UPDATE_WORLD_VIEW);
    _world_view->update();
    PAUSE_TIMER(MEASURE_UPDATE_WORLD_VIEW);
#ifdef MEASURES
    _measures->onGameUpdate(getTimerNano().elapsed() / 1000.0);
#endif
    PRINT_TIMER(MEASURE_GROW_NORMALS);
    PRINT_TIMER(MEASURE_UPDATE_RADIAL);
    PRINT_TIMER(MEASURE_UPDATE_TANGEN);

    PRINT_TIMER(MEASURE_GENERATE_GEOMETRY);
    PRINT_TIMER(MEASURE_UPDATE_WORLD_VIEW);
    PRINT_TIMER(MEASURE_UPDATE_CONTROLLER);
    PRINT_TIMER(MEASURE_BEZIER_POINTS);
    PRINT_TIMER(MEASURE_FILL_SECTORS);

}
