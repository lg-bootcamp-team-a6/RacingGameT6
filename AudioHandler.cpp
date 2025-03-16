#include "AudioHandler.h"
#include <QDebug>
#include <QTimer>
#include <QObject>

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


void AudioHandler::playAudio(const std::string& filePath, bool loop) {
    // 기존에 재생 중인 프로세스가 있다면 정리 (memory leak 방지용)
    stopAudio();

    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << "Audio is already playing: " << QString::fromStdString(filePath);
        return;
    }

    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);
    QStringList arguments;
    arguments << "-Dhw:0,0" << qFilePath;

    qDebug() << "### Executing command: ./aplay with arguments:" << arguments;
    
    process->start("./aplay", arguments);

    // 현재 재생 중인 음악을 설정 파일에 저장
    settings.setValue("audio/currentTrack", qFilePath);
    settings.sync();
    
    if (loop) {
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                this, [this, filePath, loop]() {
                    qDebug() << "[Restarting] 반복재생 실행 중... " << QString::fromStdString(filePath);
                    playAudio(filePath, loop);
                });
    }

    audioProcesses[filePath] = process;

    qDebug() << "@@@@@@@@@@@@@@@ Audio Playing: " << qFilePath << " @@@@@@@@@@@@@@@@";
}

void AudioHandler::playEffectSound(const std::string& filePath) { // 효과음 처리
    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << "Effect sound is already playing: " << QString::fromStdString(filePath);
        return;
    }

    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);  // std::string -> QString 변환
    QStringList arguments;
    arguments << "-Dhw:0,0" << qFilePath;  // -Dhw:0,0 옵션과 오디오 파일 경로

    process->startDetached("./aplay", arguments);  // 비동기처리 (startDetached())

    qDebug() << "Playing effect sound: " << QString::fromStdString(filePath);

    QTimer::singleShot(1300, this, [process]() { // 프로세스 정리
        process->terminate();
        delete process;
    });
}

void AudioHandler::stopAudio() {
    QString currentTrack = getCurrentTrack();  // 🔹 현재 재생 중인 트랙 가져오기

    if (currentTrack.isEmpty()) {
        qDebug() << "No audio is currently playing.";
        return;
    }

    auto it = audioProcesses.find(currentTrack.toStdString());  // 🔹 현재 트랙이 실행 중인지 확인
    if (it != audioProcesses.end()) {
        disconnect(it->second, nullptr, this, nullptr);
        it->second->terminate();
        it->second->waitForFinished();
        delete it->second;
        audioProcesses.erase(it);

        // 🔹 설정에서 현재 트랙 정보 삭제
        settings.remove("audio/currentTrack");
        settings.sync();

        qDebug() << "Stopped: " << currentTrack;
    }
}

void AudioHandler::stopAllAudio() {
    for (auto& processPair : audioProcesses) {
        processPair.second->terminate();
        processPair.second->waitForFinished();
        delete processPair.second;
    }
    audioProcesses.clear();

    // 설정에서 모든 재생 정보 삭제
    settings.remove("audio/currentTrack");
    settings.sync();

    qDebug() << "Stopped all audio.";
}

QString AudioHandler::getCurrentTrack() const {
    return settings.value("audio/currentTrack", "").toString();
}

/* 전역 상태관리 */
void AudioHandler::setAudioOn(bool enabled) { // 오디오 상태 설정
    settings.setValue("audio/enabled", enabled);
    settings.sync();

    if (!enabled) {
        stopAllAudio();  // 오디오 비활성화 시 모든 음악 정지
    }
}

bool AudioHandler::isAudioOn() const { // 켜져있는지 여부 반환
    return settings.value("audio/enabled", true).toBool();
}