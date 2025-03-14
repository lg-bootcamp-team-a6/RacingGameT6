#ifndef INPUTDEVICEHANDLER_H
#define INPUTDEVICEHANDLER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <linux/input.h>
#include <QSocketNotifier> // Add this line
#include "gamescene.h"
#include <math.h>
#include <QSocketNotifier> // Add this line
#include "gamescene.h"
#include "view.h"


class InputDeviceHandler : public QObject
{
    Q_OBJECT

public:
    explicit InputDeviceHandler(GameScene* gameScene, View* view, QObject *parent = nullptr);
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

    /* sh add */
    bool m_bIsResume;
    View * m_View;
};

#endif // INPUTDEVICEHANDLER_H
