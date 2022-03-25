.. _install:
====================
Installing JPSreport
====================

Download installer
==================

Release
-------

- Windows installer:
- MacOS installer:


Nightly build
-------------

- Windows installer:
- MacOS installer:


Build from source
=================
It should be possible to build on all major platforms however, we only test a few:

Right now, we ensure a working Build for:

- Windows 10
- MacOS BigSur
- Ubuntu 20.04

Get the code
------------
In order to compile JPSreport, you need to download the source code via:

.. code:: bash

   git clone https://github.com/JuPedSim/jpscore.git

Build Options
-------------
We support only a few settings. For a list of build options, please see [CMakeLists.txt] directly.


General Information
-------------------
All of the following descriptions assume the following layout on disk:

::

    .
    ├── jpsreport <- code repository
    ├── jpsreport-build <- build folder
    └── jpsreport-deps <- install location of library dependencies


Build on Windows
----------------

System Requirements
^^^^^^^^^^^^^^^^^^^
- Visual Studio
- vcpkg (see https://vcpkg.io/en/getting-started.html)

Library Dependencies
^^^^^^^^^^^^^^^^^^^^
On Windows dependencies are installed with ``vcpkg``, get it from vcpkg.io.
``vcpkg`` will automatically download and install the dependencies listen in ``vcpkg.json`` if used with CMake.


Compiling
^^^^^^^^^
To call the CMake configuration and to install the needed dependencies, call

.. code:: bash

    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=<PATH-TO-VCPKG-INSTALLATION>/scripts/buildsystems/vcpkg.cmake <path-to-cmakelists>


To finally compile the source code, use

.. code:: bash

    cmake --build . --config Debug


Build on Linux
----------------

System Requirements
^^^^^^^^^^^^^^^^^^^

Builds are only tested on the latest Ubuntu LTS version (at this time 20.04).
To compile from source you will need a couple of system dependencies.

- C++17 capable compiler (default GCC will do)
- wget

Recommended:

- ninja

Library Dependencies
^^^^^^^^^^^^^^^^^^^^
On Linux and MacOS all dependencies are built from source, they are either part of the source tree and do not need any special attention or they are built with ``scripts/setup-deps.sh``.
To compile dependencies invoke the script and specify the install path:

.. code:: bash

    ./scripts/setup-deps.sh --install-path ~/jpsreport-deps


The output created in ``~jpsreport-deps`` now contains an install tree of all required library dependencies.

.. warning::
    If you do not specify an install path the script tries to install into ``/usr/local``.

Compiling
^^^^^^^^^
Now that you have all library dependencies, you need to generate build files with CMake and compile.

.. code:: bash

   mkdir jpsreport-build
   cd jpsreport-build
   cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<path-to-dependencies> <path-to-cmakelists>
   ninja

Alternatively you can generate a make based build with:

.. code:: bash

   mkdir jpsreport-build
   cd jpsreport-build
   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<path-to-dependencies> <path-to-cmakelists>
   make -j$(nproc)

You will find the ``jpsreport`` executable in ``jpsreport-build/bin`` after the build.

Build on MacOS
----------------

System Requirements
^^^^^^^^^^^^^^^^^^^

- AppleClang/XCode command line tools
- wget

Recommended:

- ninja

Library Dependencies
^^^^^^^^^^^^^^^^^^^^

On Linux and MacOS all dependencies are built from source, they are either part of the source tree and do not need any special attention or they are built with ``scripts/setup-deps.sh``.
To compile dependencies invoke the script and specify the install path:

.. code:: bash

    ./scripts/setup-deps.sh --install-path ~/jpsreport-deps


The output created in ``~jpsreport-deps`` now contains an install tree of all required library dependencies.

.. warning::
    If you do not specify an install path the script tries to install into ``/usr/local``.


Compiling
^^^^^^^^^

Now that you have all library dependencies, you need to generate build files with CMake and compile.

.. code:: bash

   mkdir jpsreport-build
   cd jpsreport-build
   cmake -GNinja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<path-to-dependencies> <path-to-cmakelists>
   ninja

Alternatively you can generate a make based build with:

.. code:: bash

   mkdir jpsreport-build
   cd jpsreport-build
   cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<path-to-dependencies> <path-to-cmakelists>
   make -j$(nproc)

You will find the ``jpsreport`` executable in ``jpsreport-build/bin`` after the build.
