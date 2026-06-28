#include "SignLibraryPanel.h"
#include <QVBoxLayout>
#include <QLabel>

using namespace std;

SignLibraryPanel::SignLibraryPanel(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background:#111116; border:1px solid #2A2A35; border-radius:12px;");

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(6);

    // Title
    auto *title = new QLabel("SIGN LIBRARY");
    title->setStyleSheet("color:#888899; font-size:10px; letter-spacing:2px;");
    root->addWidget(title);

    // Scrollable grid
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border:none; background:transparent;");
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto *container = new QWidget();
    container->setStyleSheet("background:transparent;");

    auto *grid = new QGridLayout(container);
    grid->setSpacing(8);
    grid->setContentsMargins(4, 4, 4, 4);

    const int COLS = 7;
    for (int i = 0; i < SIGN_COUNT; i++) {
        const SignEntry &e = SIGN_LIBRARY[i];

        auto *card = new QWidget();
        card->setFixedSize(90, 110);
        card->setStyleSheet(
            "background:#FFFFFF; border:1px solid #2A2A35; border-radius:8px;"
            );

        auto *lay = new QVBoxLayout(card);
        lay->setContentsMargins(4, 4, 4, 4);
        lay->setSpacing(2);
        lay->setAlignment(Qt::AlignCenter);

        auto *img = new QLabel();
        QPixmap pix(QString::fromStdString(e.imagePath));
        if (!pix.isNull())
            img->setPixmap(pix.scaled(54, 54, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            img->setText("?");
        img->setAlignment(Qt::AlignCenter);
        img->setStyleSheet("background:transparent; border:none;");

        auto *letter = new QLabel(e.letter == ' ' ? "SPC" : QString(e.letter));
        letter->setAlignment(Qt::AlignCenter);
        letter->setStyleSheet("color:#E8E8F0; font-size:13px; font-weight:bold; background:transparent; border:none;");

        auto *meaning = new QLabel(QString::fromStdString(e.meaning));
        meaning->setAlignment(Qt::AlignCenter);
        meaning->setStyleSheet("color:#22C55E; font-size:9px; background:transparent; border:none;");

        lay->addWidget(img);
        lay->addWidget(letter);
        lay->addWidget(meaning);

        grid->addWidget(card, i / COLS, i % COLS);
        m_cards[e.letter] = card;
    }

    scroll->setWidget(container);
    root->addWidget(scroll);
}

void SignLibraryPanel::setActiveLetter(char letter)
{
    for (auto *card : m_cards)
        card->setStyleSheet(
            "background:#1A1A22; border:1px solid #2A2A35; border-radius:8px;"
            );

    if (letter >= 'a' && letter <= 'z') letter -= 32;
    if (m_cards.contains(letter))
        m_cards[letter]->setStyleSheet(
            "background:#0D2A40; border:1.5px solid #00D4FF; border-radius:8px;"
            );
}