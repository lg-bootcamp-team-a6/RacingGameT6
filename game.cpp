#include "game.h"

#include <QDebug>

const int Game::num_checkpoints = 8;
//orginal point bg.png 1x1
/*
0) 120 1590
1) 120 735
2) 130 200
3) 710 225
4) 775 620
5)1200 735
6)1190 1650
7) 645 1714
 */
const int Game::points[8][2]=
{
                               {240, 3180},
                               {240, 1470},
                               {260, 400},
                               {1420,450},
                               {1550,1240},
                               {2400,1470},
                               {2380,3300},
                               {1290, 3430}
};
const int Game::m_checkpoint[3][30][2]=
{
    {
                               {350,1300},
                               {350,1200},
                               {350,1100},
                               {350,1000},
                               {350,900},
                               {350,800},
                               {350,700},
                               {350,600},
                               {350,500},
                               {350,400},
                               {470,300},
                               {570,300},
                               {670,300},
                               {780,360},
                               {810,450},
                               {880,550},
                               {940,650},
                               {1000,700},
                               {1100,800},
                               {1100,900},
                               {1100,1000},
                               {1100,1100},
                               {1100,1200},
                               {1100,1300},
                               {1100,1400},
                               {950,1570},
                               {850,1570},
                               {750,1570},
                               {650,1570},
                               {550,1570}
    },
    {
                               {350,1300},
                               {350,1200},
                               {350,1100},
                               {350,1000},
                               {350,900},
                               {350,800},
                               {350,700},
                               {350,600},
                               {350,500},
                               {350,400},
                               {470,300},
                               {570,300},
                               {670,300},
                               {780,360},
                               {810,450},
                               {880,550},
                               {940,650},
                               {1000,700},
                               {1100,800},
                               {1100,900},
                               {1100,1000},
                               {1100,1100},
                               {1100,1200},
                               {1100,1300},
                               {1100,1400},
                               {950,1570},
                               {850,1570},
                               {750,1570},
                               {650,1570},
                               {550,1570}
    },
    {
                               {350,1300},
                               {350,1200},
                               {350,1100},
                               {350,1000},
                               {350,900},
                               {350,800},
                               {350,700},
                               {350,600},
                               {350,500},
                               {350,400},
                               {470,300},
                               {570,300},
                               {670,300},
                               {780,360},
                               {810,450},
                               {880,550},
                               {940,650},
                               {1000,700},
                               {1100,800},
                               {1100,900},
                               {1100,1000},
                               {1100,1100},
                               {1100,1200},
                               {1100,1300},
                               {1100,1400},
                               {950,1570},
                               {850,1570},
                               {750,1570},
                               {650,1570},
                               {550,1570}
    }
};
const QSize Game::RESOLUTION = QSize(640, 480);
const int Game::COUNT_OF_CARS = 1;
const int Game::COUNTING_STARS = 30;

Game::Game()
    : PATH_TO_BACKGROUND_PIXMAP{QString(":/images/bg0.png"), QString(":/images/bg1.png"), QString(":/images/bg2.png")}, PATH_TO_CAR_PIXMAP{QString(":/images/car0.png"), QString(":/images/car1.png"), QString(":/images/car2.png"), QString(":/images/car3.png"), QString(":/images/car4.png")}, PATH_TO_STAR_PIXMAP{QString(":/images/star0.png"), QString(":/images/star1.png"), QString(":/images/star2.png")}, ITERATION_VALUE(1000.0f/200.0f),
      car_R(22), speed(0), angle(0), maxSpeed(20), acc(2.0f), dec(2.0f), turnSpeed(0.08), offsetX(0), offsetY(0)
{
    car_R = 22;
    speed = 0;
    angle = 0;
    maxSpeed = 20; 
    acc = 2.0f;
    dec = 2.0f;
    turnSpeed = 0.08;
    offsetX = 0;
    offsetY = 0;
    car[0].x = 350 * gamescale;
    car[0].y = 1500 * gamescale;

    /*
     * cars` center
     *  0) 41x65
        1) 41x67
        2) 42x72
        3) 42x72
        4) 41x70
        avg: 42x69
     */
}

void Game::resetGameData(int mapIdx)
{
    if(mapIdx == 0)
    {
        car_R = 22;
        speed = 0;
        angle = 0;
        maxSpeed = 20; 
        acc = 2.0f;
        dec = 2.0f;
        turnSpeed = 0.08;
        offsetX = 0;
        offsetY = 0;
        car[0].x = 370 * gamescale;
        car[0].y = 1300 * gamescale;
        qDebug() << "Game data reset for map 0";
    }
    else if (mapIdx == 1)
    {
        car_R = 22;
        speed = 0;
        angle = 0;
        maxSpeed = 20; 
        acc = 2.0f;
        dec = 2.0f;
        turnSpeed = 0.08;
        offsetX = 0;
        offsetY = 0;
        car[0].x = 370 * gamescale;
        car[0].y = 1300 * gamescale;
        qDebug() << "Game data reset for map 1";
    }
    else if (mapIdx == 2)
    {
        car_R = 22;
        speed = 0;
        angle = 0;
        maxSpeed = 20; 
        acc = 2.0f;
        dec = 2.0f;
        turnSpeed = 0.08;
        offsetX = 0;
        offsetY = 0;
        car[0].x = 300 * gamescale;
        car[0].y = 600 * gamescale;
        qDebug() << "Game data reset for map 2";
    }
    else
    {
        qDebug() << "Invalid map index";
    }
}
