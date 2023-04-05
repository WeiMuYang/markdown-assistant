#include "get_assets_dialog.h"
#include "ui_get_assets_dialog.h"

GetAssetsDialog::GetAssetsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetAssetsDialog)
{
    ui->setupUi(this);
}

GetAssetsDialog::~GetAssetsDialog()
{
    delete ui;
}

QString GetAssetsDialog::getMarkdownCode()
{
    return ui->codeText->toPlainText();
}

void GetAssetsDialog::setMarkdownCode(const QString &str)
{
//    ui->codeText->setMarkdown(str);
     ui->codeText->setText(str);
}

QString GetAssetsDialog::getRename()
{
    return ui->renameText->text();
}

void GetAssetsDialog::setRename(const QString &str)
{
    ui->renameText->setText(str);
}

void GetAssetsDialog::reject()
{
    this->hide();
    this->close();
}

void GetAssetsDialog::closeEvent(QCloseEvent *)
{
    ui->codeText->clear();
    ui->renameText->clear();
}

void GetAssetsDialog::on_okPbn_clicked()
{
    emit sigSearchMarkdownCode(ui->codeText->toPlainText(), ui->renameText->text());
}

void GetAssetsDialog::on_noPbn_clicked()
{
    close();
}

