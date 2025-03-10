#include "InputDeviceHandler.h"
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <QSocketNotifier>

#define DEV_NAME "/dev/input/event0"  // 입력 장치 파일 경로

InputDeviceHandler::InputDeviceHandler(QObject *parent)
    : QObject(parent), m_upDir(false), m_downDir(false), m_leftDir(false), m_rightDir(false)
{
    inputDevice = new QFile(DEV_NAME, this);
    if (!inputDevice->open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input device!";
    } else {
        qDebug() << "Input device opened successfully!";
    }

    QSocketNotifier *notifier = new QSocketNotifier(inputDevice->handle(), QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &InputDeviceHandler::processInputEvents);
}

InputDeviceHandler::~InputDeviceHandler()
{
    if (inputDevice->isOpen()) {
        inputDevice->close();
    }
}

void InputDeviceHandler::processInputEvents()
{
    if (!inputDevice->isOpen()) {
        return;
    }

    struct input_event ev;
    qDebug() << "[INFO] Listening for input events...";

    while (inputDevice->read(reinterpret_cast<char*>(&ev), sizeof(ev)) == sizeof(ev)) {
        qDebug() << "[INTERRUPT] Event detected! Type:" << ev.type << ", Code:" << ev.code << ", Value:" << ev.value;

        // EV_KEY만 처리
        if (ev.type == EV_KEY) {
            handleKeyEvent(ev);
        }
    }
}

void InputDeviceHandler::handleKeyEvent(const struct input_event &ev)
{
    bool isPressed = (ev.value == 1);

    qDebug() << "[KEY EVENT] Code:" << ev.code << (isPressed ? "pressed" : "released");

    // GPIO_KEY_UP에만 반응
    if (ev.code == KEY_UP) {  // KEY_UP은 GPIO_KEY_UP에 대응하는 키 코드입니다.
        if (isPressed) {
            m_upDir = true;
            qDebug() << "[ACTION] Up key pressed";
        } else {
            m_upDir = false;
            qDebug() << "[ACTION] Up key released";
        }
    }
}
