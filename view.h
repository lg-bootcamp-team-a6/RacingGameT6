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
    void onAccelButtonClicked();
    void onBrakeButtonClicked();

    QWidget *m_overlay;
    QPushButton *m_accelButton;
    QPushButton *m_brakeButton;
    QLabel *m_directionArrow; // 차량 진행 방향 화살표 표시용 QLabel
};

#endif // VIEW_H
