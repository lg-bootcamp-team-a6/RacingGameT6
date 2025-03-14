#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QResizeEvent>

class GameScene;

class View : public QGraphicsView
{
    Q_OBJECT
public:
    explicit View();
    GameScene *m_gameScene;

    // 차량 진행 방향(도 단위) 업데이트를 위한 함수
    void updateDirectionArrow(double angle);

signals:

private:
    void setupOverlay();
    void repositionOverlay();
    void resizeEvent(QResizeEvent *event) override;

    QWidget *m_overlay;
    QPushButton *m_accelButton;
    QPushButton *m_brakeButton;
    QPushButton *m_audioButton;  // 추가된 음량 버튼
    QLabel *m_directionArrow; // 차량 진행 방향 화살표 표시용 QLabel

    QLabel *m_timerLabel;     // 타이머 및 추가 정보 표시용 QLabel
    QTimer *m_displayTimer;   // 타이머 업데이트용 QTimer
    qint64 m_elapsedTime;        // 경과 시간 (밀리초 단위)
    bool m_isAudioOn = true;  // 음량 상태 (기본값: on)
};

#endif // VIEW_H
