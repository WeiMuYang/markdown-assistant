#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QDialog>
#include <QKeyEvent>

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
protected:

    void keyPressEvent(QKeyEvent *event) override {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
            on_okPbn_clicked();
        }
    }

private:
    Ui::AboutDialog *ui;
    QString version_;
    int width_;
};

#endif // ABOUT_DIALOG_H
