#ifndef RENAME_FILE_NAME_H
#define RENAME_FILE_NAME_H

#include <QDialog>
#include <QFileDialog>
#include <QMap>

#define NEW_FILE_NAME   QString("NewFileName.log")
#define OLD_FILE_NAME   QString("OldFileName.log")

namespace Ui {
class RenameFileName;
}

enum StatusType{
    OK, EditNameFileError,NamesNumberError
};

class RenameFileName : public QDialog
{
    Q_OBJECT

public:
    explicit RenameFileName(QWidget *parent = nullptr);
    ~RenameFileName();
    void setPath(const QString& path);
    bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist);
    bool readNewFileList();
    void initRenameFileList();
    void updateRenameFileList();
    void renameFileListClear();
    void updateMarkdownRefFileList();
    void getRefMarkdownFile(const QString& path, const QString &oldFileName);
    void getDirAllFiles(const QString &dirPath);
    bool isMarkdownFile(QString fileName);
    bool modifyRefMarkdown(const QString& refFilePath, const QString& oldFileName,
                           const QString& newFilePath);
    QString replaceNewFile(QDir newFileDir, QDir curFileDir, QString line, QString oldFileName);
private slots:
    void on_ChoosePbn_clicked();
    void on_ReplaceByListPbn_clicked();
    void on_ModifyNameFilePbn_clicked();
    void on_ReplaceByFilePbn_clicked();
    void on_OpenFilePathPbn_clicked();

signals:
    void sigRenameFileNameLog(QString log);
    void sigRenameFileVSCodeOpenList(QString pathA, QString pathB);
    void sigRenameFileOpenPath(QString path);
private:
    Ui::RenameFileName *ui;
    QString tarPath_;
    QString filePath_;
    QStringList oldFileAndDirNameList_;
    QStringList newFileAndDirNameList_;
    StatusType status_;
    QMap<QString, int> refFilePathAndNum_;
};

#endif // RENAME_FILE_NAME_H
