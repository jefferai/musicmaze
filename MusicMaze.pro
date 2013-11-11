# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

CONFIG += c++11

TEMPLATE += app
QT += quick widgets multimedia

SOURCES += main.cpp \
    TrackHandler.cpp

HEADERS += \
    TrackHandler.h

OTHER_FILES += \
    qml/MusicMaze/main.qml \
    qml/MusicMaze/TileRect.qml

RESOURCES += \
    resources.qrc

