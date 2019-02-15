# async_playground

Playground project to test library approaches for handling asnyc Qt/C++ code.

Currently includes test code for handling QtConcurrent or Signal/Slot async code via:

- KAsync: https://github.com/KDE/kasync

  I don't really like it, and wrose of all the code compiles but doesn't do what I think it should...

- QFuture: https://doc.qt.io/qt-5/qfuturewatcher.html

  It isn't trivial to roll your own continuation library, so don't go down that road

- QtPromise::QPromise: https://github.com/simonbrunel/qtpromise

  Quite nice and under active development. Great that it mimicks the Promise/A+ API! This makes it easy to learn
  this library when you already know JavaScript promises. Some API could be made even simpler, and upstream is working on that.

- AsyncFuture: https://github.com/benlau/asyncfuture

  Would be perfect if this could use the Promise/A+ API, but otherwise it's quite feature complete and straight-forward to use!
