#include "AudioHandler.h"
#include <QDebug>
#include <QTimer>
#include <QObject>


AudioHandler::~AudioHandler() {
    stopAllAudio();
}

void AudioHandler::playAudio(const std::string& filePath, bool loop) {
    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << "Audio is already playing: " << QString::fromStdString(filePath);
        return;
    }

    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);  // std::string -> QString 변환
    QStringList arguments;
    arguments << "-Dhw:0,0";

    arguments << qFilePath;

    qDebug() << "Executing command: ./aplay with arguments:" << arguments;
    
    process->start("./aplay", arguments);
    
    if (loop) {
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
                this, [this, filePath, loop]() {
                    qDebug() << "Restarting audio for looping: " << QString::fromStdString(filePath);
                    playAudio(filePath, loop); // 다시 실행하여 무한 반복
                });
    }
    
    // Error handling
    QByteArray errorOutput = process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qDebug() << "Error Output:" << errorOutput;
    }

    qDebug() << "@@@@@@@@@@@@@@@ Audio Playing @@@@@@@@@@@@@@@@";
    audioProcesses[filePath] = process;
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

    QTimer::singleShot(1300, this, [this, process]() { // 프로세스 정리
        process->terminate();
        delete process;
    });
}

void AudioHandler::stopAudio(const std::string& filePath) {
    auto it = audioProcesses.find(filePath);
    if (it != audioProcesses.end()) {
        disconnect(it->second, nullptr, this, nullptr); // 루프 방지
        it->second->terminate();
        it->second->waitForFinished();
        delete it->second;
        audioProcesses.erase(it);
        qDebug() << "Stopped: " << QString::fromStdString(filePath);
    }
}

void AudioHandler::stopAllAudio() {
    for (auto& pair : audioProcesses) {
        disconnect(pair.second, nullptr, this, nullptr); // 루프 방지
        pair.second->terminate();
        pair.second->waitForFinished();
        delete pair.second;
    }
    audioProcesses.clear();
    qDebug() << "All audio stopped";
}

bool AudioHandler::isPlaying(const std::string& filePath) const {
    return audioProcesses.find(filePath) != audioProcesses.end();
}
