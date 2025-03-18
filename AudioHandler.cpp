#include "AudioHandler.h"
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QFileInfo>
#include <QIcon>
#include <QPointer>

AudioHandler::AudioHandler() 
    : settings("RacingGameT6", "AudioHandler"),
    loopTimer(new QTimer(this))
{
    setAudioStatus(true);
    setCurrentTrack("cookie");
    settings.sync();

    loopTimer->setSingleShot(true);  // 한 번만 실행 후 정지
    qDebug() << "AudioHandler initialized.";
}

AudioHandler::~AudioHandler() {
    stopAllAudio();
    delete loopTimer; // FREE loopTimer memory
}

const QMap<QString, AudioData>& AudioHandler::getAudioMap() {
    static QMap<QString, AudioData> audioMap = {
        {"cookie", {"cookie.wav", ":/images/cookie.png", 138}},
        {"july", {"july.wav", ":/images/july.png", 123}},
        {"dear", {"dear.wav", ":/images/dear.png", 128}},
        {"magic", {"magic.wav", ":/images/magic.png", 143}},
        {"walk", {"walk.wav", ":/images/walk.png", 130}}
    };
    return audioMap;
}

void AudioHandler::playAudio() {
    QString currentTrack = getCurrentTrack(); // no .wav -> magic

    // 현재 트랙이 비어 있으면 재생하지 않음
    if (currentTrack.isEmpty()) {
        qDebug() << __FUNCTION__ << " - No audio is currently set to play.";
        return;
    }

    // 트랙이 이미 재생 중인지 확인
    if (audioProcesses.find(currentTrack.toStdString()) != audioProcesses.end()) {
        qDebug() << __FUNCTION__ << " - Audio is already playing: " << currentTrack;
        return;
    }

    // 새 프로세스를 생성하여 트랙을 재생
    QProcess* process = new QProcess();
    QStringList arguments;

    // 확장자가 없으면 .wav를 추가
    QString trackWithExtension = currentTrack;
    if (!trackWithExtension.endsWith(".wav", Qt::CaseInsensitive)) {
        trackWithExtension += ".wav";
    }

    // arguments에 확장자를 포함한 트랙 경로 추가
    arguments << "-Dhw:0,0" << trackWithExtension;

    qDebug() << __FUNCTION__ << " - Executing command: ./aplay with arguments:" << arguments;

    process->start("./aplay", arguments);

    /* Setting */
    setAudioStatus(true);  // 오디오 활성화 상태로 설정
    setCurrentTrack(currentTrack);
    settings.sync();  // 설정 동기화

    audioProcesses[currentTrack.toStdString()] = process;

    qDebug() << __FUNCTION__ << " - Audio Playing: " << currentTrack;
    
}

void AudioHandler::playEffectSound(const std::string& filePath) {
    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << __FUNCTION__ << " - Effect sound is already playing: " << QString::fromStdString(filePath);
        return;
    }

    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);
    QStringList arguments;
    arguments << "-Dhw:0,0" << qFilePath;

    process->startDetached("./aplay", arguments);

    qDebug() << __FUNCTION__ << " - Playing effect sound: " << QString::fromStdString(filePath);

    QTimer::singleShot(5000, this, [process]() {
        process->terminate();
        delete process;
    });
}

void AudioHandler::stopAudio() {
    QString currentTrack = getCurrentTrack();
 
    if (currentTrack.isEmpty()) {
        qDebug() << __FUNCTION__ << " - No audio is currently playing.";
        return;
    }

    auto it = audioProcesses.find(currentTrack.toStdString());
    if (it != audioProcesses.end()) {
        QProcess* process = it->second;
        qDebug() << __FUNCTION__ << " - Killing process: " << currentTrack;
        process->kill();
        process->waitForFinished();

        disconnect(process, nullptr, this, nullptr);
        process->deleteLater();  // 안전한 삭제
        audioProcesses.erase(it);

        settings.remove("audio/currentTrack");
        setAudioStatus(false);
        settings.sync();

        qDebug() << __FUNCTION__ << " - Stopped: " << currentTrack;
    }
}

void AudioHandler::stopAllAudio() {
    for (auto& processPair : audioProcesses) {
        processPair.second->terminate();
        processPair.second->waitForFinished();
        delete processPair.second;
    }
    audioProcesses.clear();

    settings.remove("audio/currentTrack");
    setAudioStatus(false);
    settings.sync();

    qDebug() << __FUNCTION__ << " - Stopped all audio.";
}

QString AudioHandler::getCurrentTrack() const {
    QString currentTrack = settings.value("audio/currentTrack", "").toString();
    qDebug() << __FUNCTION__ << " - Current track: " << currentTrack;
    
    return currentTrack;
}

void AudioHandler::setAudioStatus(bool enabled) {
    settings.setValue("audio/enabled", enabled);
    settings.sync();
}

bool AudioHandler::isAudioOn() const {
    return settings.value("audio/enabled", true).toBool();
}

void AudioHandler::setCurrentTrack(const QString& trackName) {
    // 확장자 제거 (예: "magic.wav" -> "magic")
    QString trackBaseName = QFileInfo(trackName).baseName();

    // 설정에 트랙 이름(확장자 없는) 저장
    settings.setValue("audio/currentTrack", trackBaseName);
    settings.sync();  // 설정 동기화

    qDebug() << __FUNCTION__ << " - Current track set to: " << trackBaseName;
}

std::pair<QString, QString> AudioHandler::playNextTrack() {
    const QMap<QString, AudioData>& audioMap = getAudioMap();
    QString currentTrack = getCurrentTrack();
    
    // stop
    stopAudio();

    // get next key
    QList<QString> keys = audioMap.keys();
    int index = keys.indexOf(currentTrack);
    if (index == -1 || index + 1 >= keys.size()) {
        index = 0;
    } else {
        index++;
    }
    
    QString nextTrack = keys[index];
    setCurrentTrack(nextTrack);
    playAudio();

    qDebug() << __FUNCTION__ << " - Switching to next track: " << nextTrack;
    
    return {nextTrack, audioMap[nextTrack].iconPath};
}

int AudioHandler::getTrackDurationMs(const QString& filePath) {
    const QMap<QString, AudioData>& audioMap = getAudioMap();
    QString trackKey = QFileInfo(filePath).baseName();

    if (audioMap.contains(trackKey)) {
        return audioMap[trackKey].playBacktime;  // second -> ms 
    }

    return -1; // unknown duration
}


void AudioHandler::startLoopPlayback() {
    stopLoopPlayback(); // 기존 타이머 중지
    QString currentTrack = getCurrentTrack();

    int trackDurationMs = getTrackDurationMs(currentTrack);
    if (trackDurationMs <= 0) {
        qDebug() << __FUNCTION__ << " - Invalid track duration, skipping loop.";
        return;
    }

    loopTimer->start(trackDurationMs + 1000);  // 🎵 종료시간 +1초 후 재생
    connect(loopTimer, &QTimer::timeout, this, [this, currentTrack]() {
        qDebug() << __FUNCTION__ << " - Looping track: " << currentTrack;
        playAudio(); // 🔄 다시 재생
    });

    qDebug() << __FUNCTION__ << " - Loop playback scheduled every " << trackDurationMs + 1000 << "ms";
}

void AudioHandler::stopLoopPlayback() {
    if (loopTimer->isActive()) {
        loopTimer->stop();
        qDebug() << __FUNCTION__ << " - Loop playback stopped.";
    }
    stopAudio();
}

