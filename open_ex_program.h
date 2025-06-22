#ifndef OPENEXPROGRAM_H
#define OPENEXPROGRAM_H

#include <QObject>
#include <QMap>
#include <QProcess>

class OpenExProgram: public QObject
{
    Q_OBJECT
public:
    OpenExProgram();
    void setMarkdownSoftWarePath(const QString& Path);
    void setDataDirSoftWarePath(const QString& Path);
    QString getPathByKey(QString key);
public slots:
    void OpenMarkdownAndDirSlot(QString fileName);
    void CompareFileSlot(QString fileNameA, QString fileNameB);
    void OpenJsonAndIniSlot(QString fileName);
    void OpenDirSlot(QString dirName);

signals:
    void sigOpenExProLog(QString log);
    void sigCompareFinished();
private:

    QString markdownSoftPath_;
    QString dataDirSoftPath_;

};

#endif // OPENEXPROGRAM_H
