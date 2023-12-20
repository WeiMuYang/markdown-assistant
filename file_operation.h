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
    bool clipFilesByFileInfo(const QStringList addList, QVector<ImgData> fileInfoVec, QString fullTarPath,int fileNum, QString &clipText);
    bool getFileNameByNum(QString fullPath, int fileNum, QString& fileName);
    void getDirAllFiles(const QString &oldFilePath);
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
    QString makeDirPath(QString dirPath);

    void getHistoryFileList(const QString &dirPath, QFileInfoList& fileListTop20);
    void getHistorySubDirFileList(const QString &dirPath, QFileInfoList& fileListTop20);
    void getLastmodifiedTop20Files(const QString& path, QFileInfoList& fileListTemp);
    bool createMarkdownFile(const QString& FullPath, QString& currentFileName);
    bool createJsonFile(const QString& FullPath, QString& currentFileName);
    void getSearchDirFiles(const QString& path, QFileInfoList& fileListTemp, QString txt);
    void getSearchFileList(const QString &dirPath, QFileInfoList& fileList, QString txt);
    bool getReadMePath(QString dirPath, QString& markdownFile);
    bool delDesktopFile(QString dirPath, QString fileName);

    ////////   renamefile    /////////
    void getMarkdownQString(const QString& markdownAbsPath, const QString &repoPath, const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList
                                           , QVector<FileRenameInfo> &replaceNameFileInfoList, int index = -1);
    void getRefMarkdownFile(const QString& subPath, const QString &repoPath, const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList
                                           , QVector<FileRenameInfo> &replaceNameFileInfoList,int index = -1);
    void updateReplaceNameRefereceList(const QString &repoPath, const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList,
                                                      QVector<FileRenameInfo> &replaceNameFileInfoList,int index = -1);

    void updateReplaceNameByList(const QString &listPathAbs, QVector<DirRenameInfo> &replaceNameDirInfoList
                                       , QVector<FileRenameInfo> &replaceNameFileInfoList);
    void findReferByJump(QDir newFileDir, QDir curFileDir, QString context, QString oldFileAbsPath,QVector<ReText>& reTextList);
    void findReferBySrc(QDir newFileDir, QDir curFileDir, QString context, QString oldFileAbsPath,QVector<ReText>& reTextList);
    void updateReferDir(const QString& markdownAbsPath, const QString &repoPath,const QString &context,
                                       QDir curDir,const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList,int i);
    void updateReferFile(const QString& markdownAbsPath, const QString &repoPath,const QString &context,
                                        QDir curDir,const QString &renameDirPath, QVector<FileRenameInfo> &replaceNameFileInfoList, int i);


    void updateReferDirByFilePath(const QString& markdownAbsPath, const QString &repoPath,
                                       QDir curDir,const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList,int i);
    void updateReferFileByFilePath(const QString& markdownAbsPath, const QString &repoPath,
                                        QDir curDir,const QString &renameDirPath, QVector<FileRenameInfo> &replaceNameFileInfoList, int i);
signals:
    void sigFileOperationLog(QString log);

private:
    QFileInfoList fileList_;
    QStringList assetTypes_;
};

#endif // FILEOPERATION_H
