#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
class GameScene;
class View : public QGraphicsView
{
    Q_OBJECT
public:
    explicit View();
    GameScene *m_gameScene;

signals:
private:
};

#endif // VIEW_H
