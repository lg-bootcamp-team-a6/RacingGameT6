QT       += \
    core gui \
    network \
    
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    game.cpp \
    gamescene.cpp \
    main.cpp \
    view.cpp \
    InputDeviceHandler.cpp \
    UdpSokcetHandler.cpp

HEADERS += \
    car.h \
    game.h \
    gamescene.h \
    view.h \
    InputDeviceHandler.h \
    UdpSocketHandler.h

RESOURCES += \
    resource.qrc \
    module \

# set binary destination
DESTDIR = /nfsroot

# add module build steps
QMAKE_POST_LINK = cd $$PWD/module && make clean && make
