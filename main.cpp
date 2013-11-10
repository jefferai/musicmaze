// This work is copyright 2013 by Jeff Mitchell <jeff@jefferai.org>
// This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

#include <QtGui/QGuiApplication>

#include "TrackHandler.h"

#include <QQmlEngine>
#include <QQuickView>
#include <QQmlContext>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickView* view = new QQuickView();

    qmlRegisterType< Tile >("MusicMaze", 1, 0, "Tile");
    qmlRegisterType< TrackHandler >("MusicMaze", 1, 0, "TrackHandler");

    view->rootContext()->setContextProperty("mainWindow", view);
    view->setSource(QUrl::fromLocalFile("/Users/jemitche/src/MusicMaze/MusicMaze/qml/MusicMaze/main.qml"));
    view->show();

    return app.exec();
}
