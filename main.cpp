#include <QApplication>
#include <QTimer>
#include "InputDeviceHandler.h"
#include "view.h"
#include <iostream>

int main(int argc, char** argv)
{
    QApplication a(argc, argv);

    View v;

    InputDeviceHandler inputHandler(v.m_gameScene, &v);

    v.show();

    return a.exec();
}
