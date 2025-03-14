#include "AudioHandler.h"
#include <QDebug>
#include <QTimer>

AudioHandler::~AudioHandler() {
    stopAllAudio();  // 소멸자에서 모든 오디오 중지
}

void AudioHandler::playAudio(const std::string& filePath, bool loop) {
    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << "Audio is already playing: " << QString::fromStdString(filePath);
        return;
    }

    // 새로운 오디오 재생 프로세스 생성
    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);  // std::string -> QString 변환
    QStringList arguments;
    arguments << "-Dhw:0,0" << qFilePath;  // -Dhw:0,0 옵션과 오디오 파일 경로

    // 오디오 파일을 반복 재생하려면 추가 인자 설정
    // if (loop) {
    //     arguments << "--loop";
    // }

    // 디버깅용 출력: 실행할 커맨드와 인자 출력
    qDebug() << "Executing command: ./aplay with arguments:" << arguments;
    
    process->start("./aplay", arguments);  // QProcess의 start()는 QString을 사용
    
    // 디버깅용 출력: 표준 오류 출력 읽기
    QByteArray errorOutput = process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qDebug() << "Error Output:" << errorOutput;
    }

    qDebug() << "@@@@@@@@@@@@@@@ Audio Playing @@@@@@@@@@@@@@@@";
    audioProcesses[filePath] = process;
}

void AudioHandler::playEffectSound(const std::string& filePath) {
    if (audioProcesses.find(filePath) != audioProcesses.end()) {
        qDebug() << "Effect sound is already playing: " << QString::fromStdString(filePath);
        return;
    }

    // 짧은 효과음을 위한 처리
    QProcess* process = new QProcess();
    QString qFilePath = QString::fromStdString(filePath);  // std::string -> QString 변환
    QStringList arguments;
    arguments << "-Dhw:0,0" << qFilePath;  // -Dhw:0,0 옵션과 오디오 파일 경로

    // 비동기적으로 실행하여 짧은 효과음을 재생
    process->startDetached("./aplay", arguments);  // startDetached()로 비동기 실행

    // 디버깅용 출력
    qDebug() << "Playing effect sound: " << QString::fromStdString(filePath);

    // 짧은 효과음은 바로 끝나므로, 잠시 후에 해당 프로세스를 정리
    QTimer::singleShot(1000, this, [this, process]() {
        process->terminate();
        delete process;
    });
}

void AudioHandler::stopAudio(const std::string& filePath) {
    auto it = audioProcesses.find(filePath);
    if (it != audioProcesses.end()) {
        it->second->terminate();
        it->second->waitForFinished();
        delete it->second;
        audioProcesses.erase(it);
        qDebug() << "Stopped: " << QString::fromStdString(filePath);
    }
}

void AudioHandler::stopAllAudio() {
    for (auto& pair : audioProcesses) {
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
