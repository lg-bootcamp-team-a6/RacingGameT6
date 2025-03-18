#include "gamescene.h"
#include "AudioHandler.h"
#include <QTimer>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QDir>
#include <QPainter>
#include <cmath>
#include <QGraphicsView>
#include <QUdpSocket>
#include <unistd.h>
#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QWidget>
#include "InputDeviceHandler.h"

#define DEV_NAME "/dev/mydev"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene{parent}, m_game(), m_timer(new QTimer(this)),
      m_upDir(true), m_rightDir(false), m_downDir(false), m_leftDir(false), m_dirChanged(false),
     m_pauseItem(nullptr), m_elapsedTime(0), m_computeTime(0), m_bIsResume(false), m_bReady(false),
     m_audioHandler(AudioHandler::getInstance())
{
    m_mapIdx =3;
    m_pUdpSocketHandler = new UdpSocketHandler(this);
    loadPixmap();

    setSceneRect(0, 0, Game::RESOLUTION.width(), Game::RESOLUTION.height());

    QPixmap startPixmap;
    if(startPixmap.load(m_game.PATH_TO_START_PIXMAP))
    {
        qDebug() << "startPixmap is loaded successfully";
    }
    QGraphicsPixmapItem *start = new QGraphicsPixmapItem(startPixmap);

    if (nullptr != start) {
        start->setScale(0.7);
        start->setPos(-40, 15);
        addItem(start);
        start->setVisible(true);
    }

    m_timer->setInterval(m_game.ITERATION_VALUE);
    connect(m_timer, &QTimer::timeout, this, &GameScene::update);
    m_audioHandler->playAudio();
    m_timer->start(m_game.ITERATION_VALUE);


    // 여기서 receiveMessage() blocking 호출 대신, 별도의 worker를 사용합니다.
    // 먼저, receive_packet 타입을 Qt 메타 타입 시스템에 등록
    qRegisterMetaType<receive_packet>("receive_packet");

    // UdpReceiverWorker 생성: 포트는 SERVER_PORT 사용 (define.h에 설정되어 있어야 함)
    UdpReceiverWorker* receiverWorker = new UdpReceiverWorker(SERVER_PORT);
    QThread* receiverThread = new QThread(this);
    receiverWorker->moveToThread(receiverThread);

    // 스레드 시작 시 worker의 process() 슬롯 실행
    connect(receiverThread, &QThread::started, receiverWorker, &UdpReceiverWorker::process);

    // worker가 보낸 packetReceived 시그널을 GameScene의 슬롯(handleUdpPacket)과 연결
    connect(receiverWorker, &UdpReceiverWorker::packetReceived,
            this, &GameScene::handleUdpPacket, Qt::QueuedConnection);

    // 스레드 종료 시 worker 및 스레드 정리
    connect(receiverThread, &QThread::finished, receiverWorker, &QObject::deleteLater);
    connect(receiverThread, &QThread::finished, receiverThread, &QObject::deleteLater);

    receiverThread->start();

    //Send map status
    char str[20]; // 문자열 크기 20 (64bit + NULL 종료자)
    sprintf(str, "%d", m_mapIdx);  // 숫자를 문자열로 변환
    qDebug() << "Send map status" << str;
    m_pUdpSocketHandler -> BtHsendMessage(MAP_STATUS, str);

    update();
}

void GameScene::handleUdpPacket(const receive_packet &pkt)
{
    qDebug() << "[GameScene] UDP packet received: cmd =" << pkt.cmd
             << ", data =" << (pkt.data ? pkt.data : "null");
    
    // 예: 패킷 내용에 따라 게임 상태를 업데이트하거나, 필요한 로직을 수행합니다.
    // 사용 후 동적으로 할당된 메모리는 해제해 주세요.
    switch(pkt.cmd)
    {
        //START, PAUSE
        case GAME_STATUS:
            qDebug() << "START";

            if(!strcmp(pkt.data,"START"))
            {
                qDebug() << "START setMapIdx";
                setMapIdx(m_mapIdx);
                m_bConnect = true;
                m_carCnt = 2;
            }
            break;
        //how many checkpoints 
        case CHECKPOINT:
            m_rivalScore = atoi(pkt.data);
            
            break;
        case CAR_POSITION:
            parseRivalPosition(pkt.data);
            break;
        case FINISH:
            break;
        case MAP_STATUS:
            break;
        //cmd : winner, data : time lap
        case WINNER: 
            qDebug() << "WINNER";
            m_rivalScore = 0;
            m_carCnt = 1;
            m_bConnect = false;
            FinishRace(true, pkt.data);
            break;
        
        //cmd : loser, data : winner's time lap
        case LOSER: 
            qDebug() << "LOOSER";
            m_rivalScore = 0;
            m_carCnt = 1;
            m_bConnect = false;
            FinishRace(false, pkt.data);
            break;

        case IP_ADDRESS:
            parseMyIp(pkt.data);
            break;
        default:
            break;
    }


    if (pkt.data) {
        delete[] pkt.data;
    }
}

void GameScene::FinishRace(bool win, char *pszTime) {
    if (win) {
        qDebug() << "dual finished, lap time: " << pszTime << ")";
        QGraphicsPixmapItem *win = new QGraphicsPixmapItem(m_winPixmap);

        if (nullptr != win) {
            win->setScale(1.15);
            win->setPos(-35, 0);
            addItem(win);
            win->setVisible(true);
        }

        QGraphicsTextItem* record = new QGraphicsTextItem();

        QString recordText;
        recordText = pszTime;

        record->setPlainText(QString("%1").arg(recordText));
        record->setDefaultTextColor(Qt::black);
        record->setFont(QFont("D2Coding", 20, QFont::Bold));
        record->setPos(450, 165); // col * row
        addItem(record);
        record->setVisible(true);
    } else {
        qDebug() << "dual finished, lap time: " << pszTime << ")";
        QGraphicsPixmapItem *lose = new QGraphicsPixmapItem(m_losePixmap);

        if (nullptr != lose) {
            lose->setScale(1.15);
            lose->setPos(-35, 0);
            addItem(lose);
            lose->setVisible(true);
        }

        QGraphicsTextItem* record = new QGraphicsTextItem();

        QString recordText;
        recordText = pszTime;

        record->setPlainText(QString("%1").arg(recordText));
        record->setDefaultTextColor(Qt::white);
        record->setFont(QFont("D2Coding", 30, QFont::Bold));
        record->setPos(450, 165); // col * row
        addItem(record);
        record->setVisible(true);

        m_timer->stop();
    }
}

void GameScene::parseMyIp(char* data)
{
    printf("%s\n", data);
    
    if(!strcmp(data,"192.168.10.4"))
    {
        m_myIp = 4;
        
        if(m_carPixmap[0].load(m_game.PATH_TO_CAR_PIXMAP[1]))
        {
            qDebug() << "CarPixmap[0] is loaded successfully";
        }

        if(m_carPixmap[1].load(m_game.PATH_TO_CAR_PIXMAP[0]))
        {
            qDebug() << "CarPixmap[0] is loaded successfully";
        }
    }
}

void GameScene::parseRivalPosition(char* data)
{
    float x = 0, y = 0, angle = 0;
    
    qDebug() << "data : " << data;
    
    // 접두사를 건너뛰기 위해 콜론(:)을 찾습니다.
    const char* p = strchr(data, ':');
    if (p != nullptr) {
        p++; // 콜론 다음 문자로 이동
        // 필요하면 공백도 건너뜁니다.
        while (*p == ' ') {
            p++;
        }
        if (sscanf(p, "%f,%f,%f", &x, &y, &angle) == 3) {
            qDebug() << "Parsed x =" << x << ", y =" << y <<", angle = " << angle;
        } else {
            qDebug() << "Parsing failed!!!!!!";
        }
    } else {
        qDebug() << "No colon found in data.";
    }
    
    m_game.car[1].x = x;
    m_game.car[1].y = y;
    m_game.car[1].angle = angle;
}

/* sh) pause function */
void GameScene::togglePause(bool IsResume)
{
    QGraphicsPixmapItem *pauseItem = new QGraphicsPixmapItem(m_pausePixmap);
        QWidget *pauseWidget = new QWidget();
    pauseWidget->setFixedSize(609, 473);  // 위젯 크기 설정 (배경 이미지 크기)
    pauseWidget->setStyleSheet("background: transparent;");
    pauseWidget->setAttribute(Qt::WA_TranslucentBackground);

    QLabel *backgroundLabel = new QLabel(pauseWidget);
    QPixmap backgroundPixmap(":/images/pause.png");  // 배경 이미지 경로 설정
    backgroundLabel->setPixmap(backgroundPixmap);
    backgroundLabel->setScaledContents(true);  // 크기에 맞게 이미지를 자동으로 조정
    backgroundLabel->setGeometry(0, 0, backgroundPixmap.width(), backgroundPixmap.height());
    backgroundLabel->setStyleSheet("background: transparent;");
    
    /* Audio Button */
    m_audioButton = new QPushButton(pauseWidget);
    m_audioButton->setFixedSize(232, 72);
    m_audioButton->setIcon(QIcon(":/images/on.png"));  // default: on 상태
    m_audioButton->setIconSize(QSize(232, 72));
    m_audioButton->setStyleSheet("border: none; background:transparent;");
    m_audioButton->setFocusPolicy(Qt::NoFocus);
    m_audioButton->setAutoRepeat(false);
    m_audioButton->setVisible(false);

    /* Audio Change Button */
    m_audioChangeButton = new QPushButton(pauseWidget);
    m_audioButton->setFixedSize(249, 59);
    // 초기값 설정 (첫 번째 음악 재생되도록)
    QString currentTrack = m_audioHandler->getCurrentTrack();
    qDebug() << __FUNCTION__ << "Initial Track: " << currentTrack;
    m_audioChangeButton->setIcon(QIcon(":/images/cookie.png"));
    m_audioChangeButton->setIconSize(QSize(249, 59));
    m_audioChangeButton->setStyleSheet("border: none; background:transparent;");
    m_audioChangeButton->setFocusPolicy(Qt::NoFocus);
    m_audioChangeButton->setAutoRepeat(false);
    m_audioChangeButton->setVisible(false);
    pauseItem->setScale(1);
    pauseItem->setPos(0, 0);
    addItem(pauseItem);

    if (!IsResume) {
        m_timer->stop();
        qDebug() << "stop timer success";
        if (nullptr != pauseItem) {
                        // pauseItem->setVisible(true);
            m_audioButton->setVisible(true);
            m_audioChangeButton->setVisible(true);

            /* Button position set */
            qreal audioButtonX = 295;  // 원하는 X 위치
            qreal audioButtonY = 200;  // 원하는 Y 위치
            m_audioButton->setGeometry(audioButtonX, audioButtonY, 232, 72);

            qreal audioChangeButtonX = 310;  // 원하는 X 위치
            qreal audioChangeButtonY = 275;  // 원하는 Y 위치
            m_audioChangeButton->setGeometry(audioChangeButtonX, audioChangeButtonY, 249, 59);

            // QWidget을 QGraphicsProxyWidget에 추가
            QGraphicsProxyWidget *pauseWidgetProxy = new QGraphicsProxyWidget();
            pauseWidgetProxy->setWidget(pauseWidget);
            addItem(pauseWidgetProxy);

            connect(m_audioButton, &QPushButton::clicked, this, &GameScene::toggleAudioStatus);
            connect(m_audioChangeButton, &QPushButton::clicked, this, &GameScene::changeAudio);

            qDebug() << "[GAME] Paused by an interrupt.";
            //SocketUDP();
            //SocketUDP();
            char* data = "PAUSE";
            m_pUdpSocketHandler -> BtHsendMessage(GAME_STATUS, data);
        }
    } else {
        m_timer->start(m_game.ITERATION_VALUE);
        m_audioButton->setVisible(false);
        m_audioChangeButton->setVisible(false);
        qDebug() << __FUNCTION__ << "[GAME] Resumed by an interrupt.";
        m_bIsResume = true;
    }
}

void GameScene::loadPixmap()
{
    if(m_bgPixmap[0].load(m_game.PATH_TO_BACKGROUND_PIXMAP[0]))
    {
        qDebug() << "BgPixmap0 is loaded successfully";
    }
    else
    {
        qDebug() << "BgPixmap0 is not loaded successfully";
    }

    if(m_bgPixmap[1].load(m_game.PATH_TO_BACKGROUND_PIXMAP[1]))
    {
        qDebug() << "BgPixmap1 is loaded successfully";
    }
    else
    {
        qDebug() << "BgPixmap1 is not loaded successfully";
    }


    if(m_bgPixmap[2].load(m_game.PATH_TO_BACKGROUND_PIXMAP[2]))
    {
        qDebug() << "BgPixmap2 is loaded successfully";
    }
    else
    {
        qDebug() << "BgPixmap2 is not loaded successfully";
    }

    if(m_bgPixmap[3].load(m_game.PATH_TO_BACKGROUND_PIXMAP[3]))
    {
        qDebug() << "BgPixmap3 is loaded successfully";
    }
    else
    {
        qDebug() << "BgPixmap3 is not loaded successfully";
    }


    if(m_carPixmap[0].load(m_game.PATH_TO_CAR_PIXMAP[0]))
    {
        qDebug() << "CarPixmap[0] is loaded successfully";
    }

    if(m_carPixmap[1].load(m_game.PATH_TO_CAR_PIXMAP[1]))
    {
        qDebug() << "CarPixmap[0] is loaded successfully";
    }

    if(m_starPixmap[0].load(m_game.PATH_TO_STAR_PIXMAP[0]))
    {
        qDebug() << "StarPixmap[0] is loaded successfully";
    }

    if(m_readyPixmap[0].load(m_game.PATH_TO_COUNT_PIXMAP[2]))
    {
        qDebug() << "ReadyPixmap[0] is loaded successfully";
    }

    if(m_readyPixmap[1].load(m_game.PATH_TO_COUNT_PIXMAP[1]))
    {
        qDebug() << "ReadyPixmap[1] is loaded successfully";
    }

    if(m_readyPixmap[2].load(m_game.PATH_TO_COUNT_PIXMAP[0]))
    {
        qDebug() << "ReadyPixmap[2] is loaded successfully";
    }

    if(m_pausePixmap.load(m_game.PATH_TO_PAUSE_PIXMAP))
    {
        qDebug() << "PausePixmap is loaded successfully";
    }

    if(m_finishPixmap.load(m_game.PATH_TO_FINISH_PIXMAP))
    {
        qDebug() << "FinishPixmap is loaded successfully";
    }

    if(m_winPixmap.load(m_game.PATH_TO_WIN_PIXMAP))
    {
        qDebug() << "Winixmap is loaded successfully";
    }

    if(m_losePixmap.load(m_game.PATH_TO_LOSE_PIXMAP))
    {
        qDebug() << "LosePixmap is loaded successfully";
    }
}

void GameScene::carMovement()
{
    if (m_upDir && m_game.speed < m_game.maxSpeed)
    {
        if (m_game.speed < 0)
        {
            m_game.speed += m_game.dec;
        }
        else
        {
            m_game.speed += m_game.acc;
        }
    }

    if (m_downDir && m_game.speed > -m_game.maxSpeed)
    {
        if (m_game.speed > 0)
        {
            m_game.speed -= m_game.dec;
        }
        else
        {
            m_game.speed -= m_game.acc;
        }
    }


    if (!m_upDir && !m_downDir)
    {
        if (m_game.speed - m_game.dec > 0)
        {
            m_game.speed -= m_game.dec;
        }
        else if (m_game.speed + m_game.dec < 0)
        {
            m_game.speed += m_game.dec;
        }
        else
        {
            m_game.speed = 0;
        }
    }


    if (m_rightDir && m_game.speed!=0)
    {
        m_game.angle += m_game.turnSpeed * m_game.speed/m_game.maxSpeed;
    }

    if (m_leftDir && m_game.speed!=0)
    {
        m_game.angle -= m_game.turnSpeed * m_game.speed/m_game.maxSpeed;
    }


    QImage bgImage = m_bgPixmap[m_mapIdx].toImage();

    m_game.car[0].speed = m_game.speed;
    m_game.car[0].angle = m_game.angle;


    // Get the pixel value at the car's position
    QRgb pixelValue = bgImage.pixel(m_game.car[0].x/m_game.gamescale, m_game.car[0].y/m_game.gamescale);

    // Extract RGB components
     int red = qRed(pixelValue);
     int green = qGreen(pixelValue);
     int blue = qBlue(pixelValue);

    int newX = m_game.car[0].x + sin(m_game.car[0].angle) * m_game.car[0].speed;
    int newY = m_game.car[0].y - cos(m_game.car[0].angle) * m_game.car[0].speed;

    int pixelX = newX / m_game.gamescale;
    int pixelY = newY / m_game.gamescale;


    // Get the pixel value at the car's position
     pixelValue = bgImage.pixel(pixelX, pixelY);

    // Extract RGB components
     red = qRed(pixelValue);
     green = qGreen(pixelValue);
     blue = qBlue(pixelValue);


    if(red == m_game.m_pixelRed && green == m_game.m_pixelGreen && blue == m_game.m_pixelBlue)
        m_game.car[0].move();
    else
    {
        int dx=0, dy=0;
        while (dx*dx + dy*dy < m_game.car_R*m_game.car_R)
        {
            m_game.car[0].x -= dx/10.0;
            m_game.car[0].y -= dy/10.0;

            dx = newX - m_game.car[0].x;
            dy = newY - m_game.car[0].y;

            if (!dx && !dy)
            {
                break;
            }
        }
    }

    m_game.car[0].findTarget();

}

void GameScene::carCollision()
{

    for(int i = 0; i < m_carCnt; i++)
    {
        for(int j=0; j < m_carCnt; j++)
        {
            int dx=0, dy=0;
            while (dx*dx + dy*dy < 4* m_game.car_R*m_game.car_R)
             {
               m_game.car[i].x += dx/10.0;
               m_game.car[i].x += dy/10.0;
               m_game.car[j].x -= dx/10.0;
               m_game.car[j].y -= dy/10.0;
               dx = m_game.car[i].x - m_game.car[j].x;
               dy = m_game.car[i].y - m_game.car[j].y;
               if (!dx && !dy)
               {
                   break;
               }
             }
        }
    }
}

void GameScene::renderScene()
{
    static int index = 0;
    QString fileName = QDir::currentPath() + QDir::separator() + "screen" + QString::number(index++) + ".png";
    QRect rect = sceneRect().toAlignedRect();
    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    render(&painter);
    image.save(fileName);
    qDebug() << "saved " << fileName;
}


void GameScene::showText() {
    m_elapsedTime += 9;
    int seconds = m_elapsedTime / 100;
    int mseconds = m_elapsedTime % 100;
    QString timeText = QString("Time: %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0'));
    // QGraphicsTextItem을 사용하여 주행 시간 표시
    QGraphicsTextItem* textItem = new QGraphicsTextItem();
    textItem->setPlainText(timeText);  // Time format: "seconds.miliseconds"
    textItem->setDefaultTextColor(Qt::black);
    textItem->setFont(QFont("Arial", 15));
    textItem->setPos(600, -20); // hard coding..
    addItem(textItem);
    textItem->setVisible(true);

    QGraphicsTextItem* textItem2 = new QGraphicsTextItem();
    textItem2->setPlainText(QString("Speed: %1 | Angle: %2").arg(m_game.speed).arg(m_game.angle));
    textItem2->setDefaultTextColor(Qt::black);
    textItem2->setFont(QFont("Arial", 15));
    textItem2->setPos(600, -50); // col * row
    addItem(textItem2);
    textItem->setVisible(true);

    for(int i = 0; i < m_game.m_rankRecord[m_mapIdx].size() && i < 3; i++)
    {
        QGraphicsTextItem* textItem3 = new QGraphicsTextItem();
        
        int seconds = m_game.m_rankRecord[m_mapIdx][i] / 100;
        int mseconds = m_game.m_rankRecord[m_mapIdx][i] % 100;

        if(i == 0)
            textItem3->setPlainText(QString("1st : %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0')));
        else if(i == 1)
            textItem3->setPlainText(QString("2nd : %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0')));
        else if (i == 2)
            textItem3->setPlainText(QString("3rd : %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0')));

        textItem3->setDefaultTextColor(Qt::black);
        textItem3->setFont(QFont("Arial", 15));
        textItem3->setPos(600, 10 + 30* i); // col * row
        addItem(textItem3);
        textItem->setVisible(true);
    }

    if(m_rivalScore)
    {
        QGraphicsTextItem* textItem4 = new QGraphicsTextItem();
        textItem4->setPlainText(QString("Rival's Score : %1").arg(m_rivalScore));
        textItem4->setDefaultTextColor(Qt::black);
        textItem4->setFont(QFont("Arial", 15));
        textItem4->setPos(0, -50); // col * row
        addItem(textItem4);
        textItem->setVisible(true);
    }
}

void GameScene::SocketUDP(const int16_t cmd, const char* data) {
    m_pUdpSocketHandler -> BtHsendMessage(cmd, data);
}

void GameScene::Wait3Seconds() {
    m_bReady = true;
    QGraphicsPixmapItem *three = new QGraphicsPixmapItem(m_readyPixmap[0]);
    QGraphicsPixmapItem *two = new QGraphicsPixmapItem(m_readyPixmap[1]);
    QGraphicsPixmapItem *one = new QGraphicsPixmapItem(m_readyPixmap[2]);
    m_timer->stop();

    if (nullptr != three && nullptr != two && nullptr != one) {
        qDebug() << "print 321...";
        m_audioHandler->playEffectSound("start_sound.wav"); /* Play SoundEffect */
        three->setScale(1);
        three->setPos(15, 30);
        three->setVisible(true);
        two->setScale(1);
        two->setPos(15, 30);
        two->setVisible(false);
        one->setScale(1);
        one->setPos(15, 30);
        one->setVisible(false);
        addItem(three);
        addItem(two);
        addItem(one);

        QTimer::singleShot(1000, this, [=]() {
            three->setVisible(false);
            removeItem(three);
            delete three;
            two->setVisible(true);
        });

        m_timer->stop();
        QTimer::singleShot(2000, this, [=]() {
            two->setVisible(false);
            removeItem(two);
            delete two;
            one->setVisible(true);
        });

        m_timer->stop();
        QTimer::singleShot(3000, this, [=]() {
            m_timer->start(m_game.ITERATION_VALUE);  // timer restart
            one->setVisible(false);
            removeItem(one);
            m_bReady = false;
            InputDeviceHandler::m_sbIsResume = false;
            delete one;
        });
    }
}

void GameScene::update()
{
    if (!m_bStart) {
        // qDebug() << "before set mode";
        return;
    }

    clear();

    if(m_game.m_starScore == Game::COUNTING_STARS) {
        Goal();
        return;
    }

    for(int i = 0; i < m_mapCnt; i ++)
        m_bgItem[i] = new QGraphicsPixmapItem(m_bgPixmap[i]);

    for(int i = 0; i < Game::COUNTING_STARS; i ++)
        m_starItem[i] = new QGraphicsPixmapItem(m_starPixmap[0]);

    for (int i = 0; i < m_carCnt; ++i)
        m_carItem[i] = new QGraphicsPixmapItem(m_carPixmap[i]);

    m_bgItem[m_mapIdx]->setScale(m_game.gamescale);
    m_game.offsetX = m_game.car[0].x-160 * m_game.gamescale;
    m_game.offsetY = m_game.car[0].y-120 * m_game.gamescale;
    m_bgItem[m_mapIdx]->setPos(-m_game.offsetX, -m_game.offsetY);

    addItem(m_bgItem[m_mapIdx]);

    carMovement();
    carCollision();
    checkStarCollision();

    for (int i = m_game.m_starScore; i < Game::COUNTING_STARS; ++i) {
        m_starItem[i]->setScale(1);
        m_starItem[i]->setPos(Game::m_checkpoint[m_mapIdx][i][0] * m_game.gamescale - m_game.offsetX,Game::m_checkpoint[m_mapIdx][i][1] * m_game.gamescale - m_game.offsetY);
        addItem(m_starItem[i]);
    }

    for(int i=0; i < m_carCnt; i++)
    {
        m_carItem[i]->setScale(0.7);
        m_carItem[i]->setTransformOriginPoint(21, 34);
        m_carItem[i]->setPos(m_game.car[i].x - m_game.offsetX, m_game.car[i].y - m_game.offsetY);
        m_carItem[i]->setRotation(m_game.car[i].angle * 180/3.141593);

        addItem(m_carItem[i]);
    }

    if(m_bConnect)
    {
        char message[100];
        qDebug() << m_game.car[0].x << " " << m_game.car[0].y ;
        snprintf(message, sizeof(message), "%f,%f,%f", m_game.car[0].x, m_game.car[0].y, m_game.car[0].angle);
        printf("%s\n",message);
        m_pUdpSocketHandler -> BtHsendMessage(CAR_POSITION, message);
    }

    if (m_bIsResume) {
        m_bIsResume = false;
        Wait3Seconds();
    }

    showText();
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
    {
        m_upDir = true;
    }
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
    {
        m_rightDir = true;
    }
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
    {
        m_downDir = true;
    }
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
    {
        m_leftDir = true;
    }
        break;
    case Qt::Key_Z:
    {
        renderScene();
    }
        break;
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
    {
        m_upDir = false;
    }
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
    {
        m_rightDir = false;
    }
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
    {
        m_downDir = false;
    }
        break;
    case Qt::Key_A:
    case Qt::Key_Left:
    {
        m_leftDir = false;
    }
        break;
    }
    QGraphicsScene::keyReleaseEvent(event);
}

bool GameScene::getDirectionChanged()
{
    return m_dirChanged;
}

void GameScene::setUpDirection(bool upDir)
{
    m_upDir = upDir;
}

void GameScene::setRightDirection(bool rightDir)
{
    m_rightDir = rightDir;
}

void GameScene::setDownDirection(bool downDir)
{
    m_downDir = downDir;
}



void GameScene::setAngleDirection(double angle)
{

    bool previousLeftDir = m_leftDir;
    bool previousRightDir = m_rightDir;

    if(abs(angle) < 200){
        //qDebug()<<"##########[Staight]###########################";
        m_leftDir = false;
        m_rightDir = false;

    }
    else if(angle < 0){
        //qDebug()<<"!!!!!!!!!!!!!!!!!!!![Right]!!!!!!!!!!!!!!!!!!!!";
        m_leftDir = false;
        m_rightDir = true;
    }
    else{
        //qDebug()<<"@@@@@@@@@@@@@@@@@@@@[LEFT]@@@@@@@@@@@@@@@@@";
        m_leftDir = true;
        m_rightDir = false;
    }

    if (m_leftDir != previousLeftDir || m_rightDir != previousRightDir) {
        m_dirChanged = true;  // 방향이 변경되었으면 true
        //qDebug()<<"-------------Changed direction--------------";

    } else {
        m_dirChanged = false; // 변경되지 않으면 false
    }

}

void GameScene::setMapIdx(int mapIdx)
{
    bool bSend = true;

    if(m_mapIdx == mapIdx)
        bSend = false;

    m_mapIdx = mapIdx;
    m_bConnect = false;
    m_carCnt = 1;
    m_game.resetGameData(mapIdx);

    if(m_myIp == 4)
    {
        if(mapIdx == 0)
        {
            m_game.car[0].x = 320 * m_game.gamescale;
            m_game.car[0].y = 1500 * m_game.gamescale;
        }
        else if (mapIdx == 1)
        {
            m_game.car[0].x = 320 * m_game.gamescale;
            m_game.car[0].y = 1300 * m_game.gamescale;
        }
        else if (mapIdx == 2)
        {
            m_game.car[0].x = 270 * m_game.gamescale;
            m_game.car[0].y = 600 * m_game.gamescale;
        }
        else if (mapIdx == 3)
        {
            m_game.car[0].x = 200 * m_game.gamescale;
            m_game.car[0].y = 2400 * m_game.gamescale;
        }
    }

    m_elapsedTime = 0;
    update();
    Wait3Seconds();
    char str[20]; // 문자열 크기 20 (64bit + NULL 종료자)
    sprintf(str, "%d", m_mapIdx);  // 숫자를 문자열로 변환
    qDebug() << "Send map status in changed mode" << str;

    if(bSend)
        m_pUdpSocketHandler -> BtHsendMessage(MAP_STATUS, str);
}

void GameScene::resetGame() {
    m_game.resetGameData(m_mapIdx);
    m_elapsedTime = 0;
    update();
    Wait3Seconds();
}

bool GameScene::checkStarCollision()
{
    bool bReturn = false;
    int i32Range = 100;
    int i32CarX = m_game.car[0].x;
    int i32CarY = m_game.car[0].y;

    do
    {
        if(m_game.m_starScore >= Game::COUNTING_STARS)
            break;

        int i32StarX = Game::m_checkpoint[m_mapIdx][m_game.m_starScore][0]* m_game.gamescale;
        int i32StarY = Game::m_checkpoint[m_mapIdx][m_game.m_starScore][1]* m_game.gamescale;

        if(i32CarX > i32StarX - i32Range && i32CarX < i32StarX + i32Range)
        {
            if(i32CarY > i32StarY - i32Range && i32CarY < i32StarY + i32Range)
            {
                bReturn = true;
                m_game.m_starScore++;
                m_audioHandler->playEffectSound("star_sound.wav");
                //send checkpoint to Server
                char data[3];  // 30까지의 숫자를 문자열로 표현하기 위해 충분히 큰 배열 크기 (최대 3자리 숫자)
                sprintf(data, "%d", m_game.m_starScore);  // 숫자를 문자열로 변환
                m_pUdpSocketHandler -> BtHsendMessage(CHECKPOINT, data);
                break;
            }
        }
    } while (false);

    return bReturn;
}

void GameScene::Goal()
{
    m_game.m_rankRecord[m_mapIdx].append(m_elapsedTime);
    std::sort(m_game.m_rankRecord[m_mapIdx].begin(), m_game.m_rankRecord[m_mapIdx].end());
    int seconds = m_elapsedTime / 100;
    int mseconds = m_elapsedTime % 100;

    char str[20]; // 문자열 크기 20 (64bit + NULL 종료자)
    sprintf(str, "%d.%d", seconds,mseconds);  // 숫자를 문자열로 변환
    qDebug() << "The elapsedTime is " << str;
    m_pUdpSocketHandler -> BtHsendMessage(FINISH, str);

    //Display Finish in solo play
    if (m_bSingle) {
        qDebug() << "round finished (single mode: " << m_bSingle << ")";
        QGraphicsPixmapItem *fin = new QGraphicsPixmapItem(m_finishPixmap);

        if (nullptr != fin) {
            fin->setScale(1.15);
            fin->setPos(-35, 0);
            addItem(fin);
            fin->setVisible(true);
            }

        QStringList rankNames = {"1st", "2nd", "3rd", "4th", "5th"};
        int tmpX = 0;
        for(int i = 0; i < 5; i++)
        {
            QGraphicsTextItem* textItem3 = new QGraphicsTextItem();

            QString timeText;
            if (i < m_game.m_rankRecord[m_mapIdx].size()) {
                int seconds = m_game.m_rankRecord[m_mapIdx][i] / 100;
                int mseconds = m_game.m_rankRecord[m_mapIdx][i] % 100;
                timeText = QString("%1.%2")
                        .arg(seconds, 2, 10, QChar('0'))
                        .arg(mseconds, 2, 10, QChar('0'));
            } else {
                timeText = "--.--";  // if no lap time
            }

            textItem3->setPlainText(QString("%1 : %2").arg(rankNames[i]).arg(timeText));
            textItem3->setDefaultTextColor(Qt::white);
            textItem3->setFont(QFont("D2Coding", 20, QFont::Bold));
            textItem3->setPos(250, 133 + 40 * i); // col * row
            addItem(textItem3);
            textItem3->setVisible(true);
        }
        InputDeviceHandler::m_sbIsRetry = true;
    } else {
        // FinishRace(false, "12:43"); // todo) remove this code
    }

    m_timer->stop();
}


/* Sound ON/OFF */
void GameScene::toggleAudioStatus() {
    // 현재 오디오 상태 및 재생 중인 트랙 확인
    bool isAudioOn = m_audioHandler->isAudioOn();
    qDebug() << __FUNCTION__ << ": Audio status: " << isAudioOn;
    const QMap<QString, AudioData>& audioMap = m_audioHandler->getAudioMap();

    if (isAudioOn) {    // play -> stop
        m_audioHandler->stopAudio();
        m_audioButton->setIcon(QIcon(":/images/off.png"));
    } else {            // stop -> play
        m_audioButton->setIcon(QIcon(":/images/on.png"));
        QString initialTrack = "cookie";
        m_audioHandler->setCurrentTrack(initialTrack);
        m_audioHandler->playAudio();
        /* apply music info to select button */
        m_audioChangeButton->setIcon(QIcon(":/images/cookie.png"));
        qDebug() << "stop >> play: icon name is " << QIcon(audioMap[initialTrack].iconPath);
        return;
    }

    // 오디오 상태 토글 후 저장
    qDebug() << __FUNCTION__ << "[Audio Status] Audio is " << (!isAudioOn ? "on" : "off") << "@@@@@@@@@@@@@@";
}

/* Sound Change */
void GameScene::changeAudio() {
    if (!m_audioHandler->isAudioOn()) {
        qDebug() << __FUNCTION__ << "OFF";
        return;
    }

    // playNextTrack 호출하여 트랙과 아이콘 경로 받기
    auto result = m_audioHandler->playNextTrack();
    qDebug() << __FUNCTION__ << "result: " << result;
    QString nextTrack = result.first;   // 다음 트랙 이름
    QString iconPath = result.second;   // 아이콘 경로

    if (nextTrack.isEmpty() || iconPath.isEmpty()) {
        qDebug() << __FUNCTION__ << ": @@@@@@@@@@ Track && Icon Empty @@@@@@@@@@@";
        return;
    }

    QIcon newIcon(iconPath);
    m_audioChangeButton->setIcon(newIcon);

    qDebug() << __FUNCTION__ << "##### Changed background audio to: " << nextTrack;
}