#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPixmap>
#include "game.h"

class QTimer;

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    void setUpDirection(bool upDir);
    void setAngleDirection(double angle);

signals:

private slots:
    void update();

private:
    void loadPixmap();
    void carMovement();
    void carCollision();
    void renderScene();

    Game m_game;
    QTimer* m_timer;
    QPixmap m_bgPixmap, m_carPixmap[5];
    QGraphicsPixmapItem* m_bgItem;
    QList<QGraphicsPixmapItem*> m_carItems;

    bool m_upDir, m_rightDir, m_downDir, m_leftDir;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // GAMESCENE_H
