#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>
class GameScene;
class View : public QGraphicsView
{
    Q_OBJECT
public:
    explicit View();

signals:
private:
    GameScene *m_gameScene;
};

#endif // VIEW_H
