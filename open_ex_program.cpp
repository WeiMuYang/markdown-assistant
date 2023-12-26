#include "open_ex_program.h"
#include <QDesktopServices>
#include <QUrl>

OpenExProgram::OpenExProgram()
{

}

void OpenExProgram::setMarkdownSoftWarePath(const QString& path)
{
    markdownSoftPath_ = path;
}

void OpenExProgram::setDataDirSoftWarePath(const QString& path)
{
    dataDirSoftPath_ = path;
}


void OpenExProgram::OpenMarkdownAndDirSlot(QString fileName){
    QProcess* pProcess = new QProcess;
    if(fileName.size() > 3 && fileName.right(3) == ".md"){
        if(!markdownSoftPath_.isEmpty()){
            pProcess->start(markdownSoftPath_, QStringList(fileName));
        }
    }else{
        if(!dataDirSoftPath_.isEmpty()){
            pProcess->start(dataDirSoftPath_, QStringList(fileName));
        }
    }
}

void OpenExProgram::CompareFileSlot(QString fileNameA, QString fileNameB){
    QProcess* pProcess = new QProcess;
    if(!dataDirSoftPath_.isEmpty()){
        QStringList list;
        list.append("--diff");
        list.append(fileNameA);
        list.append(fileNameB);
        pProcess->start(dataDirSoftPath_, list);
        pProcess->waitForFinished();
        emit sigCompareFinished();
    }
}

void OpenExProgram::OpenJsonAndIniSlot(QString fileName){
    QProcess* pProcess = new QProcess;
    if(!dataDirSoftPath_.isEmpty()){
        pProcess->start(dataDirSoftPath_, QStringList(fileName));
        pProcess->waitForFinished();
    }
}

void OpenExProgram::OpenDirSlot(QString dirName)
{
    dirName = QString("file:") +dirName;
    QDesktopServices::openUrl(QUrl(dirName, QUrl::TolerantMode));
}
