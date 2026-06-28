#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QCamera>
#include <QMediaCaptureSession>
#include <QVideoSink>

class CameraFeed : public QWidget
{
    Q_OBJECT
public:
    explicit CameraFeed(QWidget *parent = nullptr);
    void setDetectedLetter(const QString &l) { m_letter = l; update(); }
    void setTrackingRect(const QRectF &r)    { m_rect = r;   update(); }
    void setTracking(bool on)                { m_tracking=on; update(); }

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void startCamera();
    void drawBrackets(QPainter &p);
    void drawBox(QPainter &p);

    QLabel *m_display = nullptr;
    QCamera *m_cam = nullptr;
    QMediaCaptureSession *m_session = nullptr;
    QVideoSink *m_sink = nullptr;

    QString m_letter;
    QRectF  m_rect     = {0.2, 0.25, 0.6, 0.5};
    bool    m_tracking = true;
    bool    m_camOk    = false;
    float   m_pulse    = 1.5f;
    bool    m_pDir     = true;
};