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
#include <QTimer>

struct AudioData {
    QString filePath;
    QString iconPath;
    qreal playBacktime;
};

class AudioHandler : public QObject {
    Q_OBJECT

public:
    /* Singleton */ 
    static AudioHandler* getInstance() {
        static AudioHandler instance; 
        return &instance;
    }

    void playAudio();
    void playEffectSound(const std::string& filePath);
    void stopAudio();                   // set off / remove currentTrack / remove processMap / kill
    void stopAllAudio();
    QString getCurrentTrack() const;    // get audio/currentTrack
    void setCurrentTrack(const QString& trackName);
    void setAudioStatus(bool enabled);      // audio on / off setting
    bool isAudioOn() const;             // on / off return
    std::pair<QString, QString> playNextTrack();

    static const QMap<QString, AudioData>& getAudioMap();
    int getTrackDurationMs(const QString& filePath);

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

    /* Loop */
    QTimer* loopTimer;  // 🎵 루프 타이머
    int getTrackDurationMs(const std::string& filePath);
    void stopLoopPlayback();
    void startLoopPlayback();
};

#endif // AUDIOHANDLER_H
