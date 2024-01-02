#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

    void setVersion(QString version) {
        version_ =  version;
    }

    void setWidth(int res) {
        width_ = res;
    }

    void showWindow();
    void initSize();

private slots:

    void on_okPbn_clicked();

private:
    Ui::AboutDialog *ui;
    QString version_;
    int width_;
};

#endif // ABOUT_DIALOG_H
