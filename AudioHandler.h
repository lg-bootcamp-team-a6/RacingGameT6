#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QProcess>
#include <QSettings> 
#include <unordered_map>
#include <string>
#include <memory>
#include <QMap>
#include <QString>

struct AudioData {
    QString filePath;
    QString iconPath;
};

class AudioHandler : public QObject {
    Q_OBJECT

public:
    /* Singleton */ 
    static AudioHandler* getInstance() {
        static AudioHandler instance; 
        return &instance;
    }

    void playAudio(const std::string& filePath, bool loop = true);
    void playEffectSound(const std::string& filePath);
    void stopAudio();  // 🔹 현재 재생 중인 음악을 자동으로 정지
    void stopAllAudio();
    QString getCurrentTrack() const; // 🔹 세미콜론 추가
    void setAudioOn(bool enabled);
    bool isAudioOn() const;
    std::pair<QString, QString> playNextTrack();
    static const QMap<QString, AudioData>& getAudioMap();

signals:

private:
    explicit AudioHandler();  // 🔹 explicit 추가
    ~AudioHandler(); 

    /* Singleton */ 
    AudioHandler(const AudioHandler&) = delete;  
    AudioHandler& operator=(const AudioHandler&) = delete;

    /* Process */
    QSettings settings;
    QProcess* m_currentProcess = nullptr;  // 🔹 안전한 초기화 추가
    std::unordered_map<std::string, QProcess*> audioProcesses;  // 프로세스 전체 처리
};

#endif // AUDIOHANDLER_H
