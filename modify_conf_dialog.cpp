#include "modify_conf_dialog.h"
#include "ui_modify_conf_dialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>
#include <QScreen>
#include "debug_box.h"

ModifyConfDialog::ModifyConfDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyConfDialog)
{
    ui->setupUi(this);
    initWindow();
    assetsTypeItem_ = new AssetsTypeItem(this);

    connect(assetsTypeItem_,&AssetsTypeItem::sigAssetsType,this, &ModifyConfDialog::addAssetsTypeSlot);
    connect(assetsTypeItem_,&AssetsTypeItem::sigAssetsTypeLog,[this](QString msg) {
        emit sigModifyConfDlgLog(msg);
    });
}

ModifyConfDialog::~ModifyConfDialog()
{
    delete ui;
}

void ModifyConfDialog::initWindow() {

    {//============
        ui->assetsDirList->setColumnCount(2);
        ui->assetsDirList->setSelectionBehavior(QAbstractItemView::SelectRows);
        /*去掉每行的行号 */
        QHeaderView *headerView = ui->assetsDirList->verticalHeader();
        headerView->setHidden(true);
        ui->assetsDirList->horizontalHeader()->setStretchLastSection(true);
        // 消除表格控件的边框
        ui->assetsDirList->setFrameShape(QFrame::NoFrame);
        //设置表格不显示格子线
        ui->assetsDirList->setShowGrid(false); //设置不显示格子线
        connect(ui->assetsDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateAssetsDirListEditSlot);
    }

    {//==============
        ui->repoDirList->setColumnCount(2);
        ui->repoDirList->setSelectionBehavior(QAbstractItemView::SelectRows);
        /* 去掉每行的行号 */
        QHeaderView *headerView = ui->repoDirList->verticalHeader();
        headerView->setHidden(true);
        ui->repoDirList->horizontalHeader()->setStretchLastSection(true);
        // 消除表格控件的边框
        ui->repoDirList->setFrameShape(QFrame::NoFrame);
        //设置表格不显示格子线
        ui->repoDirList->setShowGrid(false); //设置不显示格子线
        connect(ui->repoDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateRepoListEditSlot);
    }
    {
        connect(ui->assetsTypeList,&QListWidget::itemChanged,this, &ModifyConfDialog::updateAssetsTypeListEditSlot);
    }
    QIcon icon = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    ui->helpAssetsTypePbn->setIcon(icon);
}

void ModifyConfDialog::updateAssetsDirList() {
    disconnect(ui->assetsDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateAssetsDirListEditSlot);
    ui->assetsDirList->clearContents();
    QList<NamePath> AssetsPath = configdata_.getAssetsPathDir();
    for(int i = 0; i < AssetsPath.size(); ++i) {
        ui->assetsDirList->setRowCount(i + 1);
        NamePath dirInfo = AssetsPath.at(i);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(dirInfo.key);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(dirInfo.value);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        ui->assetsDirList->setItem(i, 0, pItem1);
        ui->assetsDirList->setItem(i, 1, pItem2);
    }
    connect(ui->assetsDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateAssetsDirListEditSlot);
}

void ModifyConfDialog::updateRepoList()
{
    disconnect(ui->repoDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateRepoListEditSlot);
    ui->repoDirList->clearContents();
    QList<NamePath> repoPath = configdata_.getRepoPaths();
    for(int i = 0; i < repoPath.size(); ++i) {
        ui->repoDirList->setRowCount(i + 1);
        NamePath dirInfo = repoPath.at(i);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(dirInfo.key);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(dirInfo.value);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        ui->repoDirList->setItem(i, 0, pItem1);
        ui->repoDirList->setItem(i, 1, pItem2);
    }
    connect(ui->repoDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateRepoListEditSlot);
}

void ModifyConfDialog::updateAssetsType()
{
    disconnect(ui->assetsTypeList,&QListWidget::itemChanged,this, &ModifyConfDialog::updateAssetsTypeListEditSlot);
    ui->assetsTypeList->clear();
    QStringList assetsType = configdata_.getAssetsTypes();
    for(int i = 0; i < assetsType.size(); ++i) {
        QListWidgetItem* newItem = new QListWidgetItem(assetsType.at(i));
        newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
        ui->assetsTypeList->addItem(newItem);
    }
    connect(ui->assetsTypeList,&QListWidget::itemChanged,this, &ModifyConfDialog::updateAssetsTypeListEditSlot);
}

void ModifyConfDialog::initWindowsSize() {
    double widthIn4K = 1250;
    double heightIn4K = 1000;
    double zoom = 1;
    if(width_ < 3840) {
        zoom = 1.1;
    }
    // 1000 870
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

void ModifyConfDialog::showWindow() {
    initWindowsSize();
    updateAssetsDirList();
    updateRepoList();
    updateAssetsType();
    updateSoftWarePath();
    updateMeetingPath();
    qDebug() << QString::number(this->width()) << " " << QString::number(this->height());
    show();
}

void ModifyConfDialog::updateSoftWarePath() {
    ui->markdownSoftEdit->setText(configdata_.getMarkdownSoftPath());
    ui->dataDirSoftEdit->setText(configdata_.getDataDirSoftPath());
}

void ModifyConfDialog::updateMeetingPath() {
    ui->meetingFilePathEdit->setText(configdata_.getMeetFilePath());
}

void ModifyConfDialog::on_addAssetsDir_clicked()
{
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString path = QFileDialog::getExistingDirectory(this,"选择Assets目录", desktop,QFileDialog::ShowDirsOnly);
    if(path.isEmpty()) {
        emit sigModifyConfDlgLog("添加资源目录\"" + path + "\"为空，无法添加!");
        return ;
    }
    QFileInfo fileInfo(path);
    QString existName;
    if(!configdata_.addAssetsPath(fileInfo.fileName(), fileInfo.filePath(), existName)) {
        emit sigModifyConfDlgLog("\"" + existName + "\" and \"" + fileInfo.fileName() + "\"是同一个目录! ");
    }
    updateAssetsDirList();
}

void ModifyConfDialog::on_delAssetsDir_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->assetsDirList->selectedItems();
    for (int i = 0; i < selectedItems.size(); ++i) {
        QTableWidgetItem* item = selectedItems.at(i);
        if (item->column() == 1) {
            QString path = item->text();
            configdata_.delAssetsPath(path);
        }
    }
    updateAssetsDirList();
}

void ModifyConfDialog::on_addRepoDir_clicked()
{
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString path = QFileDialog::getExistingDirectory(this,"选择仓库目录", desktop,QFileDialog::ShowDirsOnly);
    if(path.isEmpty()) {
        emit sigModifyConfDlgLog("添加资源目录\"" + path + "\"为空，无法添加!");
        return ;
    }
    QFileInfo fileInfo(path);
    QString existName;
    if(!configdata_.addRepoPath(fileInfo.fileName(), fileInfo.filePath(), existName)) {
        emit sigModifyConfDlgLog("\"" + existName + "\" and \"" + fileInfo.fileName() + "\"是同一个仓库!");
        return ;
    }
    updateRepoList();
}

void ModifyConfDialog::on_delRepoDir_clicked()
{
    QList<QTableWidgetItem*> selectedItems = ui->repoDirList->selectedItems();
    for (int i = 0; i < selectedItems.size(); ++i) {
        QTableWidgetItem* item = selectedItems.at(i);
        if (item->column() == 1) {
            QString path = item->text();
            configdata_.delRepoPath(path);
        }
    }
    updateRepoList();
}

void ModifyConfDialog::addAssetsTypeSlot(QString type) {
    QListWidgetItem *pItem = new QListWidgetItem(type);
    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
    ui->assetsTypeList->addItem(pItem);
    configdata_.addAssetsTypes(type);
}

void ModifyConfDialog::on_addAssetsTypePbn_clicked()
{
    assetsTypeItem_->setWidth(width_);
    assetsTypeItem_->showWindow();
}

void ModifyConfDialog::on_delAssetsTypePbn_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->assetsTypeList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        int row = ui->assetsTypeList->row(item);
        configdata_.delAssetsTypes(item->text());
        ui->assetsTypeList->takeItem(row);
        delete item; // 如果需要手动释放内存
    }
}

void ModifyConfDialog::updateAssetsTypeListEditSlot(QListWidgetItem *item) {
    if(item->text() == "") {
        emit sigModifyConfDlgLog( "AssetsType不能为空，无法添加!");
    }
    configdata_.modifyAssetsTypes(ui->assetsTypeList->row(item), item->text());
}

void ModifyConfDialog::updateRepoListEditSlot(QTableWidgetItem *item) {
    if(item->column() == 0) {
        int row = item->row();
        QString name = item->text();
        configdata_.modifyRepoName(row, name);
    }
}

void ModifyConfDialog::updateAssetsDirListEditSlot(QTableWidgetItem *item) {
    if(item->column() == 0) {
        int row = item->row();
        QString name = item->text();
        configdata_.modifyAssetsName(row, name);
    }
}

void ModifyConfDialog::on_markdownSoftPbn_clicked()
{
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString path = QFileDialog::getOpenFileName(this,"选择Markdown软件",desktop ,tr("EXE files(*.exe);;Shell files(*.sh);;All files(*.*)"));
    if(path.isEmpty()) {
        emit sigModifyConfDlgLog("Markdown软件路径\"" + path + "\"为空，无法添加!");
        return ;
    }
    ui->markdownSoftEdit->setText(path);
    configdata_.setMarkdownSoftWarePath(path);
}

void ModifyConfDialog::on_dataDirSoftPbn_clicked()
{
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString path = QFileDialog::getOpenFileName(this,"选择Data目录软件",desktop ,tr("EXE files(*.exe);;Shell files(*.sh);;All files(*.*)"));
    if(path.isEmpty()) {
        emit sigModifyConfDlgLog("Data软件路径\"" + path + "\"为空，无法添加!");
        return ;
    }
    ui->dataDirSoftEdit->setText(path);
    configdata_.setDataDirSoftWarePath(path);
}

void ModifyConfDialog::on_yesPbn_clicked()
{
    this->close();
    emit sigModifyConfigData(configdata_);
}

void ModifyConfDialog::on_cancelPbn_clicked()
{
    this->close();
}


void ModifyConfDialog::on_applyPbn_clicked()
{
    emit sigModifyConfigData(configdata_);
}

void ModifyConfDialog::on_helpAssetsTypePbn_clicked()
{
    DebugBox box;
    QString msg;
    msg += "过滤器可以是简单的文件名，也可以是带有通配符的模式: \n";
    msg += "1. \"*\"：匹配任意长度。  ";
    msg += "2. \"?\"：匹配单个字符。   ";
    msg += "3. \"[ ]\"：匹配括号内的任意单个字符。  ";
    msg += "4. \"[! ]\"：匹配不在括号内的任意单个字符。  ";
    msg += "5. \"{ }\"：匹配花括号内的任意字符序列。 \n";

    box.helpBoxSlot(msg, width_);
}
