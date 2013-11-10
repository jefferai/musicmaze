#ifndef TRACKHANDLER_H
#define TRACKHANDLER_H

#include <QObject>
#include <QUrl>
#include <QMediaPlayer>
#include <QQuickItem>
#include <QStack>
#include <QTimer>

struct Direction
{
    Direction()
        : wall(true)
        , visited(false)
    {}

    Direction& operator=(const Direction& other)
    {
        wall = other.wall;
        visited = other.visited;
        return *this;
    }

    bool wall;
    bool visited;
};

class Tile : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int id READ id WRITE setId NOTIFY idChanged)
    Q_PROPERTY(bool northWall READ northWall WRITE setNorthWall NOTIFY northWallChanged)
    Q_PROPERTY(bool southWall READ southWall WRITE setSouthWall NOTIFY southWallChanged)
    Q_PROPERTY(bool eastWall READ eastWall WRITE setEastWall NOTIFY eastWallChanged)
    Q_PROPERTY(bool westWall READ westWall WRITE setWestWall NOTIFY westWallChanged)
    Q_PROPERTY(bool partOfSolution READ partOfSolution WRITE setPartOfSolution NOTIFY partOfSolutionChanged)

public:
    Tile(QQuickItem* parent = 0)
        : QQuickItem(parent)
        , m_id(-1)
        , m_partOfSolution(false)
    {}

    Tile(int sid, QQuickItem* parent = 0);

    Tile(const Tile& other)
        : QQuickItem()
    {
        *this = other;
    }

    Tile& operator=(const Tile& other) {
        //qDebug() << "Tile equivalency";
        m_id = other.m_id;
        m_north = other.m_north;
        m_south = other.m_south;
        m_east = other.m_east;
        m_west = other.m_west;
        m_partOfSolution = other.m_partOfSolution;
        return *this;
    }

    Q_INVOKABLE int id() { return m_id; }
    Q_INVOKABLE bool northWall() { return m_north.wall; }
    Q_INVOKABLE bool southWall() { return m_south.wall; }
    Q_INVOKABLE bool eastWall() { return m_east.wall; }
    Q_INVOKABLE bool westWall() { return m_west.wall; }
    Q_INVOKABLE bool partOfSolution() { return m_partOfSolution; }

    Q_INVOKABLE void setId( int val );

    void setNorthWall( bool val ) {
        m_north.wall = val;
        emit northWallChanged();
    }

    void setSouthWall( bool val ) {
        m_south.wall = val;
        emit southWallChanged();
    }

    void setEastWall( bool val ) {
        m_east.wall = val;
        emit eastWallChanged();
    }

    void setWestWall( bool val ) {
        m_west.wall = val;
        emit westWallChanged();
    }

    void setPartOfSolution( bool val ) {
        m_partOfSolution = val;
        emit partOfSolutionChanged();
    }

    int m_id;
    Direction m_north;
    Direction m_south;
    Direction m_east;
    Direction m_west;

signals:
    void idChanged();
    void northWallChanged();
    void southWallChanged();
    void eastWallChanged();
    void westWallChanged();
    void partOfSolutionChanged();

private:
    bool m_partOfSolution;
};

class TrackHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int size READ size NOTIFY sizeChanged)
    Q_PROPERTY(int stepDuration READ stepDuration WRITE setStepDuration NOTIFY stepDurationChanged)
    Q_PROPERTY(int solutionSteps READ solutionSteps NOTIFY solutionStepsChanged)
    Q_PROPERTY(int stepsLeft READ stepsLeft WRITE setStepsLeft NOTIFY stepsLeftChanged)

public:
    explicit TrackHandler(QObject *parent = 0);

    ~TrackHandler();

    Q_INVOKABLE void loadTrack(const QString& track);
    Q_INVOKABLE void playForward();
    Q_INVOKABLE void playReverse();

    Q_INVOKABLE void createMaze();
    Q_INVOKABLE void solveMaze(int startPoint, int endPoint);

    Q_INVOKABLE void animationStarted(bool correctDirection);
    Q_INVOKABLE void animationStopped();

    Q_INVOKABLE void remaze(double value);

    int stepDuration() { return m_stepDuration; }
    void setStepDuration(int duration);

    int size() { return m_size; }

    int solutionSteps() { return m_solutionSteps; }

    int stepsLeft() { return m_stepsLeft; }
    void setStepsLeft(int steps);

signals:
    void sizeChanged();
    void stepDurationChanged();
    void solutionStepsChanged();
    void stepsLeftChanged();
    void newTrackLoaded();

private slots:
    void checkDuration();
    void mediaTimerTimeout();
    void mediaStateChanged(QMediaPlayer::State);

private:
    QSet< int > getNeighbors( Tile* input );

    int m_size;
    int m_stepDuration;
    int m_solutionSteps;
    int m_stepsLeft;
    int m_prevForwardVolume;
    qint64 m_currentTrackPos;
    qint64 m_trackDuration;
    QMediaPlayer* m_forwardPlayer;
    QMediaPlayer* m_reversePlayer;
    QStack< Tile* > m_locationStack;
    QStack< int > m_solutionStack;
    QTimer* m_mediaTimeoutTimer;
};

static QVector< Tile* > s_tiles;

#endif // TRACKHANDLER_H
