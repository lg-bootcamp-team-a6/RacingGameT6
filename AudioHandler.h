#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QProcess>
#include <unordered_map>
#include <string>

class AudioHandler : public QObject {
    Q_OBJECT

public:
    // 싱글톤 인스턴스를 반환하는 함수
    static AudioHandler* getInstance() {
        static AudioHandler instance;  // 싱글톤 패턴
        return &instance;
    }

    // 오디오 재생 함수
    void playAudio(const std::string& filePath, bool loop = false);

    // effect sound
    void playEffectSound(const std::string& filePath);

    // 특정 오디오 파일 정지 함수
    void stopAudio(const std::string& filePath);

    // 모든 오디오 정지 함수
    void stopAllAudio();

    // 오디오가 재생 중인지 확인하는 함수
    bool isPlaying(const std::string& filePath) const;

private:
    std::unordered_map<std::string, QProcess*> audioProcesses;  // 오디오 파일과 관련된 QProcess 맵
    AudioHandler() = default;  // 기본 생성자
    ~AudioHandler();  // 소멸자

    AudioHandler(const AudioHandler&) = delete;  // 복사 생성자 삭제
    AudioHandler& operator=(const AudioHandler&) = delete;  // 대입 연산자 삭제
};

#endif // AUDIOHANDLER_H
