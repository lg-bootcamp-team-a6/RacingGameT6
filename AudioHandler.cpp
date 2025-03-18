#include "AudioHandler.h"
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QFileInfo>
#include <QIcon>
#include <QPointer>
#include <QEventLoop>


AudioHandler::AudioHandler() 
    : settings("RacingGameT6", "AudioHandler"),
    loopTimer(new QTimer(this))  // ✅ 여기에서 생성됨
{
    setAudioStatus(true);
    setCurrentTrack("cookie");
    settings.sync();

    qDebug() << "AudioHandler initialized.";
}

AudioHandler::~AudioHandler() {
    stopAllAudio();
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

        loopTimer->stop();  // 🎯 반복 재생 중지

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
    setAudioStatus(true);
    setCurrentTrack(nextTrack);
    loopAudio();

    qDebug() << __FUNCTION__ << " - Switching to next track: " << nextTrack;
    
    return {nextTrack, audioMap[nextTrack].iconPath};
}

void AudioHandler::loopAudio() {
    setAudioStatus(true);
    QString currentTrack = getCurrentTrack();
    const QMap<QString, AudioData>& audioMap = getAudioMap();

    if (!audioMap.contains(currentTrack)) {
        qDebug() << __FUNCTION__ << " - Invalid track, stopping loop.";
        return;
    }

    int durationMs = audioMap[currentTrack].playBacktime * 1000; // 초 -> 밀리초 변환

    /* Existing process removal */
    auto it = audioProcesses.find(currentTrack.toStdString());
    if (it != audioProcesses.end()) {
        QProcess* oldProcess = it->second;
        qDebug() << __FUNCTION__ << " - Stopping existing process for: " << currentTrack;
        oldProcess->kill();
        oldProcess->waitForFinished();
        delete oldProcess;
        audioProcesses.erase(it);
    }

    playAudio();

    loopTimer->start(durationMs);
    connect(loopTimer, &QTimer::timeout, this, &AudioHandler::loopAudio, Qt::UniqueConnection);

    qDebug() << __FUNCTION__ << " - Looping track: " << currentTrack;
}

