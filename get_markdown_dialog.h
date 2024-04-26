#ifndef GET_MARKDOWN_DIALOG_H
#define GET_MARKDOWN_DIALOG_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
class get_markdown_dialog;
}

class GetMarkdownDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetMarkdownDialog(QWidget *parent = nullptr);
    ~GetMarkdownDialog();

    void numOldMarkdownSpinBoxStatus(int flags);

    void initGetMarkdownDlg(QString repoPath, QString subDirPath, int width, int fileNum);
    void numNewMarkdownSpinBoxStatus(int flags);
    void showWindow();
    void initSize();
    bool copyMarkdownFile(QString oldfileAbs, QString newFileDir, QString fileName);
    bool copyAssetsFile(QString oldfileAbs, QString newFileDir);
    void updateMarkdownNum();
    bool isMarkdownFile(QString fileName);
public slots:
    void on_numOldMarkdownSpinBox_valueChanged(int fileNum);
    void on_fileDirPbn_clicked();

    void on_openTempPbn_clicked();
    void on_numNewMarkdownSpinBox_valueChanged(int fileNum);

    void on_yesPbn_clicked();

    void on_cancelPbn_clicked();
signals:
    void sigGetMarkdownFileLog(QString msg);
    void sigOpenTempleMarkdown(QString templateFileAbs);
    void sigCopyAssetsToExportDir(QString oldfileAbs, QString newFileDir, QString fileName);

protected:

    // 回车相当于单击ok button
    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            on_yesPbn_clicked();
        }
    }

private slots:
    void on_newFilePathEdit_textChanged(const QString &arg1);

private:
    Ui::get_markdown_dialog *ui;
    QString subDirPath_;
    QString repoPath_;
    int currentFileNum_{0};
    int width_;

    QString oldFileName_;
};

#endif // GET_MARKDOWN_DIALOG_H
