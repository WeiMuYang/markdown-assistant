#ifndef MODIFY_CONF_DIALOG_H
#define MODIFY_CONF_DIALOG_H

#include <QDialog>

namespace Ui {
class ModifyConfDialog;
}

class ModifyConfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyConfDialog(QWidget *parent = nullptr);
    ~ModifyConfDialog();

private:
    Ui::ModifyConfDialog *ui;
};

#endif // MODIFY_CONF_DIALOG_H
