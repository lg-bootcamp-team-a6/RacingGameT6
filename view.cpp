#include "view.h"
#include "gamescene.h"
#include "AudioHandler.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QDebug>
#include <QLabel>
#include <QPixmap>
#include <QTransform>
#include <QTimer>

View::View()
    : QGraphicsView{},
      m_gameScene(new GameScene(this)),
      m_directionArrow(nullptr),
      m_timerLabel(nullptr),
      m_displayTimer(nullptr),
      m_elapsedTime(0)
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
    // 오버레이 배경은 완전 투명
    m_overlay->setStyleSheet("background-color: transparent;");

    // 오버레이 전체를 QVBoxLayout으로 구성 (상단: 화살표 + 타이머, 하단: 버튼)
    QVBoxLayout *mainLayout = new QVBoxLayout(m_overlay);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 상단 영역: QHBoxLayout으로 좌측에 화살표, 우측에 타이머 표시
    QHBoxLayout *topLayout = new QHBoxLayout();
    // 차량 진행 방향 화살표 (왼쪽)
    m_directionArrow = new QLabel(m_overlay);
    QPixmap arrowPixmap(":/images/arrow.png");  // 화살표 이미지 리소스 경로
    QTransform transform;
    transform.rotate(270);
    QPixmap rotated = arrowPixmap.transformed(transform, Qt::SmoothTransformation);
    m_directionArrow->setPixmap(rotated.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_directionArrow->setFixedSize(50, 50);
    m_directionArrow->setStyleSheet("background-color: transparent;");
    topLayout->addWidget(m_directionArrow, 0, Qt::AlignLeft | Qt::AlignVCenter);

    // 타이머 및 정보 라벨 (오른쪽)
    m_timerLabel = new QLabel(m_overlay);
    m_timerLabel->setStyleSheet("color: black; font-size: 20px; background-color: transparent;");
    m_timerLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    topLayout->addStretch();
    topLayout->addWidget(m_timerLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
    mainLayout->addLayout(topLayout);

    // 중간에 스트레치 (상단과 하단 사이 공간 확보)
    mainLayout->addStretch();

    // 하단 영역: 버튼들을 포함하는 수평 레이아웃 생성
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    // 왼쪽 버튼: Accel
    m_accelButton = new QPushButton("", m_overlay);
    m_accelButton->setFixedSize(100, 100);
    m_accelButton->setIcon(QIcon(":/images/accel_pedal.png"));
    m_accelButton->setIconSize(QSize(100, 100));
    m_accelButton->setStyleSheet("border-radius: 50px; background-color: rgba(255, 255, 255, 150);");
    m_accelButton->setFocusPolicy(Qt::NoFocus);
    m_accelButton->setAutoRepeat(false);

    // 오른쪽 버튼: Brake
    m_brakeButton = new QPushButton("", m_overlay);
    m_brakeButton->setFixedSize(100, 100);
    m_brakeButton->setIcon(QIcon(":/images/accel_pedal.png"));
    m_brakeButton->setIconSize(QSize(100, 100));
    m_brakeButton->setStyleSheet("border-radius: 50px; background-color: black;");
    m_brakeButton->setFocusPolicy(Qt::NoFocus);
    m_brakeButton->setAutoRepeat(false);

    buttonLayout->addWidget(m_accelButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_brakeButton, 0, Qt::AlignRight | Qt::AlignVCenter);
    mainLayout->addLayout(buttonLayout);

    // Add Audio button
    m_audioButton = new QPushButton("", m_overlay);
    m_audioButton->setFixedSize(100, 100);
    m_audioButton->setIcon(QIcon(":/images/off.png"));  // default: on 상태
    m_audioButton->setIconSize(QSize(100, 100));
    m_audioButton->setStyleSheet("border-radius: 50px; left-padding : 25px; background-color: green;");
    m_audioButton->setFocusPolicy(Qt::NoFocus);
    m_audioButton->setAutoRepeat(false);
    mainLayout->addWidget(m_audioButton, 0, Qt::AlignHCenter | Qt::AlignBottom);  // m_brakeButton 밑에 배치

    // set layout
    m_overlay->setLayout(mainLayout);
    repositionOverlay();
    m_overlay->raise();
    m_overlay->show();

    // 버튼 이벤트 연결
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

    // 타이머 업데이트: 100ms 간격으로 타이머 및 (옵션) 속도/각도 정보 갱신
    m_elapsedTime = 0;
    m_displayTimer = new QTimer(this);
    connect(m_displayTimer, &QTimer::timeout, this, [this]() {
         m_elapsedTime += m_displayTimer->interval();
         int seconds = m_elapsedTime / 1000;
         int centiseconds = (m_elapsedTime % 1000) / 10;
         // 예시: 시간만 표시 (필요 시 속도와 각도 정보를 추가할 수 있음)
         QString infoText = QString("Time: %1.%2")
                              .arg(seconds, 2, 10, QChar('0'))
                              .arg(centiseconds, 2, 10, QChar('0'));
         // 예시로 m_gameScene에서 속도와 각도를 얻을 수 있다면:
         // infoText += QString("\nSpeed: %1 | Angle: %2").arg(m_gameScene->m_game.speed).arg(m_gameScene->m_game.angle);
         m_timerLabel->setText(infoText);
    });
    m_displayTimer->start(100);
    // 음량 버튼 클릭 시 상태 변경
    connect(m_audioButton, &QPushButton::clicked, this, [this]() {
        if (m_isAudioOn) {
            // 음악 정지
            AudioHandler::getInstance()->stopAudio("magicjuly.wav");
            m_audioButton->setIcon(QIcon(":/images/on.png"));  // 아이콘 변경
        } else {
            // 음악 재생
            AudioHandler::getInstance()->playAudio("magicjuly.wav", true);
            m_audioButton->setIcon(QIcon(":/images/off.png"));  // 아이콘 변경
        }
        m_isAudioOn = !m_isAudioOn;  // 상태 반전
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
        // 하단에서 150px을 제외한 영역으로 설정 (bottom margin 150px)
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
    if(abs(angle) < 200){
        qDebug()<<"------------[Staight Arrow]-------------";
        transform.rotate(270); // 필요에 따라 오프셋 조정

    }
    else if(angle < 0){
        qDebug()<<"------------[Right Arrow]-------------";
        transform.rotate(0); // 필요에 따라 오프셋 조정
    }
    else{
        qDebug()<<"-----------[LEFT Arrow]----------------";
        transform.rotate(180); // 필요에 따라 오프셋 조정
    }

    QPixmap rotated = arrowPixmap.transformed(transform, Qt::SmoothTransformation);
    m_directionArrow->setPixmap(rotated.scaled(m_directionArrow->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
