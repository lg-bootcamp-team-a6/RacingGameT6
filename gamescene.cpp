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
#include "InputDeviceHandler.h"

#define DEV_NAME "/dev/mydev"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene{parent}, m_game(), m_timer(new QTimer(this)),
      m_upDir(true), m_rightDir(false), m_downDir(false), m_leftDir(false), m_dirChanged(false),
     m_pauseItem(nullptr), m_elapsedTime(0), m_computeTime(0), m_bIsResume(false), m_bReady(false)
{
    m_mapIdx =3;
    m_pUdpSocketHandler = new UdpSocketHandler(this);
    loadPixmap();

    setSceneRect(0, 0, Game::RESOLUTION.width(), Game::RESOLUTION.height());

    m_timer->setInterval(m_game.ITERATION_VALUE);
    connect(m_timer, &QTimer::timeout, this, &GameScene::update);
    m_timer->start(m_game.ITERATION_VALUE);
    //Send map status
    
    char str[20]; // 문자열 크기 20 (64bit + NULL 종료자)
    sprintf(str, "%d", m_mapIdx);  // 숫자를 문자열로 변환
    qDebug() << "Send map status" << str;
    m_pUdpSocketHandler -> BtHsendMessage(MAP_STATUS, str);

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
            }
            break;
        //how many checkpoints 
        case CHECKPOINT:
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
            break;
        //cmd : loser, data : winner's time lap
        case LOSER:
            break;
        default:
            break;
    }


    if (pkt.data) {
        delete[] pkt.data;
    }
}

void GameScene::parseRivalPosition(char* data)
{
    int x = 0, y = 0;
    if (sscanf(data, "%d,%d", &x, &y) == 2) {
        qDebug() << "Parsed x =" << x << ", y =" << y;
    } else {
        qDebug() << "Parsing failed!";
    }
    car[1].x = x;
    car[1].y = y;
}

/* sh) pause function */
void GameScene::togglePause(bool IsResume)
{
    QGraphicsPixmapItem *pauseItem = new QGraphicsPixmapItem(m_pausePixmap);
    pauseItem->setScale(1);
    pauseItem->setPos(0, 0);
    addItem(pauseItem);

    if (!IsResume) {
        m_timer->stop();
        qDebug() << "stop timer success";
        if (nullptr != pauseItem) {
            pauseItem->setVisible(true);
            qDebug() << "[GAME] Paused by an interrupt.";
            SocketUDP();
            //SocketUDP();
            char* data = "PAUSE";
            m_pUdpSocketHandler -> BtHsendMessage(GAME_STATUS, data);
        }
    } else {
        m_timer->start(m_game.ITERATION_VALUE);
        pauseItem->setVisible(false);
        qDebug() << "[GAME] Resumed by an interrupt.";
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

    for(int i = 1; i < m_game.COUNT_OF_CARS; i++)
    {
        m_game.car[i].move();
    }

    for(int i=1; i < m_game.COUNT_OF_CARS ;i++)
    {
        m_game.car[i].findTarget();
    }


}

void GameScene::carCollision()
{

    for(int i = 0; i < Game::COUNT_OF_CARS;i++)
    {
        for(int j=0; j<Game::COUNT_OF_CARS;j++)
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
            textItem3->setPlainText(QString("First : %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0')));
        else if(i == 1)
            textItem3->setPlainText(QString("Second : %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0')));
        else if (i == 2)
            textItem3->setPlainText(QString("Third : %1.%2").arg(seconds, 2, 10, QChar('0')).arg(mseconds, 2, 10, QChar('0')));

        textItem3->setDefaultTextColor(Qt::black);
        textItem3->setFont(QFont("Arial", 15));
        textItem3->setPos(600, 10 + 30* i); // col * row
        addItem(textItem3);
        textItem->setVisible(true);
    }
}

void GameScene::SocketUDP() {
    QString message = "PAUSED";
    char* data = "data";
    m_pUdpSocketHandler -> BtHsendMessage(1, data);
}

void GameScene::Wait3Seconds() {
    m_bReady = true;
    qDebug() << "wait 3 sec .."<< m_bReady;
    QGraphicsPixmapItem *three = new QGraphicsPixmapItem(m_readyPixmap[0]);
    QGraphicsPixmapItem *two = new QGraphicsPixmapItem(m_readyPixmap[1]);
    QGraphicsPixmapItem *one = new QGraphicsPixmapItem(m_readyPixmap[2]);
    m_timer->stop();

    if (nullptr != three && nullptr != two && nullptr != one) {
        qDebug() << "print 321...";
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
    clear();

    for(int i = 0; i < m_mapCnt; i ++)
        m_bgItem[i] = new QGraphicsPixmapItem(m_bgPixmap[i]);

    for(int i = 0; i < Game::COUNTING_STARS; i ++)
        m_starItem[i] = new QGraphicsPixmapItem(m_starPixmap[0]);

    for (int i = 0; i < Game::COUNT_OF_CARS; ++i)
        m_carItem[i] = new QGraphicsPixmapItem(m_carPixmap[i]);

    m_bgItem[m_mapIdx]->setScale(m_game.gamescale);
    m_game.offsetX = m_game.car[0].x-160 * m_game.gamescale;
    m_game.offsetY = m_game.car[0].y-120 * m_game.gamescale;
    m_bgItem[m_mapIdx]->setPos(-m_game.offsetX, -m_game.offsetY);

     addItem(m_bgItem[m_mapIdx]);

    carMovement();
    carCollision();
    checkStarCollision();

    if(m_game.m_starScore == Game::COUNTING_STARS)
         Goal();

    for (int i = m_game.m_starScore; i < Game::COUNTING_STARS; ++i) {
        m_starItem[i]->setScale(1);
        m_starItem[i]->setPos(Game::m_checkpoint[m_mapIdx][i][0] * m_game.gamescale - m_game.offsetX,Game::m_checkpoint[m_mapIdx][i][1] * m_game.gamescale - m_game.offsetY);
        addItem(m_starItem[i]);
    }

    for(int i=0; i < Game::COUNT_OF_CARS; i++)
    {
        m_carItem[i]->setScale(0.7);
        m_carItem[i]->setTransformOriginPoint(21, 34);
        m_carItem[i]->setPos(m_game.car[i].x - m_game.offsetX, m_game.car[i].y - m_game.offsetY);
        m_carItem[i]->setRotation(m_game.car[i].angle * 180/3.141593);
        addItem(m_carItem[i]);
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
    m_mapIdx = mapIdx;
    m_game.resetGameData(mapIdx);
    m_elapsedTime = 0;
    update();
    Wait3Seconds();
    char str[20]; // 문자열 크기 20 (64bit + NULL 종료자)
    sprintf(str, "%d", m_mapIdx);  // 숫자를 문자열로 변환
    qDebug() << "Send map status in changed mode" << str;
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
                AudioHandler::getInstance()->playEffectSound("star_sound.wav");
                break;
            }
        }
    } while (false);

    return bReturn;
}

void GameScene::Goal()
{
    int idx = m_mapIdx;

    idx++;

    idx = idx == m_mapCnt ? 0 : idx;

    m_game.m_rankRecord[m_mapIdx].append(m_elapsedTime);
    std::sort(m_game.m_rankRecord[m_mapIdx].begin(), m_game.m_rankRecord[m_mapIdx].end());
    //Send elapsedTime
    int seconds = m_elapsedTime / 100;
    int mseconds = m_elapsedTime % 100;

    //qDebug() << "Goal 1 ! ";
    char str[20]; // 문자열 크기 20 (64bit + NULL 종료자)
    sprintf(str, "%d.%d", seconds,mseconds);  // 숫자를 문자열로 변환
    //qDebug() << "Goal 2 ! ";
    qDebug() << "The elapsedTime is " << str;
    m_pUdpSocketHandler -> BtHsendMessage(FINISH, str);

    //Display Finish in solo play
    QGraphicsPixmapItem *fin = new QGraphicsPixmapItem(m_finishPixmap);

    if (nullptr != fin) {
        fin->setScale(0.7);
        fin->setPos(-40, 15);
        addItem(fin);
        fin->setVisible(true);
        }

        InputDeviceHandler::m_sbIsRetry = true;
        
        m_timer->stop();
        // removeItem(fin);
        // delete fin;
}
