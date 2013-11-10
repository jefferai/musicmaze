// This work is copyright 2013 by Jeff Mitchell <jeff@jefferai.org>
// This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

import QtQuick 2.1
import MusicMaze 1.0

Rectangle {
    id: tileRect
    property int tileNum
    property bool current
    property alias solution : localTile.partOfSolution
    property alias westWall : localTile.westWall
    property alias eastWall : localTile.eastWall
    property alias northWall : localTile.northWall
    property alias southWall : localTile.southWall
    width: outerRect.tileSize
    height: outerRect.tileSize
    color: "black"
    Tile {
        id: localTile
        Component.onCompleted: {
            setId(tileNum)
            //console.log("Completed with solution: " + solution)
            if (tileNum == 0) {
                person.visible = true
            }
        }
        onObjectNameChanged: {
            setId(tileNum)
        }
    }

    Rectangle {
        id: topWall
        color: "white"
        anchors.top: tileRect.top
        anchors.horizontalCenter: tileRect.horizontalCenter
        radius: 0.8
        width: tileRect.width
        height: tileRect.height * 0.1
        visible: tileRect.northWall
    }

    Rectangle {
        id: rightWall
        color: "white"
        anchors.right: tileRect.right
        anchors.verticalCenter: tileRect.verticalCenter
        radius: 0.8
        width: tileRect.width * 0.1
        height: tileRect.height
        visible: tileRect.eastWall
    }

    Rectangle {
        id: bottomWall
        color: "white"
        anchors.bottom: tileRect.bottom
        anchors.horizontalCenter: tileRect.horizontalCenter
        radius: 0.8
        width: tileRect.width
        height: tileRect.height * 0.1
        visible: tileRect.southWall
    }

    Rectangle {
        id: leftWall
        color: "white"
        anchors.left: tileRect.left
        anchors.verticalCenter: tileRect.verticalCenter
        radius: 0.8
        width: tileRect.width * 0.1
        height: tileRect.height
        visible: tileRect.westWall
    }

    Rectangle {
        id: startTile
        width: tileRect.width * 0.8
        height: tileRect.height * 0.8
        anchors.centerIn: tileRect
        anchors.margins: outerRect.tileSize * 0.2
        color: outerRect.firstTile === tileRect.tileNum ? "red" : "green"
        visible: outerRect.firstTile === tileRect.tileNum || outerRect.lastTile === tileRect.tileNum
    }

    Rectangle {
        width: tileRect.width * 0.25
        height: tileRect.height * 0.25
        anchors.centerIn: tileRect
        z: 5
        color: "yellow"
        radius: width * 0.5
        visible: outerRect.showSolution && tileRect.solution
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        enabled: mazeOverlay.visible === false
        onClicked: {
            outerRect.firstTile = outerRect.currTile
            outerRect.lastTile = tileRect.tileNum
            th.solveMaze(outerRect.firstTile, tileRect.tileNum)
        }
    }
}
