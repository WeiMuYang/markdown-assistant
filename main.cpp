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
    // 获取可用的显示区域
    QRect deskrect = screen->availableGeometry();
    // 获取整个屏幕的几何区域
    QRect screenrect = screen->geometry();
    // 获取屏幕数量
    int scrcount = QGuiApplication::screens().size();
    qCritical("screenrect.w==%s\n",qPrintable(QString::number(screenrect.width())));
    qCritical("screenrect.h==%s\n",qPrintable(QString::number(screenrect.height())));
    qCritical("deskrect.w==%s\n",qPrintable(QString::number(deskrect.width())));
    qCritical("deskrect.h==%s\n",qPrintable(QString::number(deskrect.height())));
    qCritical("scrcount==%s\n",qPrintable(QString::number(scrcount)));
    return deskrect.width();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    int width = screenInfo();
    if(width >= 3840){
        QFont font = a.font();
        font.setPointSize(13);
        a.setFont(font);
    }else{
        QFont font = a.font();
        font.setPointSize(11);
        a.setFont(font);
    }

    w.setIconByConf();
    w.showSlot();

    return a.exec();
}
