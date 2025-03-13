#include "gamescene.h"
#include <QTimer>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include <QKeyEvent>
#include <QDir>
#include <QPainter>
#include <cmath>
#define DEV_NAME "/dev/mydev"

GameScene::GameScene(QObject *parent)
    : QGraphicsScene{parent}, m_game(), m_timer(new QTimer(this)),
      m_upDir(false), m_rightDir(false), m_downDir(false), m_leftDir(false)
{
    
    loadPixmap();
    setSceneRect(0, 0, Game::RESOLUTION.width(), Game::RESOLUTION.height());
    connect(m_timer, &QTimer::timeout, this, &GameScene::update);

    m_bgItem = new QGraphicsPixmapItem(m_bgPixmap[m_mapIdx]);
    //m_bgItem->setTransformationMode(Qt::SmoothTransformation);
    m_bgItem->setScale(m_game.gamescale);
    addItem(m_bgItem);

    for (int i = 0; i < Game::COUNTING_STARS; ++i) {
        QGraphicsPixmapItem *starItem = new QGraphicsPixmapItem(m_starPixmap[0]);
        //carItem->setTransformationMode(Qt::SmoothTransformation);
        starItem->setScale(1);
        starItem->setPos(Game::m_checkpoint[0][i][0] * m_game.gamescale - m_game.offsetX,Game::m_checkpoint[0][i][1] * m_game.gamescale - m_game.offsetY);
        addItem(starItem);
        m_starItems.append(starItem);
    }

    for (int i = 0; i < Game::COUNT_OF_CARS; ++i) {
        QGraphicsPixmapItem *carItem = new QGraphicsPixmapItem(m_carPixmap[i]);
        //carItem->setTransformationMode(Qt::SmoothTransformation);
        carItem->setScale(1);
        carItem->setTransformOriginPoint(21, 34);
        addItem(carItem);
        m_carItems.append(carItem);
    }

    m_timer->start(m_game.ITERATION_VALUE);
    update();
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
        qDebug() << "BgPixmap1 is loaded successfully";
    }
    else
    {
        qDebug() << "BgPixmap1 is not loaded successfully";
    }


    if(m_carPixmap[0].load(m_game.PATH_TO_CAR_PIXMAP[0]))
    {
        qDebug() << "CarPixmap[0] is loaded successfully";
    }

    if(m_starPixmap[0].load(m_game.PATH_TO_STAR_PIXMAP[0]))
    {
        qDebug() << "StarPixmap[0] is loaded successfully";
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


void GameScene::update()
{
    clear();
    QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(m_bgPixmap[m_mapIdx]);
    //bgItem->setTransformationMode(Qt::SmoothTransformation);
    bgItem->setScale(m_game.gamescale);
    addItem(bgItem);

//    QGraphicsPixmapItem* carItem = new QGraphicsPixmapItem(m_carPixmap);
//    carItem->setTransformationMode(Qt::SmoothTransformation);
//    carItem->setScale(1);
//    carItem->setPos(200, 200);
//    addItem(carItem);


    carMovement();
    carCollision();
    // checkStarCollision();

    // if(m_game.m_starScore == Game::COUNTING_STARS)
    //     Goal();
    //getPixelValueAtCarPosition();

    m_game.offsetX = m_game.car[0].x-160 * m_game.gamescale;
    m_game.offsetY = m_game.car[0].y-120 * m_game.gamescale;

    bgItem->setPos(-m_game.offsetX, -m_game.offsetY);

    for (int i = m_game.m_starScore; i < Game::COUNTING_STARS; ++i) {
        QGraphicsPixmapItem *starItem = new QGraphicsPixmapItem(m_starPixmap[0]);
        //carItem->setTransformationMode(Qt::SmoothTransformation);
        starItem->setScale(1);
        starItem->setPos(Game::m_checkpoint[0][i][0] * m_game.gamescale - m_game.offsetX,Game::m_checkpoint[0][i][1] * m_game.gamescale - m_game.offsetY);
        addItem(starItem);
        m_starItems.append(starItem);
    }

    for(int i=0; i < Game::COUNT_OF_CARS; i++)
    {
        QGraphicsPixmapItem* carItem = new QGraphicsPixmapItem(m_carPixmap[i]);
        carItem->setTransformationMode(Qt::SmoothTransformation);
        carItem->setScale(1);
        //42x69 average of pixmaps
        carItem->setTransformOriginPoint(21, 34);
        carItem->setPos(m_game.car[i].x - m_game.offsetX, m_game.car[i].y - m_game.offsetY);
        carItem->setRotation(m_game.car[i].angle * 180/3.141593);
        addItem(carItem);
    }
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
    if(abs(angle) < 200){
        qDebug()<<"##########[Staight]###########################";
        m_leftDir = false;
        m_rightDir = false;

    }
    else if(angle < 0){
        qDebug()<<"!!!!!!!!!!!!!!!!!!!![Right]!!!!!!!!!!!!!!!!!!!!";
        m_leftDir = false;
        m_rightDir = true;
    }
    else{
        qDebug()<<"@@@@@@@@@@@@@@@@@@@@[LEFT]@@@@@@@@@@@@@@@@@";
        m_leftDir = true;
        m_rightDir = false;
    }

}

void GameScene::setMapIdx(int mapIdx)
{
    m_mapIdx = mapIdx;

    m_game.resetGameData(mapIdx);
}

bool GameScene::checkStarCollision()
{
    bool bReturn = false;
    int i32Range = 20;
    int i32CarX = m_game.car[0].x;
    int i32CarY = m_game.car[0].y;

    do
    {
        if(m_game.m_starScore >= Game::COUNTING_STARS)
            break;

        int i32StarX = Game::m_checkpoint[0][m_game.m_starScore][0]* m_game.gamescale;
        int i32StarY = Game::m_checkpoint[0][m_game.m_starScore][1]* m_game.gamescale;

        if(i32CarX > i32StarX - i32Range && i32CarY < i32StarY + i32Range)
        {
            if(i32CarY > i32StarY - i32Range && i32CarY < i32StarY + i32Range)
            {
                bReturn = true;
                m_game.m_starScore++;
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

    setMapIdx(idx);
}
