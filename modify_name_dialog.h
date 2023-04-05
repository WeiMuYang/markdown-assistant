#ifndef MODIFY_NAME_DIALOG_H
#define MODIFY_NAME_DIALOG_H

#include <QDialog>

namespace Ui {
class ModifyNameDialog;
}

class ModifyNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyNameDialog(QWidget *parent = nullptr);
    ~ModifyNameDialog();

private:
    Ui::ModifyNameDialog *ui;
};

#endif // MODIFY_NAME_DIALOG_H
