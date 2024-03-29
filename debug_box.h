#ifndef LOG_BOX_H
#define LOG_BOX_H

#include <QObject>
#include <QString>


class DebugBox:public QObject
{
    Q_OBJECT
public:
    DebugBox(QString FUNCTION, int LINE, QString msg);
    DebugBox(){}
    QSize setWindowSize(int width);
    int printscreeninfo();
public slots:
    void debugBoxSlot(QString FUNCTION, int LINE, QString msg);
    void helpBoxSlot(QString msg, int width);
};

#endif // LOG_BOX_H
