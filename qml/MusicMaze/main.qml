// This work is copyright 2013 by Jeff Mitchell <jeff@jefferai.org>
// This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License
// http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Dialogs 1.0
import MusicMaze 1.0
import QtQuick.Window 2.1

Rectangle {
    id: outerRect
    width: mainWindow.width
    height: mainWindow.height
    color: "grey"
    property double tileSize: {
        if (width > (height - loadRect.height - borderRect.height - mazeRect.anchors.margins)) {
            (height - loadRect.height - borderRect.height - mazeRect.anchors.margins) / th.size
        } else {
            width / th.size
        }
    }
    onTileSizeChanged: console.log("width is " + width + ", height is " + height + ", tileSize is " + tileSize)
    property int firstTile: 0
    property int lastTile: (th.size * th.size) - 1
    property int currTile: 0
    property bool showSolution: false
    property bool correctDirection: false
    property int oldStepDuration: 0
    property bool widthChanging: false
    property bool heightChanging: false
    onWidthChanged: {
        widthChanging = true
    }
    onHeightChanged: {
        heightChanging = true
    }

    Component.onCompleted: {
        mainWindow.width = 760
        mainWindow.height = 600
    }

    TrackHandler {
        id: th
        stepsLeft: solutionSteps
        onNewTrackLoaded: outerRect.firstTile = outerRect.currTile
        onSolutionStepsChanged: th.stepsLeft = th.solutionSteps
    }

    Rectangle {
        id: loadRect
        color: "black"
        width: parent.width
        height: 30
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left

        SpinBox {
            id: sizeSpinner
            anchors.right: loadTrackButton.left
            anchors.verticalCenter: loadRect.verticalCenter
            anchors.rightMargin: 30
            minimumValue: 10
            maximumValue: 90
            value: 50
            stepSize: 1
            onValueChanged: {
                outerRect.widthChanging = true
                outerRect.heightChanging = true
                outerRect.firstTile = 0
                outerRect.currTile = 0
                th.remaze(value)
            }
        }

        Button {
            id: loadTrackButton
            text: "Load Track"
            anchors.verticalCenter: loadRect.verticalCenter
            anchors.horizontalCenter: loadRect.horizontalCenter
            anchors.leftMargin: 30
            anchors.rightMargin: 30
            onClicked: {
                outerRect.oldStepDuration = th.stepDuration
                th.stepDuration = 0
                trackDialog.open()
            }
        }

        Text {
            id: cheatText
            text: "(Press the S key to cheat)"
            anchors.left: loadTrackButton.right
            anchors.verticalCenter: loadRect.verticalCenter
            anchors.leftMargin: 30
            color: "white"
            font.pointSize: 16
        }
    }

    FileDialog {
        id: trackDialog
        title: "Please choose a track"
        selectFolder: false
        selectMultiple: false
        nameFilters: [ "Music files (*.mp3)" ]
        onAccepted: {
            console.log("You chose: " + trackDialog.fileUrls[0])
            mazeOverlayText.text = "Loading..."
            th.loadTrack(trackDialog.fileUrls[0])
        }
        onRejected: {
            console.log("Canceled")
            th.stepDuration = outerRect.oldStepDuration
        }
    }

    Rectangle {
        id: borderRect
        width: parent.width
        height: 10
        color: "white"
        anchors.top: loadRect.bottom
        anchors.right: parent.right
        anchors.left: parent.left
    }

    Rectangle {
        id: mazeRect
        width: parent.width
        height: parent.width
        anchors.top: borderRect.bottom
        anchors.horizontalCenter: borderRect.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.margins: 5
        color: "grey"
        focus: mazeOverlay.visible !== true

        Grid {
            id: grid
            rows: th.size
            columns: th.size
            anchors.horizontalCenter: parent.horizontalCenter
            Repeater {
                id: gridRepeater
                model: th.size * th.size
                TileRect {
                    tileNum: index
                    solution: false
                    current: false
                }
                onCountChanged: {
                    for(var i = 0; i < model; i++) {
                        console.log("Running setId on " + i)
                        if (itemAt(i) !== null) {
                            itemAt(i).objectName = "newitem" + i
                        }
                    }
                }
            }
        }

        Rectangle {
            id: person
            width: outerRect.tileSize * 0.66
            height: width
            x: Math.floor(outerRect.currTile % th.size) * outerRect.tileSize + grid.x + height / 4
            y: Math.floor(outerRect.currTile / th.size) * outerRect.tileSize + height / 4
            z: 10
            color: "cyan"
            radius: width * 0.5
            Behavior on x {
                NumberAnimation {
                    id: xAni
                    duration: outerRect.widthChanging ? 0 : th.stepDuration
                    alwaysRunToEnd: true
                    onRunningChanged: {
                        if (xAni.running) {
                            if (outerRect.widthChanging === false) {
                                console.log("Triggering animation start")
                                th.stepsLeft = th.stepsLeft + (outerRect.correctDirection ? -1 : 1)
                                th.animationStarted(outerRect.correctDirection)
                            }
                        } else {
                            if (outerRect.widthChanging === false) {
                                console.log("Triggering animation stop")
                                th.animationStopped()
                            }
                        }
                    }
                }
            }
            Behavior on y {
                NumberAnimation {
                    id: yAni
                    duration: outerRect.heightChanging ? 0 : th.stepDuration
                    alwaysRunToEnd: true
                    onRunningChanged: {
                        if (yAni.running) {
                            if (outerRect.heightChanging === false) {
                                console.log("Triggering animation start")
                                th.stepsLeft = th.stepsLeft + (outerRect.correctDirection ? -1 : 1)
                                th.animationStarted(outerRect.correctDirection)
                            }
                        } else {
                            if (outerRect.heightChanging === false) {
                                console.log("Triggering animation stop")
                                th.animationStopped()
                            }
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: mazeOverlay
        width: outerRect.width
        height: outerRect.height - loadRect.height - borderRect.height
        anchors.bottom: outerRect.bottom
        anchors.right: outerRect.right
        anchors.left: outerRect.left
        color: "grey"
        opacity: 0.75
        z: 20
        focus: th.stepDuration === 0
        visible: th.stepDuration === 0

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            enabled: mazeOverlay.visible
            onClicked: {
                trackDialog.open()
            }
        }

        Text {
            id: mazeOverlayText
            text: "Select a track"
            anchors.centerIn: parent
            font.pointSize: 48
            color: "white"
        }
    }


    Keys.enabled: mazeOverlay.visible === false && yAni.running === false && xAni.running === false

    Keys.onRightPressed: {
        outerRect.heightChanging = false
        outerRect.widthChanging = false
        if (!gridRepeater.itemAt(currTile).eastWall) {
            gridRepeater.itemAt(currTile).current = false
            if (gridRepeater.itemAt(currTile + 1).solution) {
                gridRepeater.itemAt(currTile).solution = false
                outerRect.correctDirection = true
            } else {
                gridRepeater.itemAt(currTile).solution = true
                outerRect.correctDirection = false
            }
            currTile = currTile + 1
            gridRepeater.itemAt(currTile).current = true
            console.log("Current tile is now " + currTile)
        }
    }

    Keys.onDownPressed: {
        outerRect.heightChanging = false
        outerRect.widthChanging = false
        if (!gridRepeater.itemAt(currTile).southWall) {
            gridRepeater.itemAt(currTile).current = false
            if (gridRepeater.itemAt(currTile + th.size).solution) {
                gridRepeater.itemAt(currTile).solution = false
                outerRect.correctDirection = true
            } else {
                gridRepeater.itemAt(currTile).solution = true
                outerRect.correctDirection = false
            }
            currTile = currTile + th.size
            gridRepeater.itemAt(currTile).current = true
            console.log("Current tile is now " + currTile)
        }
    }

    Keys.onUpPressed: {
        outerRect.heightChanging = false
        outerRect.widthChanging = false
        if (!gridRepeater.itemAt(currTile).northWall) {
            gridRepeater.itemAt(currTile).current = false
            if (gridRepeater.itemAt(currTile - th.size).solution) {
                gridRepeater.itemAt(currTile).solution = false
                outerRect.correctDirection = true
            } else {
                gridRepeater.itemAt(currTile).solution = true
                outerRect.correctDirection = false
            }
            currTile = currTile - th.size
            gridRepeater.itemAt(currTile).current = true
            console.log("Current tile is now " + currTile)
        }
    }

    Keys.onLeftPressed: {
        outerRect.heightChanging = false
        outerRect.widthChanging = false
        if (!gridRepeater.itemAt(currTile).westWall) {
            gridRepeater.itemAt(currTile).current = false
            if (gridRepeater.itemAt(currTile - 1).solution) {
                gridRepeater.itemAt(currTile).solution = false
                outerRect.correctDirection = true
            } else {
                gridRepeater.itemAt(currTile).solution = true
                outerRect.correctDirection = false
            }
            currTile = currTile - 1
            gridRepeater.itemAt(currTile).current = true
            console.log("Current tile is now " + currTile)
        }
    }

    Keys.onPressed: {
        if ( event.key === Qt.Key_S ) {
            outerRect.showSolution = outerRect.showSolution ? false : true
        }
    }
}
