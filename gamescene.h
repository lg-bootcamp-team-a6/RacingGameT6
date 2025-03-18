#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QPixmap>
#include "game.h"
#include "AudioHandler.h"
#include <QLabel>
#include <QTimer>
#include <QFont>
#include <QTime>
#include <QThread>
#include <QGraphicsTextItem>
#include <QPushButton>
#include "UdpSocketHandler.h"
#include "UdpCmd.h"
#include "UdpReceiverWorker.h"

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
    bool getDirectionChanged();
    void setMapIdx(int mapIdx);
    bool checkStarCollision();
    void Goal();
    void parseMyIp(char* data);
    int m_mapCnt = 4;
    int m_carCnt = 1;
    int m_mapIdx = 0;
    bool m_bConnect = false;
    int m_myIp = 3;

signals:

public slots:
    void handleUdpPacket(const receive_packet &pkt);


private slots:
    void update();
    void loadPixmap();
    void carMovement();
    void carCollision();
    void renderScene();

private:

    Game m_game;
    QTimer* m_timer;
    QPixmap m_bgPixmap[4], m_carPixmap[5], m_starPixmap[4], m_readyPixmap[3], m_pausePixmap, m_finishPixmap, m_winPixmap, m_losePixmap;
    QGraphicsPixmapItem* m_bgItem[4], *m_carItem[5], *m_readyItem[3], * m_starItem[30];
    UdpSocketHandler *m_pUdpSocketHandler;
    
    /* Audio */
    QPushButton* m_audioButton;
    QPushButton* m_audioChangeButton;
    QString m_currentTrackKey;
    AudioHandler* m_audioHandler; // AudioHandler 싱글턴 인스턴스

    bool m_upDir, m_rightDir, m_downDir, m_leftDir, m_dirChanged;

public:
    void togglePause(bool IsResume);
    void toggleAudioStatus();
    void changeAudio();
    void showText();
    void SocketUDP(const int16_t cmd, const char* data);
    void Wait3Seconds();
    void resetGame();
    void parseRivalPosition(char* data);
    void FinishRace(bool win);
    QGraphicsPixmapItem *m_pauseItem;
    QGraphicsPixmapItem *m_finishItem;
    QGraphicsPixmapItem *m_winItem;
    QGraphicsPixmapItem *m_loseItem;
    qint64 m_elapsedTime;
    int m_computeTime;
    bool m_bIsResume;
    QList<QGraphicsPixmapItem*> m_cntItems;
    bool m_bReady;
    int m_rivalScore;
    bool m_bSingle;
    bool m_bStart;

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
};

#endif // GAMESCENE_H
