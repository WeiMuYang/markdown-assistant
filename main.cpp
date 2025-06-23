#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QStandardPaths>
#include <QScreen>
#include <QRect>
#include <QString>
#include <QApplication>

int screenInfo()
{
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect deskrect = screen->availableGeometry();

    return deskrect.width() * screen->devicePixelRatio();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    int width = screenInfo();
    if(width >= 3830){
        QFont font = a.font();
        font.setPointSize(13);
        a.setFont(font);
    }else{
        QFont font = a.font();
        font.setPointSize(11);
        a.setFont(font);
    }

    MainWindow w;
    w.setIconByConf();
    w.showSlot();

    return a.exec();
}
