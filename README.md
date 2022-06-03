# QSSTV

## Dependencies
* pkg-config
* g++
* libfftw3-dev
* qt5-default
* hamlib-dev or libhamlib++-dev
* libasound2-dev
* libpulse-dev
* libopenjp2-7
* libopenjp2-7-dev
* lib4l-dev

## Compile and Install
    mkdir src/build
    cd src/build
    qmake ..
    make -j2
    sudo make install

Note: make -j2, 2 is the number of cores to be used for parallel compiling. If you have more cores, use a higher number.


If you have problems compiling the software, please give as much information as possible but at least: