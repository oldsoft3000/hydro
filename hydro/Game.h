#ifndef GAME_H
#define GAME_H

#include <memory>
#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

#define MEASURES
#ifdef MEASURES
    #include "MeasureStats.h"
#endif

#ifdef MEASURES
    #define START_TIMER(IDX_TIMER) Game::instance().getMeasures()->startTimer(IDX_TIMER);
#else
    #define START_TIMER(IDX_TIMER);
#endif

#ifdef MEASURES
    #define PAUSE_TIMER(IDX_TIMER) Game::instance().getMeasures()->pauseTimer(IDX_TIMER);
#else
    #define PAUSE_TIMER(IDX_TIMER);
#endif

#ifdef MEASURES
    #define PRINT_TIMER(IDX_TIMER) Game::instance().getMeasures()->addStat(IDX_TIMER);
#else
    #define PRINT_TIMER(IDX_TIMER);
#endif

class WorldView;
class WorldController;
typedef std::shared_ptr<WorldController> WorldControllerPtr;
#ifdef MEASURES
class MeasureStats;
typedef std::shared_ptr<MeasureStats> MeasureStatsPtr;
#endif

class Game : public QObject
{
    Q_OBJECT

private:
    Game();
    virtual ~Game();
private:
    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;
private:
    QElapsedTimer       _timer_nano;
    QTimer              _timer_update;
    WorldView*          _world_view;
    WorldControllerPtr  _world_controller;

#ifdef MEASURES
    MeasureStatsPtr     _measures;
#endif
public:
    static Game& instance()
    {
        {
            static Game s;
            return s;
        }
    }

    void initialize();

    const QElapsedTimer& getTimerNano() const { return _timer_nano; }

    WorldControllerPtr getWorldController() { return _world_controller; }
#ifdef MEASURES
    MeasureStatsPtr getMeasures() { return _measures; }
#endif
signals:

public slots:
    void update();
};

#endif // GAME_H
