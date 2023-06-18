#include "debug_box.h"
#include <QMessageBox>
#include <QString>

DebugBox::DebugBox(QString FUNCTION, int LINE, QString msg)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Debug message box");
    msgBox.setIcon(QMessageBox::Icon::Warning);

    QString info;
    info += "line number:" + QString::number(LINE) + QString("   ") + FUNCTION;
    info += ": " + msg;
    msgBox.setText(info);
    msgBox.exec();
}

void DebugBox::debugBoxSlot(QString FUNCTION, int LINE, QString msg){
    QMessageBox msgBox;
    msgBox.setWindowTitle("Debug message box");
    msgBox.setIcon(QMessageBox::Icon::Warning);

    QString info;
    info += "line number:" + QString::number(LINE) + QString("   ") + FUNCTION;
    info += ": " + msg;
    msgBox.setText(info);
    msgBox.exec();
}
