#ifndef GET_ASSETS_DIALOG_H
#define GET_ASSETS_DIALOG_H

#include <QDialog>
#include <QString>


namespace Ui {
class GetAssetsDialog;
}

class GetAssetsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetAssetsDialog(QWidget *parent = nullptr);
    ~GetAssetsDialog();
    bool isOKPressed(){
        return isOK;
    }
    QString getMarkdownCode();
    void setMarkdownCode(const QString& str);
    QString getRename();
    void setRename(const QString& str);

    QString getCurrentFullPath();
    void setCurrentFullPath(const QString& str);
    void initSize();

    void setWidth(int res) {
        width_ = res;
    }

signals:
    void sigGetAssetsDlgLogText(QString log);
    void sigSearchMarkdownCode(QString code,QString rename);

protected:

    void reject() override;
    void closeEvent(QCloseEvent *) override;

private slots:
    void on_okPbn_clicked();
    void on_noPbn_clicked();

private:
    Ui::GetAssetsDialog *ui;
    bool isOK{false};
    int width_;
};

#endif // GET_ASSETS_DIALOG_H
