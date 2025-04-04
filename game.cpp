#include "game.h"
#include "AudioHandler.h"

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
                               {315,1300},
                               {315,1200},
                               {315,1100},
                               {315,1000},
                               {315,900},
                               {315,800},
                               {315,700},
                               {315,600},
                               {315,500},
                               {315,400},
                               {470,270},
                               {570,270},
                               {670,270},
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
                               {950,1550},
                               {850,1550},
                               {750,1550},
                               {650,1550},
                               {550,1550}
    },
    {
                               {330,1200},
                               {330,1050},
                               {330,900},
                               {330,750},
                               {330,600},
                               {330,450},
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
                               {1100,1580},
                               {1000,1510},
                               {900,1440},
                               {750,1430},
                               {690,1520},
                               {610,1610},
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
                               {1080,480},
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
                               {650,1050},
                               {560,1170},
                               {600,1360},
                               {640,1540},
                               {450,1550},
                               {350,1500},
                               {290,1300},
                               {290,1150},
                               {290,1000},
                               {290,850},
                               {290,600}
    },

    {
                               {220,2300},
                               {220,2225},
                               {220,2150},
                               {220,2075},
                               {220,2000},
                               {220,1925},
                               {220,1850},
                               {220,1775},
                               {220,1700},
                               {220,1625},
                               {220,1550},
                               {220,1475},
                               {220,1400},
                               {220,1325},
                               {220,1250},
                               {220,1175},
                               {220,1100},
                               {220,1025},
                               {220,950},
                               {220,875},
                               {220,800},
                               {220,725},
                               {220,650},
                               {220,575},
                               {220,500},
                               {220,425},
                               {220,350},
                               {220,275},
                               {220,200},
                               {220,125}
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
      PATH_TO_WIN_PIXMAP(":/images/win.png"), PATH_TO_LOSE_PIXMAP(":/images/lose.png"),
      PATH_TO_BOOSTER_PIXMAP{QString(":/images/Booster.png"), QString(":/images/Booster1.png")},
      ITERATION_VALUE(100), car_R(22), speed(0), angle(0), maxSpeed(13), acc(1.0f), dec(1.0f), turnSpeed(0.07), offsetX(0), offsetY(0)
      
{ car_R = 22;
    speed = 0;
    angle = 0;
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
    turnSpeed = 0.07;
    offsetX = 0;
    offsetY = 0;
    m_starScore = 0;
    maxSpeed = 13;
    acc = 1.0f;
    dec = 1.0f;
    AudioHandler::getInstance()->playNextTrack();

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
        car[0].y = 500 * gamescale;
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
        turnSpeed = 0.05;
        qDebug() << "Game data reset for map 3";
    }
    else
    {
        qDebug() << "Invalid map index";
    }
}
