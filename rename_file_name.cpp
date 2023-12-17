#include "rename_file_name.h"
#include "ui_rename_file_name.h"
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QClipboard>
#include <QAction>


RenameFileName::RenameFileName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameFileName)
{
    ui->setupUi(this);
    fileOperaton_ = new FileOperation(this);
    confFile_ = new confDialog;
    renameListMenu_ = new QMenu(this);
    initRenameFileList();
}

void RenameFileName::setRepoPath(const QString& path){
    repoPath_ = path;
}

void RenameFileName::setRenameDirPath(const QString& path){
    renameDirPath_ = path;
    ui->RenameDirPathEdit->setText(renameDirPath_);
}

void RenameFileName::setRenameConfPath(const QString& path){
    renameConfPath_ = path;
    ui->NameConfFilePathEdit->setText(renameConfPath_);
}

void RenameFileName::setRenameListPath(const QString& path){
    renameListPath_ = path;
    ui->NameListPathEdit->setText(renameListPath_);
}

RenameFileName::~RenameFileName()
{
    delete ui;
}

// 拷贝文件夹
bool RenameFileName::BackUp(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){
        /* 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }
    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;
        if(fileInfo.isDir()) {    /**< 当为目录时，递归的进行copy */
            if(!BackUp(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)){
                return false;
            }
        }
        else {
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }
            /// 进行文件copy
            if(!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))){
                return false;
            }
        }
    }
    return true;
}

//添加旧文件信息：oldDirPath和oldFilePath
bool RenameFileName::updateOldPath(const QString &fromDir, QStringList& oldDirPathList,  QStringList& oldFilePathList)
{
    QDir sourceDir(fromDir);
    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList) {
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;
        if(fileInfo.isDir()) {    /**< 当为目录时，递归的进行copy */
            QDir curDir(renameDirPath_);
            curDir.setCurrent(renameDirPath_);
            QString filePath = curDir.relativeFilePath(fileInfo.filePath());
            oldDirPathList.append(filePath);

        }
        else {
            QDir curDir(renameDirPath_);
            curDir.setCurrent(renameDirPath_);
            QString filePath = curDir.relativeFilePath(fileInfo.filePath());
            oldFilePathList.append(filePath);
        }

    }
    return true;
}


void RenameFileName::initRenameFileList() {
    ui->renameFileListWgt->setColumnCount(5);     //设置列数
    ui->renameFileListWgt->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 右击菜单
    ui->renameFileListWgt->setContextMenuPolicy(Qt::CustomContextMenu);
    //设置每行内容不可编辑
//    ui->renameFileListWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*去掉每行的行号*/
    QHeaderView *headerView = ui->renameFileListWgt->verticalHeader();
    headerView->setHidden(true);
    ui->renameFileListWgt->horizontalHeader()->setStretchLastSection(true);
    // 消除表格控件的边框
    ui->renameFileListWgt->setFrameShape(QFrame::NoFrame);
    //设置表格不显示格子线
    ui->renameFileListWgt->setShowGrid(false); //设置不显示格子线
    // 鼠标掠过事件
    ui->renameFileListWgt->setMouseTracking(true);
//    connect(ui->renameFileListWgt,&QTableWidget::itemEntered,this, &RenameFileName::updateReferListEnteredSlot);
    connect(ui->renameFileListWgt,&QTableWidget::clicked,this, &RenameFileName::updateReferListClickedSlot);
    connect(ui->renameFileListWgt,&QTableWidget::itemChanged,this, &RenameFileName::updateReferListEditSlot);
    // rightClickMenu
    connect(ui->renameFileListWgt,&QTableWidget::customContextMenuRequested,this,&RenameFileName::showMenuSlot);
// =======================================================================
    ui->referFileListWgt->setColumnCount(4);     //设置列数
    ui->referFileListWgt->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置每行内容不可编辑
    ui->referFileListWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*去掉每行的行号*/
    ui->referFileListWgt->verticalHeader()->setHidden(true);
    ui->referFileListWgt->horizontalHeader()->setStretchLastSection(true);
    // 消除表格控件的边框
    ui->referFileListWgt->setFrameShape(QFrame::NoFrame);
    //设置表格不显示格子线
    ui->referFileListWgt->setShowGrid(false); //设置不显示格子线

    connect(ui->referFileListWgt,&QTableWidget::itemDoubleClicked,this,&RenameFileName::OpenReferFileSlot);
    initListMenu();
}

void RenameFileName::OpenReferFileSlot(QTableWidgetItem *item) {
    int row = ui->renameFileListWgt->currentItem()->row();
    QVector<ReFile> reFiles;
    int len = replaceNameDirInfoList_.size();
    if(row < len) {
        reFiles = replaceNameDirInfoList_.at(row).reDirList;
    }else {
        reFiles = replaceNameFileInfoList_.at(row - len).reFileList;
    }
    QString curRePathAbs = repoPath_ + "/" + reFiles.at(item->row()).reFilePath;
    emit sigRenameFileReferFile(curRePathAbs);
}

void RenameFileName::initListMenu() {
    QAction *actCopyAddList = new QAction("拷贝列表",renameListMenu_);
    renameListMenu_->addAction(actCopyAddList);
    connect(actCopyAddList, &QAction::triggered, this, &RenameFileName::copyOldFileListSlot);
}

void RenameFileName::showMenuSlot() {
    renameListMenu_->move(cursor().pos());
    renameListMenu_->show();
}

void RenameFileName::copyOldFileListSlot(){
    QClipboard* clip = QApplication::clipboard();
    QString clipText;
    for(int i = 0; i < ui->renameFileListWgt->rowCount(); ++i){
        clipText += ui->renameFileListWgt->item(i,2)->text() + "\n";
    }
    clip->setText(clipText);
}

void RenameFileName::updateRenameFileList(){
    renameListClear();
    disconnect(ui->renameFileListWgt,&QTableWidget::itemChanged,this, &RenameFileName::updateReferListEditSlot);
    int n = 1;
    for(int j = 0; j < replaceNameDirInfoList_.size(); ++j) {
        ui->renameFileListWgt->setRowCount(j + 1);
        DirRenameInfo dirInfo = replaceNameDirInfoList_.at(j);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::number(n)); pItem0->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(dirInfo.newDirPath.split("/").last());
        QTableWidgetItem *pItem2 = new QTableWidgetItem(dirInfo.oldDirPath.split("/").last());
        QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::number(dirInfo.reDirList.size()));
        pItem3->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem4 = new QTableWidgetItem(dirInfo.oldDirPath);
        if(dirInfo.reDirList.size() > 0){
            pItem0->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem0->setForeground(QBrush(Qt::red));         // 设置字体颜色
        }
        if(dirInfo.newDirPath != dirInfo.oldDirPath) {
            pItem1->setForeground(QBrush(Qt::yellow));         // 设置字体颜色
        }

        pItem0->setFlags(pItem0->flags() & ~Qt::ItemIsEditable);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        pItem3->setFlags(pItem3->flags() & ~Qt::ItemIsEditable);
        pItem4->setFlags(pItem4->flags() & ~Qt::ItemIsEditable);

        ui->renameFileListWgt->setItem(j, 0, pItem0);
        ui->renameFileListWgt->setItem(j, 1, pItem1);
        ui->renameFileListWgt->setItem(j, 2, pItem2);
        ui->renameFileListWgt->setItem(j, 3, pItem3);
        ui->renameFileListWgt->setItem(j, 4, pItem4);
        n++;
    }
    int num = replaceNameDirInfoList_.size();
    for(int j = 0; j < replaceNameFileInfoList_.size(); ++j) {
        ui->renameFileListWgt->setRowCount(num + j + 1);
        FileRenameInfo fileInfo = replaceNameFileInfoList_.at(j);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::number(n)); pItem0->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(fileInfo.newFilePath.split("/").last());
        pItem1->setFlags(pItem1->flags() | Qt::ItemIsEditable);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(fileInfo.oldFilePath.split("/").last());
        QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::number(fileInfo.reFileList.size()));
        pItem3->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem4 = new QTableWidgetItem(fileInfo.oldFilePath);
        if(fileInfo.reFileList.size() > 0){
            pItem0->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem0->setForeground(QBrush(Qt::red));         // 设置字体颜色
        }
        if(fileInfo.newFilePath != fileInfo.oldFilePath) {
            pItem1->setForeground(QBrush(Qt::yellow));         // 设置字体颜色
        }
        pItem0->setFlags(pItem0->flags() & ~Qt::ItemIsEditable);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        pItem3->setFlags(pItem3->flags() & ~Qt::ItemIsEditable);
        pItem4->setFlags(pItem4->flags() & ~Qt::ItemIsEditable);

        ui->renameFileListWgt->setItem(num + j, 0, pItem0);
        ui->renameFileListWgt->setItem(num + j, 1, pItem1);
        ui->renameFileListWgt->setItem(num + j, 2, pItem2);
        ui->renameFileListWgt->setItem(num + j, 3, pItem3);
        ui->renameFileListWgt->setItem(num + j, 4, pItem4);
        n++;
    }
    connect(ui->renameFileListWgt,&QTableWidget::itemChanged,this, &RenameFileName::updateReferListEditSlot);
    // 触发双击表头分割线信号，实现自动调整行宽
    emit ui->renameFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(0);
    emit ui->renameFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(1);
    emit ui->renameFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(2);
    emit ui->renameFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(3);
}

void RenameFileName::updateReferList(const QVector<ReFile>& reFileList) {
    ui->referFileListWgt->clearContents();
    if(reFileList.isEmpty()) {
        return;
    }
    int n = 1;
    for(int j = 0; j < reFileList.size(); ++j) {
        ui->referFileListWgt->setRowCount(j + 1);
        ReFile reFile = reFileList.at(j);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::number(n)); pItem0->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(reFile.reFilePath.split("/").last());
        QTableWidgetItem *pItem2 = new QTableWidgetItem(QString::number(reFile.reCount));     pItem2->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem3 = new QTableWidgetItem(reFile.reFilePath);
        ui->referFileListWgt->setItem(j, 0, pItem0);
        ui->referFileListWgt->setItem(j, 1, pItem1);
        ui->referFileListWgt->setItem(j, 2, pItem2);
        ui->referFileListWgt->setItem(j, 3, pItem3);
        n++;
    }
    // 触发双击表头分割线信号，实现自动调整行宽
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(0);
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(1);
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(2);
}

void RenameFileName::updateReferListByRenameFileListRow(int row) {
    int len = replaceNameDirInfoList_.size();
    if(row < len) {
        QVector<ReFile> reFileList = replaceNameDirInfoList_.at(row).reDirList;
        updateReferList(reFileList);
    }else {
        QVector<ReFile> reFileList = replaceNameFileInfoList_.at(row - len).reFileList;
        updateReferList(reFileList);
    }
}

void RenameFileName::updateReferListClickedSlot(QModelIndex index)
{
    int row = index.row();
    updateReferListByRenameFileListRow(row);
}

void RenameFileName::updateReferListEditSlot(QTableWidgetItem *item) {
    int row = item->row();
    QString name = item->text();
    int len = replaceNameDirInfoList_.size();
    if(row < len) {
        QString oldPath = replaceNameDirInfoList_.at(row).oldDirPath;
        oldPath.replace(oldPath.split("/").last(), name);
        replaceNameDirInfoList_[row].newDirPath = oldPath;
        if(oldPath != replaceNameDirInfoList_.at(row).oldDirPath) {
            item->setForeground(QBrush(Qt::yellow));
        }else{
            item->setForeground(QBrush(Qt::white));
        }
    }else {
        QString oldPath = replaceNameFileInfoList_.at(row - len).oldFilePath;
        oldPath.replace(oldPath.split("/").last(), name);
        replaceNameFileInfoList_[row - len].newFilePath = oldPath;
        if(oldPath != replaceNameFileInfoList_.at(row - len).oldFilePath) {
            item->setForeground(QBrush(Qt::yellow));
        }else{
            item->setForeground(QBrush(Qt::white));
        }
    }
}

void RenameFileName::renameDirPathByCharMap(const QStringList &oldDirPathList)
{
    for(int i = 0; i < oldDirPathList.size(); ++i) {
        QString path = oldDirPathList.at(i);
        QString name = path.split("/").last();
        QString newPathParent = path.split(name).first();
        QMap<QString, QString>::iterator iter = replaceCharMap_.begin();
        while (iter != replaceCharMap_.end())
        {
            name.replace(iter.key(), iter.value());
            iter++;
        }
        DirRenameInfo dirRenameInfo;
        dirRenameInfo.oldDirPath = oldDirPathList.at(i);
        dirRenameInfo.newDirPath = newPathParent + name;
        replaceNameDirInfoList_.append(dirRenameInfo);
    }
}

void RenameFileName::renameFilePathByCharMap(const QStringList &oldFilePathList) {
    for(int i = 0; i < oldFilePathList.size(); ++i) {
        QString path = oldFilePathList.at(i);
        QString name = path.split("/").last();
        QString newPathParent = path.split(name).first();
        QMap<QString, QString>::iterator iter = replaceCharMap_.begin();
        while (iter != replaceCharMap_.end())
        {
            if(iter.key() == ".") {
                int lastDotPos = name.lastIndexOf(".");
                name = name.left(lastDotPos).replace(".", iter.value()) + name.mid(lastDotPos);
                iter++;
            }else{
                name.replace(iter.key(), iter.value());
                iter++;
            }
        }
        FileRenameInfo fileRenameInfo;
        fileRenameInfo.oldFilePath = oldFilePathList.at(i);
        fileRenameInfo.newFilePath = newPathParent + name;
        replaceNameFileInfoList_.append(fileRenameInfo);
    }
}

void RenameFileName::createCompareDirAndFileLog(const QString& path, const QStringList& DirPathList, const QStringList& FilePathList){
    QFile file(path);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream stream(&file);
        for(int i = 0; i < DirPathList.size(); ++i){
            QDir curFileDir(renameDirPath_+ "/" + DirPathList.at(i));
            QString str = curFileDir.dirName();
            int desiredLength = 40;
            // 使用UTF-8编码
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            int byteLength = codec->fromUnicode(str).length();
            int numChineseChars = (byteLength - str.length()) / 2;
            QString paddedStr = str.leftJustified(desiredLength - numChineseChars);
            QString txt = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')) + QString(" |") + paddedStr + QString("|") + DirPathList.at(i);
            stream << txt << "\n";
        }
        for(int i = 0; i < FilePathList.size(); ++i){
            QDir curFileDir(renameDirPath_+ "/" + FilePathList.at(i));
            QString str = curFileDir.dirName();
            int desiredLength = 40;
            // 使用UTF-8编码
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            int byteLength = codec->fromUnicode(str).length();
            int numChineseChars = (byteLength - str.length()) / 2;
            QString paddedStr = str.leftJustified(desiredLength - numChineseChars);
            QString txt = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')) + QString(" |") + paddedStr + QString("|") + FilePathList.at(i);
            stream << txt << "\n";
        }
        file.close();
    }
}

// 只是将特殊字符进行替换
void RenameFileName::renameByConfFile() {
    replaceNameFileInfoList_.clear();
    replaceNameDirInfoList_.clear();
    if(!confFile_->readCharConfFile(renameConfPath_, replaceCharMap_)) {
        emit sigRenameFileNameLog(QString("Rename Configure file Open Error!"));
        return;
    }
    // 1. 备份文件
    QStringList oldDirPathList;
    QStringList oldFilePathList;
    QDir dir(repoPath_);
    dir.cdUp();
    if(ui->backupCheckBox->isChecked()){
        backUpPath_ = dir.absolutePath() + "/" + repoPath_.split("/").last() +".backup/" + renameDirPath_.split("/").last();
        dir.mkdir(dir.absolutePath() + "/" + repoPath_.split("/").last() +".backup");
        if(!BackUp(renameDirPath_, backUpPath_, true)){
            emit sigRenameFileNameLog(QString("Data backup Failed"));
            return;
        }
    }
    updateOldPath(renameDirPath_, oldDirPathList, oldFilePathList);
    // 2. 根据字符替换修改 Dir 和 File,更新新旧文件名
    renameDirPathByCharMap(oldDirPathList);
    renameFilePathByCharMap(oldFilePathList);

    // 3. 更新引用文件
    fileOperaton_->updateReplaceNameRefereceList(repoPath_ ,renameDirPath_, replaceNameDirInfoList_, replaceNameFileInfoList_);


    emit sigRenameFileNameLog(QString("更新文件命名数据完成!"));
}

void RenameFileName::renameListClear() {
    // 只删除内容，不删除表头
    ui->renameFileListWgt->clearContents();
    ui->referFileListWgt->clearContents();
}

void RenameFileName::renameByListFile() {
    fileOperaton_->updateReplaceNameByList(renameListPath_, replaceNameDirInfoList_, replaceNameFileInfoList_);
}

void RenameFileName::on_refreshPbn_clicked()
{
    if(ui->charRadioBtn->isChecked()){
        renameByConfFile();
    }
    if(ui->listRadioBtn->isChecked()){
        renameByListFile();
    }
    updateRenameFileList();
}

void RenameFileName::on_ChooseDirPbn_clicked()
{
    renameDirPath_ = QFileDialog::getExistingDirectory(this,"选择重命名目录",repoPath_,QFileDialog::ShowDirsOnly);
    ui->RenameDirPathEdit->setText(renameDirPath_);
}

void RenameFileName::on_ChooseConfPbn_clicked()
{
    renameConfPath_ = QFileDialog::getExistingDirectory(this,"选择配置文件",repoPath_,QFileDialog::ShowDirsOnly);
    ui->NameConfFilePathEdit->setText(renameConfPath_);
}

void RenameFileName::on_OpenFilePathPbn_clicked()
{
    emit sigRenameFileOpenPath(renameDirPath_);
}

void RenameFileName::on_OpenConfFilePbn_clicked()
{
    emit sigRenameFileConfFile(renameConfPath_);
}

QString RenameFileName::replaceNewFile(QDir newFileDir, QDir curFileDir, QString line, QString oldFileAbsPath)
{
    QString oldFileName = oldFileAbsPath.split("/").last();
    QVector<QPoint> refPosArr;
    curFileDir.cdUp();
    // 上一级目录
    QString desPath = curFileDir.relativeFilePath(newFileDir.absolutePath());
    int pos = 0;
    while ((pos = line.indexOf(']', pos)) != -1 && line.length() > pos + 1 && line.mid(pos + 1 ,1) == "(" ) {
        int endPos = line.indexOf(')', pos);
        if (endPos != -1) {
            QString subStr = line.mid(pos + 2, endPos - pos - 2);
            if(subStr.contains(oldFileName)) {
                QPoint refPos;
                refPos.setY(endPos);
                qDebug() << "y: " << endPos ;
                int posLeftMid = pos + 2;
                while (posLeftMid >= 0 && line.at(posLeftMid) != "[") {
                    posLeftMid--;
                }
                if(posLeftMid != -1) {
                    refPos.setX(posLeftMid + 1);
                    refPosArr.append(refPos);
                    qDebug() << "x: " << posLeftMid + 1 ;
                }
            }
            pos = endPos + 1;
        } else {
            break;
        }
    }
    if(refPosArr.isEmpty()){
        return line;
    }
    QString lineSplicing = line.mid(0, refPosArr[0].x() - 1);
    QString refFile = "[" + newFileDir.dirName() + "]" +"(" + desPath +")";
    //    int m = refFile.length();
    for(int i = 0 ; i < refPosArr.size()-1; ++i){
        qDebug() << lineSplicing ;
        qDebug() <<"------------------------" ;
        lineSplicing += refFile;
        qDebug() << lineSplicing ;
        qDebug() <<"++++++++++++++++++++++++" ;
        lineSplicing += line.mid(refPosArr.at(i).y() + 1, refPosArr.at(i+1).x() - refPosArr.at(i).y() - 2);
        qDebug() << lineSplicing ;
        qDebug() <<"=======================" ;
    }
    lineSplicing += refFile;
    lineSplicing += line.mid(refPosArr.last().y() + 1,line.size() - refPosArr.last().x());
    qDebug() << lineSplicing ;
    qDebug()<<".........................." ;
    qDebug() << line ;
    return lineSplicing;
}

bool RenameFileName::modifyRefMarkdown(const QString& reFilePathAbs, const QString& oldFileAbsPath,
                                       const QString& newFileAbsPath){
    QString strAll;
    QStringList strList;
    QDir dir;

    QDir newFileDir(newFileAbsPath);
    QDir curFileDir(reFilePathAbs);

    // 1. 自己引用自己
    if(reFilePathAbs == newFileAbsPath) {
        qDebug() << reFilePathAbs << ": This is itself!";
        return true;
    }
    // 2. 读取文件中的内容
    QFile readFile(reFilePathAbs);		//PATH是自定义读取文件的地址
    if(!readFile.open((QIODevice::ReadOnly|QIODevice::Text)))
    {
        qDebug()<< "Can't Read the RefMarkdown file!";
        return false;
    }
    QTextStream stream(&readFile);
    strAll = stream.readAll();
    readFile.close();
    // 3. 重新打开，逐句写入
    QFile writeFile(reFilePathAbs);	//PATH是自定义写文件的地址
    if(!writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug()<< "Can't write the RefMarkdown file!";
        return false;
    }
    QTextStream wStream(&writeFile);
    strList = strAll.split("\n");           //以换行符为基准分割文本
    for(int i = 0;i < strList.count();i++)  //遍历每一行
    {
        if(strList.at(i).contains(oldFileAbsPath.split("/").last()))
        {
            qDebug() << strList.at(i);
            wStream << replaceNewFile(newFileDir, curFileDir, strList.at(i), oldFileAbsPath) << '\n';
        }else{ // 如果没有找到要替换的内容，照常写入
            if(i == strList.count() - 1) {
                wStream << strList.at(i);
            }else{
                wStream << strList.at(i) << '\n';
            }
        }
    }
    writeFile.close();
    return true;
}

void RenameFileName::on_ReplaceByListPbn_clicked()
{
    QDir dir;
    // 更新修改的目录
    for(int i = 0; i < replaceNameDirInfoList_.size(); ++i) {
        if(replaceNameDirInfoList_.at(i).oldDirPath != replaceNameDirInfoList_.at(i).newDirPath) {
            QString oldPathAbs = renameDirPath_ + "/" + replaceNameDirInfoList_.at(i).oldDirPath;
            QString newPathAbs = renameDirPath_ + "/" + replaceNameDirInfoList_.at(i).newDirPath;
            if(!QFile::rename(oldPathAbs, newPathAbs)) {
                emit sigRenameFileNameLog(QString("Move ") + replaceNameDirInfoList_.at(i).oldDirPath + " Failed ！");
            } else {
                QVector<ReFile> reFileDir = replaceNameDirInfoList_.at(i).reDirList;
                for(int j = 0; j < reFileDir.size(); ++j) {
                    QString reFilePathAbs = repoPath_ + "/" + reFileDir.at(j).reFilePath;
                    // 2. 被引用的文件名字修改了
                    if(!dir.exists(reFilePathAbs)) {
                        int k;
                        for(k = 0; k < replaceNameFileInfoList_.size(); ++k) {
                            if(renameDirPath_ + "/" + replaceNameFileInfoList_.at(k).oldFilePath == reFilePathAbs) {
                                reFilePathAbs.replace(replaceNameFileInfoList_.at(k).oldFilePath,replaceNameFileInfoList_.at(k).newFilePath);
                                 modifyRefMarkdown(reFilePathAbs, oldPathAbs, newPathAbs);
                                break;
                            }
                        }
                        if(k >= replaceNameFileInfoList_.size()){
                            emit sigRenameFileNameLog(QString("引用文件:[" + reFilePathAbs + "] 不存在!"));
                        }
                    } else {
                        modifyRefMarkdown(reFilePathAbs, oldPathAbs, newPathAbs);
                    }
                }
            }
        }
    }

    // 更新修改的文件
    for(int i = 0; i < replaceNameFileInfoList_.size(); ++i) {
        if(replaceNameFileInfoList_.at(i).oldFilePath != replaceNameFileInfoList_.at(i).newFilePath) {
            QString oldPathAbs = renameDirPath_ + "/" + replaceNameFileInfoList_.at(i).oldFilePath;
            QString newPathAbs = renameDirPath_ + "/" + replaceNameFileInfoList_.at(i).newFilePath;
            if(!QFile::rename(oldPathAbs, newPathAbs)) {
                emit sigRenameFileNameLog(QString("Move ") + replaceNameFileInfoList_.at(i).oldFilePath + " Failed ！");
            } else {
                QVector<ReFile> reFileDir = replaceNameFileInfoList_.at(i).reFileList;
                for(int j = 0; j < reFileDir.size(); ++j) {
                    QString reFilePathAbs = repoPath_ + "/" + reFileDir.at(j).reFilePath;
                    // 2. 被引用的文件名字修改了
                    if(!dir.exists(reFilePathAbs)) {
                        int k;
                        for(k = 0; k < replaceNameFileInfoList_.size(); ++k) {
                            if(renameDirPath_ + "/" + replaceNameFileInfoList_.at(k).oldFilePath == reFilePathAbs) {
                                reFilePathAbs.replace(replaceNameFileInfoList_.at(k).oldFilePath,replaceNameFileInfoList_.at(k).newFilePath);
                                 modifyRefMarkdown(reFilePathAbs, oldPathAbs, newPathAbs);
                                break;
                            }
                        }
                        if(k >= replaceNameFileInfoList_.size()){
                            emit sigRenameFileNameLog(QString("引用文件:[" + reFilePathAbs + "] 不存在!"));
                        }
                    } else {
                        modifyRefMarkdown(reFilePathAbs, oldPathAbs, newPathAbs);
                    }
                }
            }
        }
    }
}



void RenameFileName::on_ChooseListPbn_clicked()
{
    renameListPath_ = QFileDialog::getExistingDirectory(this,"选择命名列表",repoPath_,QFileDialog::ShowDirsOnly);
    ui->NameListPathEdit->setText(renameListPath_);
}

void RenameFileName::on_OpenListFilePbn_clicked()
{
    emit sigRenameFileConfFile(renameListPath_);
}
