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

void AboutDialog::initSize() {
    double widthIn4K = 600;
    double heightIn4K = 320;
    double zoom = 1;
    if(width_ < 3840) {
        zoom = 1.2;
    }
    // 600  320
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

void AboutDialog::showWindow()
{
    ui->versionLab->setText(version_);
    initSize();
    show();
}

void AboutDialog::on_okPbn_clicked()
{
    close();
}

