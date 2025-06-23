#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QStandardPaths>
#include <QScreen>
#include <QRect>
#include <QString>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFont font = a.font();
    font.setPointSize(10);
    a.setFont(font);

    MainWindow w;
    w.setIconByConf();
    w.showSlot();

    return a.exec();
}
