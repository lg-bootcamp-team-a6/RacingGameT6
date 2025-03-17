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
const int Game::m_checkpoint[4][30][2]=
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
                               {350,1200},
                               {350,1050},
                               {350,900},
                               {350,750},
                               {350,600},
                               {350,450},
                               {450,300},
                               {650,390},
                               {900,290},
                               {1100,400},
                               {950,510},
                               {740,600},
                               {850,700},
                               {1040,780},
                               {1110,940},
                               {910,1000},
                               {750,1010},
                               {650,1030},
                               {720,1250},
                               {900,1250},
                               {1100,1300},
                               {1150,1450},
                               {1100,1600},
                               {1000,1510},
                               {900,1440},
                               {750,1430},
                               {690,1520},
                               {610,1640},
                               {480,1520},
                               {380,1400}
    },
    {
                               {290,450},
                               {350,260},
                               {560,280},
                               {780,340},
                               {950,240},
                               {1110,310},
                               {1080,500},
                               {900,570},
                               {720,600},
                               {620,800},
                               {800,760},
                               {950,750},
                               {1050,880},
                               {1140,1010},
                               {1200,1150},
                               {1070,1350},
                               {870,1450},
                               {800,1340},
                               {740,1100},
                               {570,1150},
                               {620,1170},
                               {640,1360},
                               {650,1540},
                               {450,1550},
                               {350,1500},
                               {290,1300},
                               {290,1150},
                               {290,1000},
                               {290,850},
                               {290,600}
    },

    {
                               {240,2300},
                               {240,2200},
                               {240,2100},
                               {240,2000},
                               {240,1900},
                               {240,1800},
                               {240,1700},
                               {240,1600},
                               {240,1500},
                               {240,1400},
                               {240,1300},
                               {240,1200},
                               {240,1100},
                               {240,1000},
                               {240,950},
                               {240,900},
                               {240,850},
                               {240,800},
                               {240,750},
                               {240,700},
                               {240,650},
                               {240,600},
                               {240,550},
                               {240,500},
                               {240,470},
                               {240,450},
                               {240,430},
                               {240,420},
                               {240,410},
                               {240,390}
    }
};
const QSize Game::RESOLUTION = QSize(640, 480);
const int Game::COUNTING_STARS = 30;

Game::Game()
    : PATH_TO_BACKGROUND_PIXMAP{QString(":/images/bg0.png"), QString(":/images/bg1.png"), QString(":/images/bg2.png"), QString(":/images/bg3.png")},
      PATH_TO_CAR_PIXMAP{QString(":/images/car0.png"), QString(":/images/car1.png"), QString(":/images/car2.png"), QString(":/images/car3.png"), QString(":/images/car4.png")},
      PATH_TO_STAR_PIXMAP{QString(":/images/star0.png"), QString(":/images/star1.png"), QString(":/images/star2.png")},
      PATH_TO_COUNT_PIXMAP{QString(":/images/Ready_1.png"), QString(":/images/Ready_2.png"), QString(":/images/Ready_3.png")},
      PATH_TO_PAUSE_PIXMAP(":/images/pause.png"), PATH_TO_FINISH_PIXMAP(":/images/finish.png"), PATH_TO_START_PIXMAP(":/images/start.png"),
      ITERATION_VALUE(100), car_R(22), speed(0), angle(0), maxSpeed(10), acc(1.0f), dec(1.0f), turnSpeed(0.08), offsetX(0), offsetY(0)
{ car_R = 22;
    speed = 0;
    angle = 0;
    maxSpeed = 12;
    acc = 2.0f;
    dec = 2.0f;
    turnSpeed = 0.08;
    offsetX = 0;
    offsetY = 0;
    car[0].x = 240 * gamescale;
    car[0].y = 2400 * gamescale;
    m_starScore = 0;
    m_pixelRed = 233;
    m_pixelGreen = 223;
    m_pixelBlue = 223;

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
    car_R = 22;
    speed = 0;
    angle = 0;
    turnSpeed = 0.08;
    offsetX = 0;
    offsetY = 0;
    m_starScore = 0;

    if(mapIdx == 0)
    {
        car[0].x = 350 * gamescale;
        car[0].y = 1500 * gamescale;
        m_pixelRed = 173;
        m_pixelGreen = 173;
        m_pixelBlue = 173;
        qDebug() << "Game data reset for map 0";
    }
    else if (mapIdx == 1)
    {
        car[0].x = 350 * gamescale;
        car[0].y = 1300 * gamescale;
        m_pixelRed = 173;
        m_pixelGreen = 173;
        m_pixelBlue = 173;
        qDebug() << "Game data reset for map 1";
    }
    else if (mapIdx == 2)
    {
        car[0].x = 300 * gamescale;
        car[0].y = 600 * gamescale;
        m_pixelRed = 173;
        m_pixelGreen = 173;
        m_pixelBlue = 173;
        qDebug() << "Game data reset for map 2";
    }
    else if (mapIdx == 3)
    {
        car[0].x = 240 * gamescale;
        car[0].y = 2400 * gamescale;
        m_pixelRed = 233;
        m_pixelGreen = 223;
        m_pixelBlue = 223;
        qDebug() << "Game data reset for map 3";
    }
    else
    {
        qDebug() << "Invalid map index";
    }
}
