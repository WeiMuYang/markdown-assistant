#include "modify_name_dialog.h"
#include "ui_modify_name_dialog.h"

ModifyNameDialog::ModifyNameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyNameDialog)
{
    ui->setupUi(this);
}

ModifyNameDialog::~ModifyNameDialog()
{
    delete ui;
}
