#include "TitleBar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(44);
    setStyleSheet("background:#16161A; border-radius:10px;");

    auto *lay = new QHBoxLayout(this);
    lay->setContentsMargins(18, 0, 14, 0);
    lay->setSpacing(10);

    // Left side labels
    auto lbl = [](const QString &text, const QString &style) {
        auto *l = new QLabel(text);
        l->setStyleSheet(style);
        return l;
    };

    lay->addWidget(lbl("ASL INTERPRETER", "color:#FFF; font-size:14px; letter-spacing:3px;"));

    auto *ver = lbl("v1.0", "background:#1E3A5F; color:#5B9BD5; border-radius:10px; font-size:10px;");
    ver->setFixedSize(38, 20);
    ver->setAlignment(Qt::AlignCenter);
    lay->addWidget(ver);

    lay->addStretch();

    // Blinking live dot
    auto *dot = lbl("●", "color:#22C55E; font-size:11px;");
    lay->addWidget(dot);

    auto *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [dot, state = true]() mutable {
        state = !state;
        dot->setVisible(state);
    });
    t->start(900);
}