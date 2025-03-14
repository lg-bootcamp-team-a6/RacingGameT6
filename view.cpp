#include "view.h"
#include "gamescene.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QTransform>

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
    // 오버레이 배경은 완전 투명으로 설정
    m_overlay->setStyleSheet("background-color: transparent;");

    // 오버레이 전체를 QVBoxLayout으로 구성 (상단: 화살표, 하단: 버튼)
    QVBoxLayout *mainLayout = new QVBoxLayout(m_overlay);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 상단 왼쪽에 고정할 차량 방향 화살표 (QLabel)
    m_directionArrow = new QLabel(m_overlay);
    QPixmap arrowPixmap(":/images/arrow.png");  // 화살표 이미지 리소스 경로
    // 화살표 크기를 50×50으로 조정
    m_directionArrow->setPixmap(arrowPixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_directionArrow->setFixedSize(50, 50);
    m_directionArrow->setStyleSheet("background-color: transparent;");
    // 화살표는 상단 왼쪽에 배치
    mainLayout->addWidget(m_directionArrow, 0, Qt::AlignLeft | Qt::AlignTop);

    // 중간에 스트레치 추가 (화살표와 버튼 사이 공간 확보)
    mainLayout->addStretch();

    // 하단 버튼 영역: 버튼들을 포함하는 수평 레이아웃 생성
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    // 왼쪽 버튼: Accel
    m_accelButton = new QPushButton("", m_overlay);
    m_accelButton->setFixedSize(100, 100);
    m_accelButton->setIcon(QIcon(":/images/accel_pedal.png"));
    m_accelButton->setIconSize(QSize(100, 100));
    m_accelButton->setStyleSheet("border-radius: 50px; left-padding : 25px; background-color: rgba(255, 255, 255, 150);");
    m_accelButton->setFocusPolicy(Qt::NoFocus);
    m_accelButton->setAutoRepeat(false);

    // 오른쪽 버튼: Brake
    m_brakeButton = new QPushButton("", m_overlay);
    m_brakeButton->setFixedSize(100, 100);
    m_brakeButton->setIcon(QIcon(":/images/accel_pedal.png"));
    m_brakeButton->setIconSize(QSize(100, 100));
    m_brakeButton->setStyleSheet("border-radius: 50px; left-padding : 25px; background-color: black;");
    m_brakeButton->setFocusPolicy(Qt::NoFocus);
    m_brakeButton->setAutoRepeat(false);

    // 버튼들을 수평으로 배치 (좌측: Accel, 우측: Brake)
    buttonLayout->addWidget(m_accelButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_brakeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    // 하단 버튼 레이아웃 추가
    mainLayout->addLayout(buttonLayout);

    m_overlay->setLayout(mainLayout);
    repositionOverlay();
    m_overlay->raise();
    m_overlay->show();

    connect(m_accelButton, &QPushButton::pressed, this, [this]() {
        qDebug() << "UP pressed";
        m_gameScene->setUpDirection(true);
    });
    connect(m_accelButton, &QPushButton::released, this, [this]() {
        qDebug() << "UP released";
        m_gameScene->setUpDirection(false);
    });
    connect(m_brakeButton, &QPushButton::pressed, this, [this]() {
        m_gameScene->setDownDirection(true);
    });
    connect(m_brakeButton, &QPushButton::released, this, [this]() {
        m_gameScene->setDownDirection(false);
    });
}

void View::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    repositionOverlay();
}

void View::repositionOverlay()
{
    if (viewport()) {
        QRect rect = viewport()->rect();
        // 하단에서 150px을 제외한 영역으로 설정 (즉, bottom margin 150px)
        rect.setBottom(rect.bottom() - 150);
        m_overlay->setGeometry(rect);
        qDebug() << "Overlay geometry:" << m_overlay->geometry();
    }
}

// 차량의 진행 방향(각도, 도 단위)을 받아 화살표를 회전시킵니다.
void View::updateDirectionArrow(double angle)
{
    if (!m_directionArrow)
        return;
    
    QPixmap arrowPixmap(":/images/arrow.png");
    QTransform transform;
    transform.rotate(angle + 90);
    QPixmap rotated = arrowPixmap.transformed(transform, Qt::SmoothTransformation);
    m_directionArrow->setPixmap(rotated.scaled(m_directionArrow->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
