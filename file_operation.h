#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QString>
#include "data_type.h"
#include <QVector>
#include <QFileInfo>
#include <QDateTime>


class FileOperation : public QObject
{
    Q_OBJECT
public:
    explicit FileOperation(QObject *parent = nullptr);
    QFileInfo getLastmodifiedTimeFileName(const QString& path);
    int getLastmodifiedTimeFileNum(const QString &path, QString& fullPath,QString& tarFile);
    int getLastmodifiedTimeFileNumSubDir(const QString &path,const QString &dirName, QString& fullPath,QString& lastModefyFile);
    bool clipFilesByFileInfo(const QVector<ImgData> &fileInfoVec, QString fullTarPath,int fileNum, QString &clipText);
    bool getFileNameByNum(QString fullPath, int fileNum, QString& fileName);
    void getDirAllFiles(const QString &dirPath);
    QStringList getSubDirNames(QString path);
    bool isPathExist(const QString &path);

    void setAssetsTypes(QStringList types);

    static bool sortFileByInfo(const QFileInfo& file1, const QFileInfo& file2)
    {
        return file1.lastModified() < file2.lastModified();
    }
   //  bool getSearchResultFromMarkdownCode(const QString &currentFullPath,const QString &codeText,QString & searchResult);
    bool getSearchResultFromMarkdownCode(const QString &currentFullPath, const QString &codeText, const QString& name, QString &searchResult);
    QString newNumFileName(QString path, QString fileName, QString suffix);
    bool copyImgVideo(QDir& CurrentPath, const QStringList& fileNameArr,const QString & name, QString &searchResult);


    void getHistoryFileList(const QString &dirPath, QFileInfoList& fileListTop20);
//    bool compare(const QFileInfo &stu1, const QFileInfo &stu2);
    void getLastmodifiedTop20Files(const QString& path, QFileInfoList& fileListTemp);
    bool createMarkdownFile(const QString& FullPath);

signals:
    void sigFileOperationLog(QString log);

private:
    QFileInfoList fileList_;
    QStringList assetTypes_;
};

#endif // FILEOPERATION_H
