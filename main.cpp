#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    MainWindow w;
    w.resize(1280, 720);
    w.show();
    return app.exec();
}