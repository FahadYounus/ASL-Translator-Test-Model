#include "CameraFeed.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QMediaDevices>

using namespace std;

CameraFeed::CameraFeed(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background:#0A0A0C; border:1px solid #2A2A35; border-radius:12px;");

    m_display = new QLabel(this);
    m_display->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_display->setStyleSheet("background:#0A0A0C;");

    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(1,1,1,1);
    lay->addWidget(m_display);

    // FIX: Start as false so the UI reads "○ NO SIGNAL" until Python actually starts tracking
    m_tracking = false;

    // Pulse the bounding box border
    auto *t = new QTimer(this);
    connect(t, &QTimer::timeout, this, [this](){
        m_pulse += m_pDir ? 0.07f : -0.07f;
        if (m_pulse > 2.5f) m_pDir = false;
        if (m_pulse < 1.0f) m_pDir = true;
        update();
    });
    t->start(30);

    // Python safely handles the hardware lock now
    // startCamera();
}

void CameraFeed::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Inform user video is rendering safely via our background service thread
    p.setPen(QColor(0, 212, 255, 120));
    p.setFont(QFont("Segoe UI", 11, QFont::DemiBold));
    p.drawText(rect(), Qt::AlignCenter,
               "AI ENGINE ACTIVE\nCheck External Live Feed Window");

    // Scanlines
    p.setPen(QPen(QColor(255,255,255,5),1));
    for (int y=0; y<height(); y+=4) p.drawLine(0,y,width(),y);

    if (m_tracking) drawBox(p);
    drawBrackets(p);

    // HUD
    QRect hud(12, height()-34, 108, 24);
    p.setPen(Qt::NoPen); p.setBrush(QColor(0,26,38,210));
    p.drawRoundedRect(hud,4,4);
    p.setPen(QColor(0,212,255));
    p.setFont(QFont("Segoe UI",9));
    p.drawText(hud, Qt::AlignCenter,
               m_tracking ? "● TRACKING" : "○ NO SIGNAL");
}

void CameraFeed::drawBrackets(QPainter &p)
{
    const int m=16, l=22;
    p.setPen(QPen(QColor(0,212,255), 2.f, Qt::SolidLine, Qt::RoundCap));
    for (int sx : {1,-1}) for (int sy : {1,-1}) {
            int x = sx==1 ? m : width()-m;
            int y = sy==1 ? m : height()-m;
            p.drawLine(x, y, x+sx*l, y);
            p.drawLine(x, y, x, y+sy*l);
        }
}

void CameraFeed::drawBox(QPainter &p)
{
    QRect box(m_rect.x()*width(), m_rect.y()*height(),
              m_rect.width()*width(), m_rect.height()*height());

    p.setPen(Qt::NoPen); p.setBrush(QColor(0,212,255,10));
    p.drawRoundedRect(box,6,6);
    p.setPen(QPen(QColor(0,212,255), m_pulse));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(box,6,6);

    if (m_letter.isEmpty()) return;

    // Letter badge above box
    QRect badge(box.center().x()-15, box.top()-32, 30, 28);
    p.setPen(Qt::NoPen); p.setBrush(QColor(0,212,255));
    p.drawRoundedRect(badge,6,6);
    p.setPen(Qt::black);
    p.setFont(QFont("Segoe UI",13,QFont::Bold));
    p.drawText(badge, Qt::AlignCenter, m_letter);
}