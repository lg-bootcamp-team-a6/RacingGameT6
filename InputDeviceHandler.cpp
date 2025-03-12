#include "InputDeviceHandler.h"
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <QSocketNotifier>

#define KEY0_CODE 108  // SW2 이벤트 코드 (적절한 코드로 변경하세요)
#define KEY1_CODE 103  // SW2 이벤트 코드 (적절한 코드로 변경하세요)

InputDeviceHandler::InputDeviceHandler(GameScene* gameScene, QObject *parent)
    : QObject(parent), m_gameScene(gameScene)
{
    inputDevice = new QFile(DEV_NAME, this);
    if (!inputDevice->open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input device!";
    } else {
        qDebug() << "Input device opened successfully!";
        qDebug() << "Input device handle:" << inputDevice->handle();

        int fd = inputDevice->handle();
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        notifier = new QSocketNotifier(inputDevice->handle(), QSocketNotifier::Read, this);
        connect(notifier, &QSocketNotifier::activated, this, &InputDeviceHandler::processInputEvents);
        qDebug() << "QSocketNotifier created and connected.";
    }
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
        qWarning() << "Input device is not open!";
        return;
    }

    struct input_event ev;

    qDebug() << "Reading input event...";

    ssize_t bytesRead = inputDevice->read(reinterpret_cast<char*>(&ev), sizeof(ev));
    if (bytesRead == sizeof(ev)) {
        qDebug() << "[INTERRUPT] Event detected! Type:" << ev.type << ", Code:" << ev.code << ", Value:" << ev.value;

        // EV_KEY 처리
        if (ev.type == EV_KEY) {
            handleKeyEvent(ev);
        } else {
            qDebug() << "Event type is not EV_KEY, ignoring.";
        }
    } else {
        qWarning() << "Failed to read a complete input event. Bytes read:" << bytesRead;
    }
}

void InputDeviceHandler::handleKeyEvent(const struct input_event &ev)
{
    qDebug() << "[KEY EVENT] Code:" << ev.code << "Value:" << ev.value;

    // SW2 이벤트 처리
    if (ev.code == KEY0_CODE) {
        if (ev.value == 1) {
            qDebug() << "[ACTION] SW2 activated";
            m_gameScene->setUpDirection(true); // Set forward direction
        } else {
            qDebug() << "[ACTION] SW2 deactivated";
            m_gameScene->setUpDirection(false); // Unset forward direction
        }
    }
    else if (ev.code == KEY1_CODE) {
        if (ev.value == 1) {
            qDebug() << "[ACTION] SW3 activated";
            m_gameScene->setRightDirection(true); // Set forward direction
        } else {
            qDebug() << "[ACTION] SW3 deactivated";
            m_gameScene->setRightDirection(false); // Unset forward direction
        }
    }
    else {
        qDebug() << "Unknown key event, ignoring.";
    }
}
