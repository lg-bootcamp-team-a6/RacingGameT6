#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPixmap>
#include "game.h"
#include <QLabel>
#include <QTimer>
#include <QFont>
#include <QTime>
#include <QGraphicsTextItem>
#include "UdpSocketHandler.h"

class QTimer;


class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    void setUpDirection(bool upDir);
    void setRightDirection(bool rightDir);
    void setDownDirection(bool downDir);
    void setAngleDirection(double angle);
    void setMapIdx(int mapIdx);
    bool checkStarCollision();
    void Goal();

    int m_mapCnt = 3;
    int m_mapIdx = 0;

signals:


private slots:
    void update();
    void loadPixmap();
    void carMovement();
    void carCollision();
    void renderScene();

private:

    Game m_game;
    QTimer* m_timer;
    QPixmap m_bgPixmap[3], m_carPixmap[5], m_starPixmap[3], m_readyPixmap[3], m_pausePixmap;
    QGraphicsPixmapItem* m_bgItem;
    QList<QGraphicsPixmapItem*> m_starItems;
    QList<QGraphicsPixmapItem*> m_carItems;
    UdpSocketHandler *m_pUdpSocketHandler;

    bool m_upDir, m_rightDir, m_downDir, m_leftDir;

public:
    void togglePause(bool IsResume);
    void showText();
    void SocketUDP();
    void Wait3Seconds();
    QGraphicsPixmapItem *m_pauseItem;
    qint64 m_elapsedTime;
    int m_computeTime;
    bool m_bIsResume;
    QList<QGraphicsPixmapItem*> m_cntItems;
    bool m_bReady = false;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // GAMESCENE_H
