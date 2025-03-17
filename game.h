#ifndef GAME_H
#define GAME_H
#include "car.h"
#include <QSize>
#include <QList>
#include <QString>
#include <algorithm> 
#include "car.h"

class Game
{
public:
    Game();

    void resetGameData(int mapIdx);
    static const int num_checkpoints;
    static const int points[8][2];
    static const int m_checkpoint[4][30][2];
    QList<int> m_rankRecord[4];
    static const QSize RESOLUTION;
    const QString PATH_TO_BACKGROUND_PIXMAP[10];
    const QString PATH_TO_CAR_PIXMAP[5];
    const QString PATH_TO_STAR_PIXMAP[5];
    QString PATH_TO_COUNT_PIXMAP[3];
    const QString PATH_TO_PAUSE_PIXMAP;
    const float ITERATION_VALUE;

    float car_R;

    static const int COUNT_OF_CARS;
    static const int COUNTING_STARS;
    Car car[5];


   float speed, angle;
   float maxSpeed;
   float acc, dec;
   float turnSpeed;
   float gamescale = 2;
   int m_pixelRed = 173;
   int m_pixelGreen = 173;
   int m_pixelBlue = 173;
   int m_starScore = 0;

   int offsetX,offsetY;
};

#endif // GAME_H
