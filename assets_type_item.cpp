#include "assets_type_item.h"
#include "ui_assets_type_item.h"

AssetsTypeItem::AssetsTypeItem(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AssetsTypeItem)
{
    ui->setupUi(this);
}

AssetsTypeItem::~AssetsTypeItem()
{
    delete ui;
}

void AssetsTypeItem::initWindowSize() {
    double widthIn4K = 600;
    double heightIn4K = 200;
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

void AssetsTypeItem::showWindow()
{
    initWindowSize();
    ui->assetTypeText->setText("");
    show();
}

void AssetsTypeItem::on_okPbn_clicked()
{
    if(ui->assetTypeText->text().isEmpty()) {
        sigAssetsTypeLog("Assets Type 不能为空! ");
    }else{
        emit sigAssetsType(ui->assetTypeText->text());
    }
    this->close();
}

