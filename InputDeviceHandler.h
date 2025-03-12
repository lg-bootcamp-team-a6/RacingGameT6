#ifndef INPUTDEVICEHANDLER_H
#define INPUTDEVICEHANDLER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <linux/input.h>
#include <QSocketNotifier> // Add this line
#include "gamescene.h"
#include <math.h>

#define DEV_NAME "/dev/input/event1"  // 해당 input device 경로

class InputDeviceHandler : public QObject
{
    Q_OBJECT

public:
    explicit InputDeviceHandler(GameScene* gameScene, QObject *parent = nullptr);
    ~InputDeviceHandler();

    void processInputEvents();
    void processAccEvents();

    GameScene *m_gameScene; // m_gameScene 멤버 변수 선언

private:
    QFile *inputDevice;
    QFile *accDevice;
    void handleKeyEvent(const struct input_event &ev);
    void handleAccEvent(const struct input_event &ev);
    QSocketNotifier *notifier; // Add this line
    QSocketNotifier *accNotifier;
    double calculateRotationAngleAxixz(int x, int y);
};

#endif // INPUTDEVICEHANDLER_H
