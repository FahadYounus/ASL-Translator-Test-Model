#include "MainWindow.h"
#include "TitleBar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTimer>    // Added for polling
#include <QWindow>   // Added to wrap the native window
#include <QDebug>    // Added for error logging
#include <windows.h> // Added for native API calls

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    auto *central = new QWidget(this);
    central->setStyleSheet("background:#0D0D0F;");
    setCentralWidget(central);

    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    root->addWidget(new TitleBar(this));

    auto *mid = new QHBoxLayout();

    // ── 1. CREATE A STABLE CAMERA CONTAINER ──
    // We put the camera in a dedicated layout so we can swap it out later without breaking the UI.
    auto *cameraContainer = new QWidget(this);
    cameraContainer->setFixedSize(560, 420);
    auto *cameraLayout = new QVBoxLayout(cameraContainer);
    cameraLayout->setContentsMargins(0, 0, 0, 0);

    m_camera = new CameraFeed(this);
    cameraLayout->addWidget(m_camera);
    mid->addWidget(cameraContainer);

    m_signLib = new SignLibraryPanel(this);
    mid->addWidget(m_signLib, 1);
    root->addLayout(mid, 1);

    // ── MODEL TOGGLE SWITCH UI ──
    auto *toggleBtn = new QPushButton("CURRENT MODE: LETTER (CLICK TO SWITCH)", this);
    toggleBtn->setFixedHeight(35);
    toggleBtn->setStyleSheet(
        "QPushButton {"
        "   background-color: #1A1A1F;"
        "   color: #A0A0AA;"
        "   border: 1px solid #27272A;"
        "   border-radius: 6px;"
        "   font-weight: bold;"
        "   font-size: 11px;"
        "   letter-spacing: 1px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #27272A;"
        "   color: #FFFFFF;"
        "   border-color: #3F3F46;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #0D0D0F;"
        "}"
        );
    root->addWidget(toggleBtn);

    m_liveBar = new LiveTextBar(this);
    m_liveBar->setFixedHeight(150);
    root->addWidget(m_liveBar);

    // INITIALIZE BACKEND
    m_backend = new ASLBackend(this);

    // CONNECT TRACKING HUD
    connect(m_backend, &ASLBackend::trackingActive, m_camera, &CameraFeed::setTracking);

    // CONNECT MODEL PREDICTIONS
    connect(m_backend, &ASLBackend::letterDetected, this, [this](QString letter, double confidence) {
        if (!letter.isEmpty()) {
            std::string s = letter.toStdString();
            m_signLib->setActiveLetter(s[0]);
            m_camera->setDetectedLetter(letter);
        }
    });

    // CONNECT LIVE OUTPUT SENTENCE
    connect(m_backend, &ASLBackend::sentenceUpdated, m_liveBar, &LiveTextBar::setText);

    // CONNECT INTERFACE BUTTONS
    connect(m_liveBar, &LiveTextBar::clearRequested, m_backend, &ASLBackend::clearSentence);
    connect(m_liveBar, &LiveTextBar::ttsRequested, m_backend, &ASLBackend::speak);

    // CONNECT TOGGLE MODE
    connect(toggleBtn, &QPushButton::clicked, m_backend, &ASLBackend::toggleMode);

    connect(m_backend, &ASLBackend::modeChanged, this, [toggleBtn](QString newMode) {
        toggleBtn->setText("CURRENT MODE: " + newMode + " (CLICK TO SWITCH)");
    });

    // ── 2. ROBUST WINDOW POLLING MECHANISM ──
    // Instead of checking just once, check every 500ms until the window is found.
    QTimer *embedTimer = new QTimer(this);
    connect(embedTimer, &QTimer::timeout, this, [this, embedTimer, cameraLayout]() {
        static int retries = 0;

        // Search the OS for the exact window title
        HWND pythonHwnd = FindWindowA(NULL, "Hand Tracker");

        if (pythonHwnd) {
            embedTimer->stop();

            // 1. Strip the OS title bar and borders
            LONG style = GetWindowLong(pythonHwnd, GWL_STYLE);
            SetWindowLong(pythonHwnd, GWL_STYLE, style & ~WS_CAPTION & ~WS_THICKFRAME);

            // 2. ── ADDED THIS LINE HERE TO FIX THE OFFSETS ──
            SetWindowPos(pythonHwnd, nullptr, 0, 0, 560, 420,
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

            // 3. Wrap the native window into a Qt Window representation
            QWindow *foreignWindow = QWindow::fromWinId((WId)pythonHwnd);

            // 4. Create the widget container
            QWidget *embeddedWindowWidget = QWidget::createWindowContainer(foreignWindow, this);
            embeddedWindowWidget->setFixedSize(560, 420);

            // Remove the default C++ CameraFeed and inject the Python one
            cameraLayout->removeWidget(m_camera);
            m_camera->hide();
            cameraLayout->addWidget(embeddedWindowWidget);

            qDebug() << "Successfully swallowed 'Hand Tracker' window and snapped to grid!";
        } else if (++retries > 20) {
            embedTimer->stop();
            qDebug() << "Timeout: Could not locate 'Hand Tracker' window.";
        }
    });

    embedTimer->start(500);
}