#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QMap>
#include "sign_library.h"

class SignLibraryPanel : public QWidget
{
    Q_OBJECT
public:
    explicit SignLibraryPanel(QWidget *parent = nullptr);
    void setActiveLetter(char letter);

private:
    QMap<char, QWidget*> m_cards;   // letter → card widget for highlighting
};