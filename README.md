# QSSTV
QSSTV is a program for receiving and transmitting SSTV and HAMDRM (sometimes called DSSTV). It is compatible with most of MMSSTV and EasyPal

## Installation

### Dependencies 

For apt based distros you can install dependencies as follows:

```
apt install pkg-config g++ libfftw3-dev qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools libqt5svg5-dev libhamlib++-dev libasound2-dev libpulse-dev libopenjp2-7 libopenjp2-7-dev libv4l-dev build-essential
```

### Compile and Install
	mkdir src/build
	cd src/build
	qmake ..
	make -j2
	sudo make install

Note: make -j2, 2 is the number of cores to be used for parallel compiling. If you have more cores, use a higher number.

### Debug Compile
If you have problems compiling the software, please give as much information as possible but at least:
* Linux Distribution and Version (e.g. Ubuntu 18.04)
* QT Version (e.g. Qt 5.4.1)
* Screen dump of the compile process showing the error

If you want to be able to debug the program, the simplest way is to install QtCreator and from within QtCreator open a new project and point to the qsstv.pro file. Note: you will need to install doxygen and libqwt

`sudo apt-get install doxygen libqwt-qt5-dev`

You can also run qmake with the following attributes:

`qmake CONFIG+=debug`

and use an external debugger (such as gdb)
