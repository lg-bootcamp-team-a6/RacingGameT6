#include "view.h"
#include "gamescene.h"
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QDebug>

View::View()
    : QGraphicsView{},
      m_gameScene(new GameScene(this))
{
    setScene(m_gameScene);
    resize(m_gameScene->sceneRect().width() + 2, m_gameScene->sceneRect().height() + 2);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setupOverlay();
}

void View::setupOverlay()
{
    // viewport()의 자식으로 오버레이 위젯 생성
    m_overlay = new QWidget(viewport());
    m_overlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    // 디버그용 배경색 (나중에 필요 없으면 제거)
    //m_overlay->setStyleSheet("background-color: rgba(255, 0, 0, 100);");

    // 오버레이에 버튼 배치를 위한 QHBoxLayout 생성
    QHBoxLayout *layout = new QHBoxLayout(m_overlay);
    // 여백: 좌우 20, 상하 20 (버튼이 오버레이 내부에서 너무 붙지 않게)
    layout->setContentsMargins(20, 20, 20, 20);

    // 왼쪽 버튼: Accel (100×100, 동그란 형태)
    m_accelButton = new QPushButton("", m_overlay);
    m_accelButton->setFixedSize(100, 100);

    m_accelButton->setIcon(QIcon(":/images/accel_pedal.png"));
    // 아이콘 크기를 버튼 크기에 맞게 지정
    m_accelButton->setIconSize(QSize(100, 100));
    // 기존 스타일 적용 (원형 버튼)
    m_accelButton->setStyleSheet("border-radius: 50px; left-padding : 25px; background-color: rgba(255, 255, 255, 150);");
    m_accelButton->setFocusPolicy(Qt::NoFocus);



    // 오른쪽 버튼: Brake (100×100, 동그란 형태)
    m_brakeButton = new QPushButton("", m_overlay);
    m_brakeButton->setFixedSize(100, 100);
    m_brakeButton->setIcon(QIcon(":/images/accel_pedal.png"));
    // 아이콘 크기를 버튼 크기에 맞게 지정
    m_brakeButton->setIconSize(QSize(100,100));
    // 기존 스타일 적용 (원형 버튼)
    m_brakeButton->setStyleSheet("border-radius: 50px; left-padding : 25px; background-color: black;");
    m_brakeButton->setFocusPolicy(Qt::NoFocus);

    // 좌측에 accel 버튼, 우측에 brake 버튼 배치 (중간에 addStretch() 사용)
    layout->addWidget(m_accelButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    layout->addStretch();
    layout->addWidget(m_brakeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_overlay->setLayout(layout);

    repositionOverlay();
    m_overlay->raise();
    m_overlay->show();

    connect(m_accelButton, &QPushButton::clicked, this, &View::onAccelButtonClicked);
    connect(m_brakeButton, &QPushButton::clicked, this, &View::onBrakeButtonClicked);
}


void View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    repositionOverlay();
}

void View::repositionOverlay()
{
    if (viewport()) {
        // 오버레이 높이를 140px로 설정 (버튼 크기 100 + 내부 여백)
        int overlayHeight = 140;
        // 바닥에서 20px 위에 위치하도록 함
        int bottomMargin = 150;
        QRect viewportRect = viewport()->rect();
        m_overlay->setGeometry(0,
                               viewportRect.height() - overlayHeight - bottomMargin,
                               viewportRect.width(),
                               overlayHeight);
        qDebug() << "Overlay geometry:" << m_overlay->geometry();
    }
}

void View::onAccelButtonClicked()
{
    qDebug() << "Accel Button Clicked";

    m_gameScene->setUpDirection(true);
}

void View::onBrakeButtonClicked()
{
    qDebug() << "brake Button Clicked";
    m_gameScene->setUpDirection(false);
}
