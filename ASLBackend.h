#pragma once
#include <QObject>
#include <QTimer>
#include <QProcess>
#include <QString>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <windows.h>
#include <sapi.h>
#include <onnxruntime_cxx_api.h>

// ── GROUPMATE'S STRUCT: Sign ──
struct Sign {
    char  letter;
    char* word;
};

class ASLBackend : public QObject
{
    Q_OBJECT
public:
    explicit ASLBackend(QObject *parent = nullptr);
    ~ASLBackend();

public slots:
    void speak();
    void clearSentence();
    void toggleMode();

signals:
    void letterDetected(QString letter, double confidence);
    void sentenceUpdated(QString sentence);
    void dictionaryWordDetected(QString meaning);
    void trackingActive(bool on);
    void modeChanged(QString mode);

private slots:
    void poll();

private:
    const std::string CSV_PATH  = "C:\\ASL_PROJECT\\landmarks.csv";
    const std::string PRED_PATH = "C:\\ASL_PROJECT\\prediction.txt";
    const std::string LOG_PATH  = "C:\\ASL_PROJECT\\history.txt";

    // ── GROUPMATE'S CORE FUNCTIONS ──
    Sign* createSignLibrary();
    Sign* findSign(Sign* lib, int size, char letter);

    std::map<int,std::string> loadMapping(const std::string &path);
    std::vector<float>        readCSV();
    std::pair<int,float>      runModel(Ort::Session &session,
                                   std::vector<float> &input,
                                   int numClasses);
    std::string smoothLetter(const std::string &s);
    std::string smoothWord(const std::string &s);
    void        writePrediction(const std::string &s);
    void        logToHistory(const std::string &s);

    Ort::Env        m_env;
    Ort::MemoryInfo  m_memInfo;
    Ort::Session    *m_alphaSession = nullptr;
    Ort::Session    *m_wordSession  = nullptr;
    std::map<int,std::string> m_alphaMap;
    std::map<int,std::string> m_wordMap;

    // ── GROUPMATE'S INSTANTIATED LIBS ──
    Sign* m_signLib  = nullptr;
    int   m_libSize  = 27;

    ISpVoice *m_voice  = nullptr;
    QProcess *m_python = nullptr;
    QTimer    *m_timer  = nullptr;

    std::string m_sentence;
    std::string m_stableLetter;
    std::string m_lastAdded;
    std::vector<std::string> m_smoothHist;
    std::vector<std::string> m_wordSmoothHist;
    int  m_stableCount = 0;
    bool m_letterMode  = true;
    const int STABLE_THRESHOLD = 8;

    // ── FIXED: Missing State Flag ──
    bool m_tracking = false;
};