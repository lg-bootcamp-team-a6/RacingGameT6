#include "AudioHandler.h"
#include <QDebug>
#include <QTimer>
#include <QObject>
#include <QFileInfo>
#include <QIcon>
#include <QPointer>

AudioHandler::AudioHandler() 
    : settings("RacingGameT6", "AudioHandler")
{
    qDebug() << "AudioHandler initialized.";
    settings.setValue("audio/enabled", true);
    settings.setValue("audio/currentTrack", "cookie.wav");
    settings.sync();
}

AudioHandler::~AudioHandler() {
    stopAllAudio();
}

const QMap<QString, AudioData>& AudioHandler::getAudioMap() {
    static QMap<QString, AudioData> audioMap = {
        {"cookie", {"cookie.wav", ":/images/cookie.png"}},
        {"july", {"july.wav", ":/images/july.png"}},
        {"dear", {"dear.wav", ":/images/dear.png"}},
        {"magic", {"magic.wav", ":/images/magic.png"}},
        {"walk", {"walk.wav", ":/images/walk.png"}}
    };
    return audioMap;
}

void AudioHandler::playAudio(const std::string& filePath, bool loop) {
    stopAudio();

    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << __FUNCTION__ << " - Audio is already playing: " << QString::fromStdString(filePath);
        return;
    }

    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);
    if (!qFilePath.endsWith(".wav", Qt::CaseInsensitive)) {
        qFilePath += ".wav";
    }
    QStringList arguments;
    arguments << "-Dhw:0,0" << qFilePath;

    qDebug() << __FUNCTION__ << " - Executing command: ./aplay with arguments:" << arguments;
    
    process->start("./aplay", arguments);

    settings.setValue("audio/currentTrack", qFilePath);
    setAudioOn(true);
    settings.sync();

    audioProcesses[filePath] = process;

    qDebug() << __FUNCTION__ << " - Audio Playing: " << qFilePath;
    
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
    settings.sync();

    qDebug() << __FUNCTION__ << " - Stopped all audio.";
}

QString AudioHandler::getCurrentTrack() const {
    QString currentTrack = settings.value("audio/currentTrack", "").toString();
    qDebug() << __FUNCTION__ << " - Current track: " << currentTrack;
    
    return currentTrack;
}

void AudioHandler::setAudioOn(bool enabled) {
    settings.setValue("audio/enabled", enabled);
    settings.sync();

    if (!enabled) {
        stopAllAudio();
    }
}

bool AudioHandler::isAudioOn() const {
    return settings.value("audio/enabled", true).toBool();
}

std::pair<QString, QString> AudioHandler::playNextTrack() {
    const QMap<QString, AudioData>& audioMap = getAudioMap();
    QString currentTrack = getCurrentTrack();
    QString currentTrackKey = QFileInfo(currentTrack).baseName();
    
    QList<QString> keys = audioMap.keys();
    int index = keys.indexOf(currentTrackKey);
    if (index == -1 || index + 1 >= keys.size()) {
        index = 0;
    } else {
        index++;
    }
    
    QString nextTrackKey = keys[index];
    QString nextTrack = audioMap[nextTrackKey].filePath;  // 첫 번째 값이 파일명 (예: "cookie.wav")
    playAudio(nextTrack.toStdString());

    qDebug() << __FUNCTION__ << " - Switching to next track: " << nextTrack;
    
    return {nextTrack, audioMap[nextTrackKey].iconPath};
}