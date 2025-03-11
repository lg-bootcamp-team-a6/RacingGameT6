#ifndef INPUTDEVICEHANDLER_H
#define INPUTDEVICEHANDLER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <linux/input.h>
#include <QSocketNotifier> // Add this line
#include "gamescene.h"

#define DEV_NAME "/dev/input/event1"  // 해당 input device 경로

class InputDeviceHandler : public QObject
{
    Q_OBJECT

public:
    explicit InputDeviceHandler(GameScene* gameScene, QObject *parent = nullptr);
    ~InputDeviceHandler();

    void processInputEvents();
    GameScene *m_gameScene; // m_gameScene 멤버 변수 선언

private:
    QFile *inputDevice;
    void handleKeyEvent(const struct input_event &ev);
    QSocketNotifier *notifier; // Add this line
};

#endif // INPUTDEVICEHANDLER_H
