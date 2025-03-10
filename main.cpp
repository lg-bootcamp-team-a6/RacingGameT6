#include <QApplication>
#include <QTimer>
#include "InputDeviceHandler.h"
#include "view.h"
#include <iostream>

int main(int argc, char** argv)
{
    QApplication a(argc,argv);

    InputDeviceHandler inputHandler;
    // 50ms마다 input events를 처리
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &inputHandler, &InputDeviceHandler::processInputEvents);
    timer.start(50);  // 50ms마다 처리

    qDebug() << "TESTESTESTESTTEST";

    View v;
    v.show();

    a.exec();


}
