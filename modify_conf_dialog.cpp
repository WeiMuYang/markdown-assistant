#include "modify_conf_dialog.h"
#include "ui_modify_conf_dialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>

ModifyConfDialog::ModifyConfDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyConfDialog)
{
    ui->setupUi(this);
    screenRes_ = ScreenRes::High;
    initWindow();
}

ModifyConfDialog::~ModifyConfDialog()
{
    delete ui;
}

void ModifyConfDialog::initWindow() {

    {//============
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
        connect(ui->assetsDirList,&QTableWidget::itemChanged,this, &ModifyConfDialog::updateAssetsDirListEditSlot);
    }

    {//==============
        ui->repoDirList->setColumnCount(2);
        ui->repoDirList->setSelectionBehavior(QAbstractItemView::SelectRows);
        /*去掉每行的行号*/
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
        QFont font;
        if(screenRes_ == ScreenRes::High){
            font = ui->assetsTypeList->font();
            font.setPointSize(13);
            ui->assetsTypeList->setFont(font);
            multiple_ = 2;
        }else{
            font = ui->assetsTypeList->font();
            font.setPointSize(10);
            ui->assetsTypeList->setFont(font);
            multiple_ = 1;
        }
        connect(ui->assetsTypeList,&QListWidget::itemChanged,this, &ModifyConfDialog::updateAssetsTypeListEditSlot);
    }
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
    if(screenRes_ == ScreenRes::High) {
        this->setMinimumSize(1000, 870);
        this->resize(1000, 870);
    } else {
        // ===============
        this->setMinimumSize(800, 720);
        this->resize(800, 720);
    }
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
        emit sigModifyConfDlgLog("\"" + existName + "\"和\"" + fileInfo.fileName() + "\"是同一个目录!");
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
//            configdata_.writeConfJson();
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
    if(configdata_.addRepoPath(fileInfo.fileName(), fileInfo.filePath(), existName)) {
//        configdata_.writeConfJson();
        ////////////////////-------------------------------------------error
//        updateDataAndWidget();
//        ui->tarPathCombox->setCurrentText(newName);
    }else{
        emit sigModifyConfDlgLog("\"" + existName + "\"和\"" + fileInfo.fileName() + "\"是同一个仓库!");
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
//            configdata_.writeConfJson();
        }
    }
    updateRepoList();
}

void ModifyConfDialog::on_addAssetsTypePbn_clicked()
{
    QListWidgetItem *pItem = new QListWidgetItem("");
    pItem->setFlags(pItem->flags() | Qt::ItemIsEditable);
    ui->assetsTypeList->addItem(pItem);
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

