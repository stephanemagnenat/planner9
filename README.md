Planner9
========

Planner9 is an open-source distributed HTN task planner developed by [Stéphane Magnenat](http://stephane.magnenat.net) and Martin Voelkle.

To know more about it, read the [associated paper](https://infoscience.epfl.ch/record/140810/files/planner9-icira2009.pdf).

Compilation
-----------

Planner9 uses [CMake](http://cmake.org).
It requires the [libboost and libboost-thread libraries](http://www.boost.org/), and, for the distributed version, [Qt4](http://doc.qt.io/qt-4.8/).

Test it
-------

You can run a test program, implementing the example of the paper, with:

    programs/p9simple
    
You can see the threaded version of the planner in action with:

    programs/p9threaded
    
The distributed version example is not documented yet.
