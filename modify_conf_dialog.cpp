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

void ModifyConfDialog::initWindow() {
    ui->assetsDirList->setColumnCount(2);
    ui->assetsDirList->setSelectionBehavior(QAbstractItemView::SelectRows);
    /*去掉每行的行号*/
    QHeaderView *headerView = ui->assetsDirList->verticalHeader();
    headerView->setHidden(true);
    ui->assetsDirList->horizontalHeader()->setStretchLastSection(true);
    // 消除表格控件的边框
    ui->assetsDirList->setFrameShape(QFrame::NoFrame);
    //设置表格不显示格子线
    ui->assetsDirList->setShowGrid(false); //设置不显示格子线


}

void ModifyConfDialog::updateAssetsDirList() {
    ui->assetsDirList->clearContents();
    QList<NamePath> AssetsPath = configdata_->getAssetsPathDir();
    for(int i = 0; i < AssetsPath.size(); ++i) {
        ui->assetsDirList->setRowCount(i + 1);
        NamePath dirInfo = AssetsPath.at(i);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(dirInfo.key);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(dirInfo.value);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        ui->assetsDirList->setItem(i, 0, pItem1);
        ui->assetsDirList->setItem(i, 1, pItem2);
    }
}

void ModifyConfDialog::showWindow() {


    show();
}

void ModifyConfDialog::on_addAssetsDir_clicked()
{

}

