#ifndef GAME_H
#define GAME_H
#include "car.h"
#include <QSize>
#include <QString>
#include "car.h"

class Game
{
public:
    Game();

    void resetGameData(int mapIdx);
    static const int num_checkpoints;
    static const int points[8][2];
    static const QSize RESOLUTION;
    const QString PATH_TO_BACKGROUND_PIXMAP[10];
    const QString PATH_TO_CAR_PIXMAP[5];
    const float ITERATION_VALUE;

    float car_R;

    static const int COUNT_OF_CARS;
    Car car[5];


   float speed, angle;
   float maxSpeed;
   float acc, dec;
   float turnSpeed;
   float gamescale = 2;
   int m_pixelRed = 200;
   int m_pixelGreen = 255;
   int m_pixelBlue = 255;

   int offsetX,offsetY;
};

#endif // GAME_H
