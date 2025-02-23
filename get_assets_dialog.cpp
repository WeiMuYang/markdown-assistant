#include "get_assets_dialog.h"
#include "ui_get_assets_dialog.h"
#include "debug_box.h"
#include <QStyle>

GetAssetsDialog::GetAssetsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetAssetsDialog)
{
    ui->setupUi(this);
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    ui->helpPbn->setIcon(icon);
}

GetAssetsDialog::~GetAssetsDialog()
{
    delete ui;
}

void GetAssetsDialog::initSize() {
    double widthIn4K = 750;
    double heightIn4K = 400;
    double zoom = 1;
    if(width_ < 3840) {
        zoom = 1.2;
    }
    // 750 400
    // 宽高比
    double WindowAspect = heightIn4K / widthIn4K;
    // 占屏比
    double Proportion = widthIn4K / 3840.0;
    // 宽 高
    int width = width_ * Proportion ;
    int height = width * WindowAspect;
    setMinimumSize(QSize(width, height) * zoom);
    this->resize(QSize(width, height) * zoom);
}

QString GetAssetsDialog::getMarkdownCode()
{
    return ui->codeText->toPlainText();
}

void GetAssetsDialog::setMarkdownCode(const QString &str)
{
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

void GetAssetsDialog::on_helpPbn_clicked()
{
    DebugBox box;
    box.helpBoxSlot("默认自动编号，可以通过逗号或者空格分割多个名字，%time是用时间命名！", width_);
    ui->renameText->setText("%time");
}

