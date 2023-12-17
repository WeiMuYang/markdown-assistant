#ifndef RENAME_FILE_NAME_H
#define RENAME_FILE_NAME_H

#include <QDialog>
#include <QFileDialog>
#include <QMap>
#include <QTableWidgetItem>
#include "file_operation.h"
#include "conf_dialog.h"
#include "open_ex_program.h"
#include "data_type.h"

#define NEW_FILE_NAME   QString("NewFileName.log")
#define OLD_FILE_NAME   QString("OldFileName.log")

namespace Ui {
class RenameFileName;
}
// QFileInfoList


class RenameFileName : public QDialog
{
    Q_OBJECT

public:
    explicit RenameFileName(QWidget *parent = nullptr);
    ~RenameFileName();
    void setRenameDirPath(const QString& path);
    void setRepoPath(const QString& path);
    void setRenameConfPath(const QString& path);

    void renameByConfFile();
    void renameByListFile();
    bool BackUp(const QString &fromDir, const QString &toDir, bool coverFileIfExist);
    bool updateOldPath(const QString &fromDir, QStringList& oldDirPathList,  QStringList& oldFilePathList);
    bool readNewFileList();
    void updateRenameFileList();
    void renameDirPathByCharMap(const QStringList &oldDirPathList, QStringList & newDirPathList);
    void renameFilePathByCharMap(const QStringList &oldDirPathList, QStringList & newDirPathList);
    void createCompareDirAndFileLog(const QString& path, const QStringList& DirPathList, const QStringList& FilePathList);

    void renameListClear();
    void initRenameFileList();

    void updateReferFileList();
    void updateReferListByRenameFileListRow(int row);
    void updateReferList(const QVector<ReFile>& reFileList);
public slots:
    void updateReplaceInfoSlot();

private slots:
    void on_ChooseDirPbn_clicked();
    void on_ChooseConfPbn_clicked();
    void on_OpenFilePathPbn_clicked();
    void on_OpenConfFilePbn_clicked();
    void on_EditNameFilePbn_clicked();
//    void updateReferListEnteredSlot(QTableWidgetItem *item);

    void updateReferListClickedSlot(QModelIndex index);
    void on_ReplaceByListPbn_clicked();

signals:
    void sigRenameFileNameLog(QString log);
    void sigRenameFileVSCodeOpenList(QString pathA, QString pathB);
    void sigRenameFileOpenPath(QString path);
    void sigRenameFileConfFile(QString path);
private:
    Ui::RenameFileName *ui;
    QString repoPath_;          // 仓库目录
    QString renameDirPath_;     // 选择的路径
    QString renameConfPath_;    // 配置文件
    QMap<QString, QString> replaceCharMap_; // replace符号表
    QVector<QString> replaceListVec_;       // replace修改列表 TODO
    QVector<DirRenameInfo> replaceNameDirInfoList_; // 更新后目录列表
    QVector<FileRenameInfo> replaceNameFileInfoList_; // 更新后文件名列表
    FileOperation* fileOperaton_;
    confDialog* confFile_;
    OpenExProgram* openExPro_;
    QString backUpPath_;
};

#endif // RENAME_FILE_NAME_H
