#ifndef INPUTDEVICEHANDLER_H
#define INPUTDEVICEHANDLER_H

#include <QObject>
#include <QFile>
#include <QDebug>
#include <linux/input.h>

#define DEV_NAME "/dev/input/event0"  // 해당 input device 경로

class InputDeviceHandler : public QObject
{
    Q_OBJECT

public:
    explicit InputDeviceHandler(QObject *parent = nullptr);
    ~InputDeviceHandler();

    void processInputEvents();

private:
    QFile *inputDevice;
    void handleKeyEvent(const struct input_event &ev);

    // 키 입력 상태
    bool m_upDir = false;
    bool m_downDir = false;
    bool m_leftDir = false;
    bool m_rightDir = false;
};

#endif // INPUTDEVICEHANDLER_H
