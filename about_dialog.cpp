#include "about_dialog.h"
#include "ui_about_dialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::showWindow()
{
    ui->versionLab->setText(version_);
    show();
}

void AboutDialog::on_okPbn_clicked()
{
    close();
}

