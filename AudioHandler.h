#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QProcess>
#include <QSettings> 
#include <unordered_map>
#include <string>

class AudioHandler : public QObject {
    Q_OBJECT

public:
    /* Singleton */ 
    static AudioHandler* getInstance() {
        static AudioHandler instance; 
        return &instance;
    }
    void playAudio(const std::string& filePath, bool loop = false);
    void playEffectSound(const std::string& filePath);
    void stopAudio(const std::string& filePath);
    void stopAllAudio();
    
    bool isPlaying(const std::string& filePath) const;
    bool isAudioOn() const;
    
    void setAudioOn(bool enabled);

signals:

private:
    AudioHandler() = default;  
    ~AudioHandler(); 

    /* Singleton */ 
    AudioHandler(const AudioHandler&) = delete;  
    AudioHandler& operator=(const AudioHandler&) = delete;

    /* Process */
    QSettings settings;
    QProcess* m_currentProcess; // 배경음악 처리
    std::unordered_map<std::string, QProcess*> audioProcesses;  // 효과음 처리
};

#endif // AUDIOHANDLER_H
