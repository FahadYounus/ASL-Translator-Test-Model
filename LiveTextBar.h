#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QProcess>

class LiveTextBar : public QWidget
{
    Q_OBJECT
public:
    explicit LiveTextBar(QWidget *parent = nullptr);
    void setText(const QString &text);

signals:
    void ttsRequested();      // ← add this
    void clearRequested();
    // void modeToggled();

private:
    QTextEdit *m_text = nullptr;
    QProcess  *m_tts  = nullptr;
};