#include "modify_conf_dialog.h"
#include "ui_modify_conf_dialog.h"

ModifyConfDialog::ModifyConfDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyConfDialog)
{
    ui->setupUi(this);
}

ModifyConfDialog::~ModifyConfDialog()
{
    delete ui;
}
