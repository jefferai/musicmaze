MusicMaze
====================
A maze generator that clues you in to the right path via the track you
select. You get to the end when the song gets to the end.

Created from scratch for Music Hack Day Boston 2013.

Features
--------------------
* Click anywhere on the maze to set a new endpoint
* Pick a new track at any time without having to start over
* Change the maze difficulty/speed by changing the size (starts you
  over)

Building
-------------------
* Qt 5.1 or higher (http://qt-project.org/downloads)
* SoX (on OSX: `brew install sox`). If sox is not in /usr/local/bin you
  will need to update the path in TrackHandler.cpp.

Just make it with qmake/make and run the resulting app bundle/binary

Terms
-------------------
Copyright 2013 by Jeff Mitchell (<jeff@jefferai.org>)

This work is licensed under a Creative Commons Attribution-ShareAlike
3.0 Unported License (<http://creativecommons.org/licenses/by-sa/3.0/deed.en_US>)
