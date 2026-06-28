#include "ASLBackend.h"
#include <QDebug>
#include <algorithm>

using namespace std;

ASLBackend::ASLBackend(QObject *parent)
    : QObject(parent)
    // FORCE the wrapper constructor down to the lowest safe structural definition
    , m_env(ORT_LOGGING_LEVEL_WARNING, "ASL")
    , m_memInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))


{
    // Configure session options to run safely on a 1.17 library build
    Ort::SessionOptions opts;
    opts.SetIntraOpNumThreads(2);
    opts.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
    opts.DisableMemPattern();
    opts.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);

    try {
        m_alphaSession = new Ort::Session(m_env, L"C:\\ASL_PROJECT\\sign_model_pytorch.onnx", opts);
        m_wordSession  = new Ort::Session(m_env, L"C:\\ASL_PROJECT\\sign_model_words_pytorch.onnx", opts);
        qDebug() << "ONNX Models loaded successfully from C:\\ASL_PROJECT\\";
    }
    catch (const std::exception& e) {
        qDebug() << "ONNX session initialization failed:" << e.what();
        m_alphaSession = nullptr;
        m_wordSession = nullptr;
    }
    catch (...) {
        qDebug() << "ONNX models failed to load at C:\\ASL_PROJECT\\";
        m_alphaSession = nullptr;
        m_wordSession = nullptr;
    }

    m_alphaMap = loadMapping("C:\\ASL_PROJECT\\alphabet_mapping.json");
    m_wordMap  = loadMapping("C:\\ASL_PROJECT\\word_mapping.json");

    // Windows SAPI TTS
    CoInitialize(NULL);
    CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&m_voice);

    // Python setup
    m_python = new QProcess(this);
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONWARNINGS", "ignore");
    m_python->setProcessEnvironment(env);

    m_python->start("C:\\Users\\FFFFF\\AppData\\Local\\Programs\\Python\\Python311\\python.exe",
                    {"C:\\ASL_PROJECT\\data_pass.py"});

    if (!m_python->waitForStarted(3000)) {
        qDebug() << "Could not start data_pass.py";
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ASLBackend::poll);
    m_timer->start(30);
}

ASLBackend::~ASLBackend()
{
    m_timer->stop();
    if (m_python->state() != QProcess::NotRunning) {
        m_python->kill();
        m_python->waitForFinished(1000);
    }
    delete m_alphaSession;
    delete m_wordSession;
    if (m_voice) m_voice->Release();
    CoUninitialize();
}

void ASLBackend::poll()
{
    auto coords = readCSV();

    if (coords.size() < 63) {
        if (m_tracking) {
            qDebug() << "Hand lost";
            m_tracking = false;
        }
        emit trackingActive(false);
        m_stableLetter = "";
        m_stableCount  = 0;
        m_lastAdded    = "";
        m_smoothHist.clear();
        m_wordSmoothHist.clear(); // <-- Clear word history too when hand is lost
        return;
    }

    if (!m_tracking) {
        qDebug() << "Hand detected, coords:" << coords.size();
        m_tracking = true;
    }
    emit trackingActive(true);

    vector<float> input63(coords.begin(), coords.begin()+63);
    string prediction;

    if (m_letterMode) {
        if (!m_alphaSession || m_alphaMap.empty()) return;
        auto [idx, gap] = runModel(*m_alphaSession, input63, (int)m_alphaMap.size());
        if (idx < 0 || !m_alphaMap.count(idx)) return;
        prediction = smoothLetter(m_alphaMap[idx]); // <-- Correctly uses smoothLetter
        emit letterDetected(QString::fromStdString(prediction), gap);
    } else {
        if (!m_wordSession || m_wordMap.empty()) return;
        auto [idx, gap] = runModel(*m_wordSession, input63, (int)m_wordMap.size());
        if (idx < 0 || !m_wordMap.count(idx)) return;
        prediction = smoothWord(m_wordMap[idx]); // <-- FIXED: Now calls smoothWord
        emit letterDetected(QString::fromStdString(prediction), gap);
    }

    if (prediction == m_stableLetter) m_stableCount++;
    else { m_stableLetter = prediction; m_stableCount = 0; }

    if (m_stableCount == STABLE_THRESHOLD) {
        m_stableCount = 0;
        if (m_letterMode) {
            if (prediction == "space")
            { m_sentence += " "; m_lastAdded = "space"; }
            else if (prediction == "del")
            { if (!m_sentence.empty()) m_sentence.pop_back(); }
            else if (prediction != "nothing" &&
                     prediction != "No Hand" &&
                     prediction.length() == 1 &&
                     prediction != m_lastAdded)
            { m_sentence += prediction; m_lastAdded = prediction; }
        } else {
            if (prediction != "No Hand" && prediction != m_lastAdded)
            { m_sentence += prediction + " "; m_lastAdded = prediction; }
        }
        if (m_sentence.length() > 35)
            m_sentence = m_sentence.substr(m_sentence.length()-35);
        writePrediction(m_sentence);
        emit sentenceUpdated(QString::fromStdString(m_sentence));
    }
}

void ASLBackend::speak()
{
    if (m_sentence.empty() || !m_voice) return;
    logToHistory(m_sentence);
    wstring w(m_sentence.begin(), m_sentence.end());
    m_voice->Speak(w.c_str(), SPF_ASYNC|SPF_PURGEBEFORESPEAK, NULL);
}

void ASLBackend::clearSentence()
{
    m_sentence = ""; m_lastAdded = ""; m_stableCount = 0;
    writePrediction("");
    emit sentenceUpdated("");
}

void ASLBackend::toggleMode()
{
    m_letterMode = !m_letterMode;
    m_sentence = "";
    m_lastAdded = "";
    m_stableCount = 0;
    m_smoothHist.clear();
    m_wordSmoothHist.clear(); // <-- FIXED: Clear out word buffer completely
    writePrediction("");
    emit sentenceUpdated("");
    emit modeChanged(m_letterMode ? "LETTER" : "WORD");
    qDebug() << "Model switched! Current mode is now:" << (m_letterMode ? "Alphabet Mode" : "Word Mode");
}

vector<float> ASLBackend::readCSV()
{
    vector<float> coords;
    ifstream file(CSV_PATH);
    if (!file.is_open()) return coords;
    string line;
    if (getline(file, line) && !line.empty()) {
        stringstream ss(line);
        string val;
        while (getline(ss, val, ','))
            try { coords.push_back(stof(val)); } catch(...) {}
    }
    return coords;
}

pair<int,float> ASLBackend::runModel(Ort::Session &session,
                                      vector<float> &input,
                                      int numClasses)
{
    try {
        vector<int64_t> shape = {1,(int64_t)input.size()};
        Ort::Value tensor = Ort::Value::CreateTensor<float>(
            m_memInfo, input.data(), input.size(),
            shape.data(), shape.size());
        Ort::AllocatorWithDefaultOptions alloc;
        auto inN  = session.GetInputNameAllocated(0, alloc);
        auto outN = session.GetOutputNameAllocated(0, alloc);
        const char *i[] = {inN.get()}, *o[] = {outN.get()};
        auto out = session.Run(Ort::RunOptions{nullptr},
                               i, &tensor, 1, o, 1);
        float *d = out[0].GetTensorMutableData<float>();
        int n = min((int)out[0].GetTensorTypeAndShapeInfo()
                        .GetElementCount(), numClasses);
        int best=0; float bestV=d[0], second=-1e9f;
        for (int i=1; i<n; i++) {
            if (d[i]>bestV) { second=bestV; bestV=d[i]; best=i; }
            else if (d[i]>second) second=d[i];
        }
        return {best, bestV-second};
    } catch(...) { return {-1,0.f}; }
}

string ASLBackend::smoothLetter(const string &s)
{
    m_smoothHist.push_back(s);
    if ((int)m_smoothHist.size()>15)
        m_smoothHist.erase(m_smoothHist.begin());
    map<string,int> freq;
    for (auto &x : m_smoothHist) freq[x]++;
    string best=s; int mx=0;
    for (auto &p : freq)
        if (p.second>mx) { mx=p.second; best=p.first; }
    return best;
}
string ASLBackend::smoothWord(const string &s)
{
    m_wordSmoothHist.push_back(s);
    if ((int)m_wordSmoothHist.size() > 15)
        m_wordSmoothHist.erase(m_wordSmoothHist.begin());

    map<string, int> freq;
    for (auto &x : m_wordSmoothHist) freq[x]++;

    string best = s;
    int mx = 0;
    for (auto &p : freq) {
        if (p.second > mx) {
            mx = p.second;
            best = p.first;
        }
    }
    return best;
}


map<int,string> ASLBackend::loadMapping(const string &path)
{
    map<int,string> m;
    ifstream f(path);
    if (!f.is_open()) return m;
    string c((istreambuf_iterator<char>(f)),
             istreambuf_iterator<char>());
    size_t pos=0;
    while ((pos=c.find('"',pos))!=string::npos) {
        size_t ks=pos+1, ke=c.find('"',ks);
        if (ke==string::npos) break;
        string key=c.substr(ks,ke-ks); pos=ke+1;
        size_t col=c.find(':',pos);
        if (col==string::npos) break;
        size_t vs=c.find('"',col)+1, ve=c.find('"',vs);
        if (ve==string::npos) break;
        try { m[stoi(key)]=c.substr(vs,ve-vs); } catch(...) {}
        pos=ve+1;
    }
    return m;
}

void ASLBackend::writePrediction(const string &s)
{ ofstream f(PRED_PATH); f<<s; }

void ASLBackend::logToHistory(const string &s)
{ ofstream f(LOG_PATH,ios::app); if(!s.empty()) f<<s<<"\n"; }