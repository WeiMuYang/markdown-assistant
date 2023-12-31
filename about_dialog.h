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

    void showWindow();

private slots:

    void on_okPbn_clicked();

private:
    Ui::AboutDialog *ui;
    QString version_;
};

#endif // ABOUT_DIALOG_H
