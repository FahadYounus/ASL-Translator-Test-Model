// sign_library.h
// ASL Sign Library for the Real-Time ASL Hand Sign Interpreter Project
// Uses: struct, arrays, functions
// Requires: Qt (QApplication, QLabel, QPixmap, QGridLayout, QScrollArea)
// Usage: #include "sign_library.h"

#ifndef SIGN_LIBRARY_H
#define SIGN_LIBRARY_H

#include <string>
#include <iostream>

#include <QApplication>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QScrollArea>
#include <QFrame>
#include <QString>
#include <QFont>

// ─────────────────────────────────────────────────────────────────────────────
// STRUCT: SignEntry
// Holds all data for one ASL sign
// ─────────────────────────────────────────────────────────────────────────────
struct SignEntry {
    char        letter;       // the ASL letter (e.g. 'A')
    std::string meaning;      // English meaning (e.g. "I")
    std::string imagePath;    // path to the PNG image (e.g. "signs/A.png")
};

// ─────────────────────────────────────────────────────────────────────────────
// ARRAY: SIGN_LIBRARY
// All 27 signs stored as a fixed array of SignEntry structs
// ─────────────────────────────────────────────────────────────────────────────
const int SIGN_COUNT = 27;

const SignEntry SIGN_LIBRARY[SIGN_COUNT] = {
    {'A', "I",         "signs/A.png"},
    {'B', "you",       "signs/B.png"},
    {'C', "he",        "signs/C.png"},
    {'D', "she",       "signs/D.png"},
    {'E', "they",      "signs/E.png"},
    {'F', "want",      "signs/F.png"},
    {'G', "need",      "signs/G.png"},
    {'H', "help",      "signs/H.png"},
    {'I', "go",        "signs/I.png"},
    {'J', "come",      "signs/J.png"},
    {'K', "stop",      "signs/K.png"},
    {'L', "eat",       "signs/L.png"},
    {'M', "drink",     "signs/M.png"},
    {'N', "yes",       "signs/N.png"},
    {'O', "no",        "signs/O.png"},
    {'P', "please",    "signs/P.png"},
    {'Q', "sorry",     "signs/Q.png"},
    {'R', "thank you", "signs/R.png"},
    {'S', "good",      "signs/S.png"},
    {'T', "bad",       "signs/T.png"},
    {'U', "home",      "signs/U.png"},
    {'V', "food",      "signs/V.png"},
    {'W', "water",     "signs/W.png"},
    {'X', "doctor",    "signs/X.png"},
    {'Y', "more",      "signs/Y.png"},
    {'Z', "finished",  "signs/Z.png"},
    {' ', "pain",      "signs/SPACE.png"}
};

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTION: findSign
// Searches the array for a sign by letter.
// Returns pointer to the SignEntry, or nullptr if not found.
// ─────────────────────────────────────────────────────────────────────────────
inline const SignEntry* findSign(char letter) {
    if (letter >= 'a' && letter <= 'z') letter -= 32;  // normalize to uppercase
    for (int i = 0; i < SIGN_COUNT; i++) {
        if (SIGN_LIBRARY[i].letter == letter) {
            return &SIGN_LIBRARY[i];
        }
    }
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTION: getMeaning
// Returns the English meaning of a sign letter.
// Returns empty string if not found.
// ─────────────────────────────────────────────────────────────────────────────
inline std::string getMeaning(char letter) {
    const SignEntry* entry = findSign(letter);
    if (entry != nullptr) return entry->meaning;
    return "";
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTION: makeCard  (internal helper)
// Builds one sign card widget: image + letter + meaning.
// imgSize controls how big the image is (90 for grid, 200 for single view).
// ─────────────────────────────────────────────────────────────────────────────
inline QWidget* makeCard(const SignEntry* entry, int imgSize = 90) {
    QWidget* card = new QWidget();
    card->setFixedSize(imgSize + 30, imgSize + 60);
    card->setStyleSheet(
        "background: white;"
        "border-radius: 14px;"
        "border: 1px solid #e0e0e0;"
        );

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(4);
    layout->setContentsMargins(8, 8, 8, 8);

    // Image
    QLabel* imgLabel = new QLabel();
    QPixmap pix(QString::fromStdString(entry->imagePath));
    if (!pix.isNull()) {
        imgLabel->setPixmap(
            pix.scaled(imgSize, imgSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)
            );
    } else {
        imgLabel->setText("?");
        imgLabel->setStyleSheet("font-size: 32px; color: #aaa;");
    }
    imgLabel->setAlignment(Qt::AlignCenter);

    // Letter
    QLabel* letterLabel = new QLabel(
        entry->letter == ' ' ? QString("SPC") : QString(entry->letter)
        );
    letterLabel->setAlignment(Qt::AlignCenter);
    QFont letterFont;
    letterFont.setPointSize(imgSize == 90 ? 18 : 36);
    letterFont.setBold(true);
    letterLabel->setFont(letterFont);
    letterLabel->setStyleSheet("color: #2c3e50; background: transparent; border: none;");

    // Meaning
    QLabel* meaningLabel = new QLabel(QString::fromStdString(entry->meaning));
    meaningLabel->setAlignment(Qt::AlignCenter);
    QFont meaningFont;
    meaningFont.setPointSize(imgSize == 90 ? 10 : 20);
    meaningFont.setBold(true);
    meaningLabel->setFont(meaningFont);
    meaningLabel->setStyleSheet("color: #27ae60; background: transparent; border: none;");

    layout->addWidget(imgLabel);
    layout->addWidget(letterLabel);
    layout->addWidget(meaningLabel);

    return card;
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTION: displaySign
// Opens a Qt popup window showing ONE sign's image, letter, and meaning.
// Call this when a sign is detected during the interpreter run.
// ─────────────────────────────────────────────────────────────────────────────
inline void displaySign(int argc, char* argv[], char letter) {
    const SignEntry* entry = findSign(letter);
    if (entry == nullptr) {
        std::cout << "[Unknown sign: " << letter << "]\n";
        return;
    }

    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle(
        QString("ASL Sign: ") + (letter == ' ' ? "SPACE" : QString(letter))
        );
    window.setFixedSize(320, 400);
    window.setStyleSheet("background-color: #f0f4f8;");

    QVBoxLayout* layout = new QVBoxLayout(&window);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(30, 30, 30, 30);

    QWidget* card = makeCard(entry, 200);
    card->setFixedSize(260, 340);
    layout->addWidget(card);

    window.show();
    app.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
// FUNCTION: displayAll
// Opens a Qt window showing ALL 27 signs as a scrollable grid.
// Good for testing the full library at once.
// ─────────────────────────────────────────────────────────────────────────────
inline void displayAll(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("ASL Sign Library - All Signs");
    window.setMinimumSize(750, 550);
    window.setStyleSheet("background-color: #f0f4f8;");

    QVBoxLayout* mainLayout = new QVBoxLayout(&window);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // Title
    QLabel* title = new QLabel("ASL Sign Library");
    title->setAlignment(Qt::AlignCenter);
    QFont titleFont;
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2c3e50; margin-bottom: 8px;");
    mainLayout->addWidget(title);

    // Scrollable area
    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("border: none; background: transparent;");

    QWidget* container = new QWidget();
    container->setStyleSheet("background: transparent;");
    QGridLayout* grid = new QGridLayout(container);
    grid->setSpacing(12);
    grid->setContentsMargins(8, 8, 8, 8);

    // 7 cards per row
    const int COLS = 7;
    int col = 0, row = 0;
    for (int i = 0; i < SIGN_COUNT; i++) {
        QWidget* card = makeCard(&SIGN_LIBRARY[i], 90);
        grid->addWidget(card, row, col);
        col++;
        if (col == COLS) { col = 0; row++; }
    }

    scroll->setWidget(container);
    mainLayout->addWidget(scroll);

    window.show();
    app.exec();
}

#endif // SIGN_LIBRARY_H
