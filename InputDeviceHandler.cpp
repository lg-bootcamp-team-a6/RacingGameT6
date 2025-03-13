#include "InputDeviceHandler.h"
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <QSocketNotifier>

#define DEV_NAME "/dev/input/event2"  // 입력 장치 파일 경로
#define ACC_NAME "/dev/input/event0"
#define KEY0_CODE 108  // SW2 이벤트 코드 (적절한 코드로 변경하세요)
#define KEY1_CODE 103  // SW3 이벤트 코드 (적절한 코드로 변경하세요)

InputDeviceHandler::InputDeviceHandler(GameScene* gameScene, QObject *parent)
    : QObject(parent), m_gameScene(gameScene), m_bIsResume(false)
{
    inputDevice = new QFile(DEV_NAME, this);
    accDevice = new QFile(ACC_NAME, this);
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
    if(!accDevice->open(QIODevice::ReadOnly)){
        qWarning() << "Failed to open acc device!";
    }
    else {
        qDebug() << "Accelerometer device opened successfully!";
        qDebug() << "Accelerometer device handle:" << accDevice->handle();
        int acc_fd = accDevice->handle();
        int acc_flags = fcntl(acc_fd, F_GETFL, 0);
        fcntl(acc_fd, F_SETFL, acc_flags | O_NONBLOCK);
        accNotifier = new QSocketNotifier(accDevice->handle(), QSocketNotifier::Read, this);
        connect(accNotifier, &QSocketNotifier::activated, this, &InputDeviceHandler::processAccEvents);
        qDebug() << "Accelerometer QSocketNotifier created and connected.";
    }
}

InputDeviceHandler::~InputDeviceHandler()
{
    if (inputDevice->isOpen()) {
        inputDevice->close();
    }
    if (accDevice->isOpen()) {
        accDevice->close();
    }
}

void InputDeviceHandler::processInputEvents()
{
    if (!inputDevice->isOpen()) {
        qWarning() << "Input device is not open!";
        return;
    }

    struct input_event ev;

    //qDebug() << "Reading input event...";

    ssize_t bytesRead = inputDevice->read(reinterpret_cast<char*>(&ev), sizeof(ev));
    if (bytesRead == sizeof(ev)) {
        //qDebug() << "[INTERRUPT] Event detected! Type:" << ev.type << ", Code:" << ev.code << ", Value:" << ev.value;

        // EV_KEY 처리
        if (ev.type == EV_KEY) {
            handleKeyEvent(ev);
        } else {
            //qDebug() << "Event type is not EV_KEY, ignoring.";
        }
    } else {
        qWarning() << "Failed to read a complete input event. Bytes read:" << bytesRead;
    }
}

void InputDeviceHandler::processAccEvents()
{
    if (!accDevice->isOpen()) {
        qWarning() << "Accelerometer device is not open!";
        return;
    }

    struct input_event ev;

    //qDebug() << "Reading accelerometer event...";

    ssize_t bytesRead = accDevice->read(reinterpret_cast<char*>(&ev), sizeof(ev));
    if (bytesRead == sizeof(ev)) {
        //qDebug() << "[ACCELEROMETER INTERRUPT] Event detected! Type:" << ev.type << ", Code:" << ev.code << ", Value:" << ev.value;

        // EV_ABS 처리
        if (ev.type == EV_ABS) {
            handleAccEvent(ev);
        } else {
            //qDebug() << "Event type is not EV_ABS, ignoring.";
        }
    } else {
        qWarning() << "Failed to read a complete accelerometer event. Bytes read:" << bytesRead;
    }
}

void InputDeviceHandler::handleKeyEvent(const struct input_event &ev)
{
    //qDebug() << "[KEY EVENT] Code:" << ev.code << "Value:" << ev.value;

    // SW2 이벤트 처리
    if (ev.code == KEY0_CODE) {
        if (ev.value == 1) {
            if (!m_bIsResume) {
                qDebug() << "[ACTION] Toggle Pause";
                m_gameScene->togglePause(m_bIsResume);
                m_bIsResume = true;
            } else {
                // m_gameScene->setUpDirection(false); // Unset forward direction
                m_gameScene->togglePause(m_bIsResume);
                m_bIsResume = false;
            }
        }
    }
    else if (ev.code == KEY1_CODE) {
        if (ev.value == 1) {
            int idx = m_gameScene->m_mapIdx;
            
            idx++;

            idx = idx == m_gameScene->m_mapCnt ? 0 : idx;

            m_gameScene->setMapIdx(idx);
        } else {
            //qDebug() << "[ACTION] SW3 deactivated";
        }
    }
}

void InputDeviceHandler::handleAccEvent(const struct input_event &ev)
{
    //qDebug() << "[ACCELEROMETER EVENT] Code:" << ev.code << "Value:" << ev.value;
    int acc_x = 0, acc_y = 0;
    double rotation_angle = 0;
    // 가속도 센서 이벤트 처리
    switch (ev.code) {
        case ABS_X:
            acc_x = ev.value;
            //qDebug() << "[ROTATION ANGLE] :" << rotation_angle << "Value (X,Y):" << acc_x << "," << acc_y;
            break;
        case ABS_Y:
            acc_y = ev.value;
            m_gameScene -> setAngleDirection(acc_y);
            qDebug() << "[ROTATION ANGLE] :" << rotation_angle << "Value (X,Y):" << acc_x << "," << acc_y;
            break;
        default:
            //qDebug() << "Unknown accelerometer event code:" << ev.code;
            break;
    }
}

double InputDeviceHandler::calculateRotationAngleAxixz(int x, int y)
{
    // Z축 회전각을 계산 (3D 공간에서 기울기)
    // 여기서는 Y와 X의 평면을 기준으로 Z축 회전각을 구함
    double angle = atan2(y, x) * 180 / M_PI; // 라디안 값을 도로 변환
    return angle;
}
