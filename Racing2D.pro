QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

SOURCES += \
    game.cpp \
    gamescene.cpp \
    main.cpp \
    view.cpp \
    InputDeviceHandler.cpp

HEADERS += \
    car.h \
    game.h \
    gamescene.h \
    view.h \
    my_ioctl.h \
    InputDeviceHandler.h

RESOURCES += \
    resource.qrc
