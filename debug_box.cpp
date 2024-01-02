#include "debug_box.h"
#include <QMessageBox>
#include <QString>
#include <QPushButton>
#include <QDesktopWidget>
#include <QApplication>

int DebugBox::printscreeninfo()
{
    QDesktopWidget *dwsktopwidget = QApplication::desktop();
    QRect deskrect = dwsktopwidget->availableGeometry();
    QRect screenrect = dwsktopwidget->screenGeometry();
    return deskrect.width();
}

DebugBox::DebugBox(QString FUNCTION, int LINE, QString msg)
{
    debugBoxSlot(FUNCTION, LINE, msg);
}

void DebugBox::debugBoxSlot(QString FUNCTION, int LINE, QString msg){
    QMessageBox msgBox;
    msgBox.setFixedSize(setWindowSize(printscreeninfo())); // 设置固定大小为400x200
    msgBox.setWindowTitle("Debug message box");
    msgBox.setIcon(QMessageBox::Icon::Warning);
    QPushButton* okButton = msgBox.addButton("OK", QMessageBox::ActionRole);
    // 设置按钮的大小为100x50
    okButton->setStyleSheet("QPushButton { min-width: 80px; min-height: 35px; }");

    QString info;
    info += "line number:" + QString::number(LINE) + QString("   ") + FUNCTION;
    info += ": " + msg;
    msgBox.setText(info);
    msgBox.exec();
}

QSize DebugBox::setWindowSize(int resWidth) {
    double widthIn4K = 900;
    double heightIn4K = 400;
    double zoom = 1;
    if(resWidth < 3840) {
        zoom = 1.1;
    }
    // 1000 870
    // 宽高比
    double WindowAspect = heightIn4K / widthIn4K;
    // 占屏比
    double Proportion = widthIn4K / 3840.0;
    // 宽 高
    int width = resWidth * Proportion ;
    int height = width * WindowAspect;

    return QSize(width, height) * zoom;
}

void DebugBox::helpBoxSlot(QString msg, int width)
{
    Q_UNUSED(width)
    QMessageBox msgBox;
    msgBox.setFixedSize(setWindowSize(printscreeninfo())); // 设置固定大小为400x200
    msgBox.setWindowTitle("Help message box");
    msgBox.setIcon(QMessageBox::Icon::Question);
    QPushButton* okButton = msgBox.addButton("OK", QMessageBox::ActionRole);
    // 设置按钮的大小为100x50
    okButton->setStyleSheet("QPushButton { min-width: 80px; min-height: 35px; }");
    QString info;
    info += msg;
    msgBox.setText(info);
    msgBox.exec();
}
