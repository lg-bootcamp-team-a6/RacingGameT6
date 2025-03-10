#include "game.h"

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
const QSize Game::RESOLUTION = QSize(640, 480);
const int Game::COUNT_OF_CARS = 5;

Game::Game()
    : PATH_TO_BACKGROUND_PIXMAP(":/images/bg.png"), PATH_TO_CAR_PIXMAP{QString(":/images/car0.png"), QString(":/images/car1.png"), QString(":/images/car2.png"), QString(":/images/car3.png"), QString(":/images/car4.png")}, ITERATION_VALUE(1000.0f/60.0f),
      car_R(22), speed(0), angle(0), maxSpeed(12), acc(0.2f), dec(0.3f), turnSpeed(0.08), offsetX(0), offsetY(0)
{
    for(int i=0; i < COUNT_OF_CARS;i++)
    {
      car[i] = Car(num_checkpoints, points);
      car[i].x = 400+i*50;
      car[i].y = 3480+i*80;
      car[i].speed = 7+i;
    }
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
