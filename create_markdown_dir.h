#ifndef CREATE_MARKDOWN_DIR_H
#define CREATE_MARKDOWN_DIR_H

#include <QDialog>

namespace Ui {
class CreateMarkdown;
}

class CreateMarkdownAndSubDir : public QDialog
{
    Q_OBJECT

public:
    explicit CreateMarkdownAndSubDir(QWidget *parent = nullptr);
    ~CreateMarkdownAndSubDir();
    bool isMarkdownFile(QString fileName);

    void setSubDirPath(const QString & path) {
        subDirPath_ = path;
    }

    void setRepoPath(const QString & path) {
        repoPath_ = path;
    }
    void showWindow();
    void updateMarkdownWgt();
    void numOldMarkdownSpinBoxStatus(int flags);
    void numNewMarkdownSpinBoxStatus(int flags);
    void numDirSpinBoxStatus(int flags);
    void updateSubDirWgt();
    bool createMarkdown(QString &path);
    bool createSubDir(QString &path);
    void initSize();

    void setWidth(int res) {
        width_ = res;
    }

private slots:

    void on_fileRadioBtn_clicked(bool checked);

    void on_dirRadioBtn_clicked(bool checked);

    void on_numOldMarkdownSpinBox_valueChanged(int arg1);

    void on_numSubDirSpinBox_valueChanged(int arg1);

    void on_numNewMarkdownSpinBox_valueChanged(int arg1);

    void on_yesPbn_clicked();

    void on_cancelPbn_clicked();

    void on_openTempPbn_clicked();

signals:
    void sigCreateMarkdownAndDirLog(QString msg);
    // 0: 不用创建  1: Markdown文件   2: 子目录文件夹
    void sigCreateType(int status, QString namePathAbs);
    void sigOpenTempleMarkdown(QString namePathAbs);

private:
    Ui::CreateMarkdown *ui;
    QString subDirPath_;
    QString repoPath_;
    int width_;
};

#endif // CREATE_MARKDOWN_DIR_H
