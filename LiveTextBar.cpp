#include "LiveTextBar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QClipboard>
#include <QTextCursor>

using namespace std;

LiveTextBar::LiveTextBar(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background:#111116; border:1px solid #2A2A35; border-radius:10px;");
    m_tts = new QProcess(this);

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(14,10,12,10);
    lay->setSpacing(10);

    // Section label
    auto *lbl = new QLabel("LIVE\nOUTPUT");
    lbl->setStyleSheet("color:#444455; font-size:9px; letter-spacing:2px;");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setFixedWidth(50);
    lay->addWidget(lbl);

    // Text box
    m_text = new QTextEdit(this);
    m_text->setReadOnly(true);
    m_text->setPlaceholderText("Waiting for hand signs...");
    m_text->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_text->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_text->setStyleSheet(R"(
    QTextEdit {
        background: #0A0A0E;
        color: #E8E8F0;
        border: 0.5px solid #222230;
        border-radius: 6px;
        font-size: 17px;
        font-family: 'Consolas', monospace;
        font-weight: 300;
        padding: 0 10px;
        selection-background-color: #0D2A40;
        selection-color: #00D4FF;
    }
    QTextEdit:focus { border: 0.5px solid #00D4FF; }
    )");
    lay->addWidget(m_text, 1);

    // Vertical button column on the right
    auto *btnCol = new QVBoxLayout();
    btnCol->setSpacing(6);
    btnCol->setContentsMargins(0,0,0,0);

    auto btn = [&](const QString &label, const QString &color) {
        auto *b = new QPushButton(label, this);
        b->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        b->setFixedWidth(72);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(QString(R"(
        QPushButton { color:%1; background:#061822; border:1px solid %1;
                      border-radius:8px; font-size:12px; }
        QPushButton:hover   { background:#0A2A40; }
        QPushButton:pressed { background:%1; color:#000; }
    )").arg(color));
        btnCol->addWidget(b);
        return b;
    };

    auto *ttsBtn  = btn("▶  TTS", "#00D4FF");
    auto *clrBtn  = btn("CLR",    "#555566");
    auto *copyBtn = btn("COPY",   "#555566");

    // ── Connect buttons ───────────────────────────────────────────
    connect(ttsBtn, &QPushButton::clicked, this, [this](){
        emit ttsRequested();
    });

    connect(clrBtn, &QPushButton::clicked, this, [this](){
        m_text->clear();
        emit clearRequested();
    });

    connect(copyBtn, &QPushButton::clicked, this, [this](){
        QApplication::clipboard()->setText(m_text->toPlainText());
    });

    lay->addLayout(btnCol);
}

void LiveTextBar::setText(const QString &text)
{
    m_text->setPlainText(text);
    auto c = m_text->textCursor();
    c.movePosition(QTextCursor::End);
    m_text->setTextCursor(c);
}