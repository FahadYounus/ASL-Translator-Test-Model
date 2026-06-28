#pragma once
#include <QMainWindow>
#include "CameraFeed.h"
#include "LiveTextBar.h"
#include "SignLibraryPanel.h"
#include "ASLBackend.h"          // ← add

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    CameraFeed       *m_camera  = nullptr;
    LiveTextBar      *m_liveBar = nullptr;
    SignLibraryPanel *m_signLib = nullptr;
    ASLBackend       *m_backend = nullptr;   // ← add
};