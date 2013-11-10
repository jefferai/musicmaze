// This work is copyright 2013 by Jeff Mitchell <jeff@jefferai.org>
// This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

#include "TrackHandler.h"

#include <QDebug>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QProcess>

Tile::Tile(int sid, QQuickItem* parent)
    : QQuickItem(parent)
{
    *this = *s_tiles[sid];
}

void
Tile::setId( int val ) {
    //qDebug() << "Setting tile" << val;
    if ( s_tiles[val]->m_id != -1 ) {
        *this = *s_tiles[val];
        s_tiles[val] = this;
        emit northWallChanged();
        emit southWallChanged();
        emit eastWallChanged();
        emit westWallChanged();
        emit partOfSolutionChanged();
    }
    else
        m_id = val;
    emit idChanged();
}


TrackHandler::TrackHandler(QObject *parent) :
    QObject(parent),
    m_size(50),
    m_stepDuration(0),
    m_trackDuration(0),
    m_forwardPlayer(new QMediaPlayer(parent, QMediaPlayer::LowLatency)),
    m_reversePlayer(new QMediaPlayer(parent, QMediaPlayer::LowLatency)),
    m_mediaTimeoutTimer(new QTimer(this))
{
    m_mediaTimeoutTimer->setTimerType(Qt::PreciseTimer);
    m_mediaTimeoutTimer->setSingleShot(true);
    m_mediaTimeoutTimer->setInterval(1000);
    connect(m_mediaTimeoutTimer, SIGNAL(timeout()), this, SLOT(mediaTimerTimeout()));

    connect(m_forwardPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(mediaStateChanged(QMediaPlayer::State)));
    connect(m_reversePlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(mediaStateChanged(QMediaPlayer::State)));

    remaze(m_size);
}

TrackHandler::~TrackHandler()
{
}


void
TrackHandler::remaze(double value)
{
    m_size = value;
    s_tiles.resize(m_size * m_size);
    for( int i = 0; i < s_tiles.size(); i++ ) {
        s_tiles[i] = new Tile();
        s_tiles[i]->setId( i );
    }
    createMaze();
    solveMaze(0, (m_size * m_size) - 1);
    emit sizeChanged();
    if (!m_forwardPlayer->media().isNull())
        m_forwardPlayer->setPosition(0);
    if (!m_reversePlayer->media().isNull())
        m_reversePlayer->setPosition(0);
    m_currentTrackPos = 0;
}

void
TrackHandler::setStepsLeft(int steps)
{
    qDebug() << Q_FUNC_INFO << "Setting steps left to" << steps;
    m_stepsLeft = steps;
    emit stepsLeftChanged();
}


void
TrackHandler::setStepDuration(int duration)
{
    qDebug() << Q_FUNC_INFO << "Setting step duration to" << duration;
    m_stepDuration = duration;
    emit stepDurationChanged();
}


void
TrackHandler::loadTrack(const QString& track)
{
    qDebug() << Q_FUNC_INFO << "Loading track" << track;
    setStepDuration(0);
    QUrl trackUrl(track);
    QTemporaryFile tmpfile;
    if ( !tmpfile.open() ) {
        qDebug() << "Could not open file!";
        return;
    }

    QStringList args;
    args << trackUrl.toLocalFile() << tmpfile.fileName() + "-forward.wav";
    QProcess::execute("/usr/local/bin/sox", args);
    args.clear();
    args << trackUrl.toLocalFile() << tmpfile.fileName() + "-reverse.wav" << "reverse";
    QProcess::execute("/usr/local/bin/sox", args);
    m_forwardPlayer->setMedia(QUrl::fromLocalFile(tmpfile.fileName() + "-forward.wav"));
    qDebug() << "Forward track is at " << m_forwardPlayer->media().canonicalUrl();
    m_reversePlayer->setMedia(QUrl::fromLocalFile(tmpfile.fileName() + "-reverse.wav"));
    qDebug() << "Reverse track is at " << m_reversePlayer->media().canonicalUrl();

    //hack to make the duration work
    m_prevForwardVolume = m_forwardPlayer->volume();
    m_forwardPlayer->setVolume(0);
    m_reversePlayer->setVolume(0);
    m_forwardPlayer->play();
    m_reversePlayer->play();
    QTimer::singleShot(20, this, SLOT(checkDuration()));
}


void
TrackHandler::checkDuration()
{
    qDebug() << Q_FUNC_INFO;

    if (m_forwardPlayer->duration() == 0 || m_reversePlayer->duration() == 0)
    {
        QTimer::singleShot(20, this, SLOT(checkDuration()));
        qDebug() << "Waiting for duration";
        return;
    }
    m_trackDuration = m_forwardPlayer->duration();
    m_forwardPlayer->pause();
    m_reversePlayer->pause();
    m_forwardPlayer->setVolume(m_prevForwardVolume);
    m_reversePlayer->setVolume(m_prevForwardVolume);
    m_forwardPlayer->setPosition(0);
    m_reversePlayer->setPosition(m_trackDuration);
    if (m_solutionStack.size()) {
        qDebug() << "Setting step duration to" << m_trackDuration / m_solutionStack.size();
        setStepDuration(m_trackDuration / m_solutionStack.size());
    }
    emit newTrackLoaded();
}


void
TrackHandler::mediaStateChanged(QMediaPlayer::State state)
{
    if ( state != QMediaPlayer::StoppedState )
        return;

    qDebug() << Q_FUNC_INFO;

    if ( sender() == qobject_cast< QObject* >( m_reversePlayer ) ) {
        m_forwardPlayer->setPosition( 0 );
        m_reversePlayer->setPosition( m_trackDuration );
        m_currentTrackPos = 0;
    } else if ( sender() == qobject_cast< QObject* >( m_forwardPlayer ) ) {
        m_forwardPlayer->setPosition( m_trackDuration );
        m_reversePlayer->setPosition( 0 );
        m_currentTrackPos = m_trackDuration;
    } else {
        qDebug() << "Woah, nothing sent this signal!";
    }
}


void
TrackHandler::playForward()
{
    qDebug() << Q_FUNC_INFO;
    if ( m_forwardPlayer->state() == QMediaPlayer::PlayingState )
        return;
    if ( m_reversePlayer->state() == QMediaPlayer::PlayingState ) {
        m_forwardPlayer->setPosition( m_trackDuration - m_reversePlayer->position() );
        m_forwardPlayer->play();
        QTimer::singleShot(100, m_reversePlayer, SLOT(pause()));
        return;
    }
    if ( m_forwardPlayer->state() == QMediaPlayer::PausedState ) {
        m_forwardPlayer->setPosition( m_currentTrackPos );
        m_forwardPlayer->play();
        return;
    }
    m_reversePlayer->pause();
//    m_forwardPlayer->setPosition( m_forwardPlayer->position() > m_reversePlayer->position() ?
//                                      m_forwardPlayer->position() : m_reversePlayer->position() );
    m_forwardPlayer->setPosition( m_currentTrackPos );
    m_forwardPlayer->play();
}

void
TrackHandler::playReverse()
{
    qDebug() << Q_FUNC_INFO;
    if ( m_reversePlayer->state() == QMediaPlayer::PlayingState )
        return;
    if ( m_forwardPlayer->state() == QMediaPlayer::PlayingState ) {
        m_reversePlayer->setPosition( m_trackDuration - m_forwardPlayer->position() );
        m_reversePlayer->play();
        QTimer::singleShot(100, m_forwardPlayer, SLOT(pause()));
        return;
    }
    if ( m_reversePlayer->state() == QMediaPlayer::PausedState ) {
        m_reversePlayer->setPosition( m_trackDuration - m_currentTrackPos );
        m_reversePlayer->play();
        return;
    }

//    m_reversePlayer->setPosition( m_forwardPlayer->position() > m_reversePlayer->position() ?
//                                      m_forwardPlayer->position() : m_reversePlayer->position() );
    m_reversePlayer->setPosition( m_trackDuration - m_currentTrackPos );
    m_reversePlayer->play();
}


QSet< int >
TrackHandler::getNeighbors( Tile* input )
{
    QSet< int > tiles;
    if ( input->m_id + 1 < s_tiles.size() ) {
        // Don't take anything at the very right side
        if ( ((input->m_id + 1) % m_size) != 0 ) {
            tiles.insert( input->m_id + 1 );
        }
    }
    if ( input->m_id + m_size < s_tiles.size() ) {
        tiles.insert( input->m_id + m_size );
    }
    if ( input->m_id - 1 > 0 ) {
        // Don't take anything at the very left side
        if ( (input->m_id % m_size) != 0 ) {
            tiles.insert( input->m_id - 1 );
        }
    }
    if ( input->m_id - m_size > 0 ) {
        tiles.insert( input->m_id - m_size );
    }
    return tiles;
}


void
TrackHandler::createMaze()
{
    qDebug() << Q_FUNC_INFO;

    m_locationStack.clear();

    int currTileNum = 0;
    int visitedTiles = 1;

    QSet< Tile* > fullNeighbors;

    while ( visitedTiles < s_tiles.size() ) {
        fullNeighbors.clear();
        Tile* currTile = s_tiles[currTileNum];
        QSet< int > neighbors = getNeighbors( currTile );
        for ( int neighborId : neighbors ) {
            Tile* neighbor = s_tiles[neighborId];
            if ( neighbor->northWall() &&
                 neighbor->southWall() &&
                 neighbor->eastWall() &&
                 neighbor->westWall() )
                fullNeighbors.insert( neighbor );
        }
        if ( fullNeighbors.size() ) {
            Tile* chosen = 0;
            for ( Tile* candidate : fullNeighbors.values() ) {
                if ( candidate->m_id == (m_size * m_size) - 1 )
                {
                    chosen = candidate;
                    break;
                }
            }
            if ( !chosen )
                chosen = fullNeighbors.values()[qrand() % fullNeighbors.size()];
            if ( chosen->m_id == currTileNum + 1 ) {
                currTile->setEastWall( false );
                chosen->setWestWall( false );
            } else if ( chosen->m_id == currTileNum + m_size ) {
                currTile->setSouthWall( false );
                chosen->setNorthWall( false );
            } else if ( chosen->m_id == currTileNum - 1 ) {
                currTile->setWestWall( false );
                chosen->setEastWall( false );
            } else {
                currTile->setNorthWall( false );
                chosen->setSouthWall( false );
            }
            m_locationStack.push(currTile);
            currTileNum = chosen->m_id;
            visitedTiles++;
        } else {
            currTileNum = m_locationStack.pop()->m_id;
        }
    }
}

void
TrackHandler::solveMaze(int startPoint, int endPoint)
{
    qDebug() << Q_FUNC_INFO << "Solving maze for startpoint" << startPoint << "and endpoint" << endPoint;

    Tile* currTile;
    for ( int i = 0; i < s_tiles.size(); i++ ) {
        currTile = s_tiles[i];
        currTile->setPartOfSolution(false);
        currTile->m_north.visited = false;
        currTile->m_west.visited = false;
        currTile->m_east.visited = false;
        currTile->m_south.visited = false;
    }

    int currTileNum = startPoint;

    m_solutionStack.clear();
    m_solutionStack.push(currTileNum);
    while ( currTileNum != endPoint ) {
        currTile = s_tiles[currTileNum];
        //qDebug() << "Solution stack size: " << m_solutionStack.size() << ", curr tile is" << currTileNum;
        if ( !currTile->m_north.wall && !currTile->m_north.visited ) {
            //qDebug() << "Going north";
            m_solutionStack.push(currTile->m_id);
            currTile->m_north.visited = true;
            currTileNum = currTileNum - m_size;
            s_tiles[currTileNum]->m_south.visited = true;
            continue;
        } else if ( !currTile->m_east.wall && !currTile->m_east.visited ) {
            //qDebug() << "Going east";
            m_solutionStack.push(currTile->m_id);
            currTile->m_east.visited = true;
            currTileNum = currTileNum + 1;
            s_tiles[currTileNum]->m_west.visited = true;
            continue;
        } else if ( !currTile->m_south.wall && !currTile->m_south.visited ) {
            //qDebug() << "Going south";
            m_solutionStack.push(currTile->m_id);
            currTile->m_south.visited = true;
            currTileNum = currTileNum + m_size;
            s_tiles[currTileNum]->m_north.visited = true;
            continue;
        } else if ( !currTile->m_west.wall && !currTile->m_west.visited ){
            //qDebug() << "Going west";
            m_solutionStack.push(currTile->m_id);
            currTile->m_west.visited = true;
            currTileNum = currTileNum - 1;
            s_tiles[currTileNum]->m_east.visited = true;
            continue;
        }
        // No valid direction we haven't been, so backtrack
        //qDebug() << "Backtracking";
        currTileNum = m_solutionStack.pop();
    }
    m_solutionStack.push(currTileNum);

    //qDebug() << "Done finding solution";

    for ( int solutionTile : m_solutionStack )
    {
        s_tiles[solutionTile]->setPartOfSolution(true);
        //qDebug() << "Tile" << solutionTile << "marked part of solution";
    }

    if ( m_trackDuration )
    {
        qDebug() << Q_FUNC_INFO << "Setting step duration to" << m_trackDuration / m_solutionStack.size();
        if ( m_forwardPlayer->position() != 0 || m_reversePlayer->position() != 0 ) {
            setStepDuration((m_trackDuration - m_currentTrackPos) / m_stepsLeft);
        }
        else
            setStepDuration(m_trackDuration / m_solutionStack.size());
    }

    m_solutionSteps = m_solutionStack.size();
    emit solutionStepsChanged();
}


void
TrackHandler::animationStarted( bool correctDirection )
{
    qDebug() << Q_FUNC_INFO;
    m_mediaTimeoutTimer->stop();
    if ( correctDirection )
        playForward();
    else
        playReverse();
}


void
TrackHandler::animationStopped()
{
    qDebug() << Q_FUNC_INFO;
    m_mediaTimeoutTimer->start();
}


void
TrackHandler::mediaTimerTimeout()
{
    qDebug() << Q_FUNC_INFO;
    if ( m_forwardPlayer->state() == QMediaPlayer::PlayingState )
    {
        m_forwardPlayer->pause();
        m_currentTrackPos = m_forwardPlayer->position();
        setStepDuration((m_trackDuration - m_currentTrackPos) / m_stepsLeft);
    }
    else if ( m_reversePlayer->state() == QMediaPlayer::PlayingState) {
        m_reversePlayer->pause();
        m_currentTrackPos = m_trackDuration - m_reversePlayer->position();
        setStepDuration((m_trackDuration - m_currentTrackPos) / m_stepsLeft);
    }
}
