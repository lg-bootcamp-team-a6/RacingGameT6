#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QTimer>

class GameScene;
class View : public QGraphicsView
{
    Q_OBJECT
public:
    explicit View();
    GameScene *m_gameScene;

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
    QLabel *m_timerLabel;
    QTimer *m_displayTimer;
};

#endif // VIEW_H
