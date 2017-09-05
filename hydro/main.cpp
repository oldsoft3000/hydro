#include <QApplication>

#include "Game.h"


int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    Game::instance().initialize();

    return app.exec();
}

