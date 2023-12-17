#include "rename_file_name.h"
#include "ui_rename_file_name.h"
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>

RenameFileName::RenameFileName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameFileName)
{
    ui->setupUi(this);
    fileOperaton_ = new FileOperation(this);
    confFile_ = new confDialog;
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

bool RenameFileName::readNewFileList() {
    QFile file(backUpPath_ + "/" + NEW_FILE_NAME);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit sigRenameFileNameLog(QString(" Read [" + NEW_FILE_NAME + "] Failed!"));
        return false;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    int i = 0, j = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(line.split("|").size() < 3) {
            emit sigRenameFileNameLog(QString(" Read [" + NEW_FILE_NAME + "] split line Error!"));
            return false;
        }
        QString name = line.split("|").at(1);
        QString path = line.split("|").last();
        int pos = path.lastIndexOf("/") + 1;
        QString strName = path.mid(0, pos) + name;
        strName = strName.trimmed();
        if(i < replaceNameDirInfoList_.size()){
            replaceNameDirInfoList_[i].newDirPath = strName;
            i++;
        }else{
            replaceNameFileInfoList_[j].newFilePath = strName;
            j++;
        }
    }
    file.close();
    return true;
}

void RenameFileName::initRenameFileList() {
    ui->renameFileListWgt->setColumnCount(5);     //设置列数
    ui->renameFileListWgt->setSelectionBehavior(QAbstractItemView::SelectRows);
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
}

void RenameFileName::updateRenameFileList(){
    renameListClear();
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
            pItem1->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem1->setForeground(QBrush(Qt::red));         // 设置字体颜色
            pItem2->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem2->setForeground(QBrush(Qt::red));         // 设置字体颜色
            pItem3->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem3->setForeground(QBrush(Qt::red));         // 设置字体颜色
            pItem4->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem4->setForeground(QBrush(Qt::red));         // 设置字体颜色
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
            pItem1->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem1->setForeground(QBrush(Qt::red));         // 设置字体颜色
            pItem2->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem2->setForeground(QBrush(Qt::red));         // 设置字体颜色
            pItem3->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem3->setForeground(QBrush(Qt::red));         // 设置字体颜色
            pItem4->setBackground(QBrush(Qt::gray));        // 设置背景色
            pItem4->setForeground(QBrush(Qt::red));         // 设置字体颜色
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

//void RenameFileName::updateReferListEnteredSlot(QTableWidgetItem *item)
//{
//    if(item == Q_NULLPTR){
//        return;
//    }
//    int row = ui->renameFileListWgt->row(item);
//    updateReferListByRenameFileListRow(row);
//}

void RenameFileName::updateReferListClickedSlot(QModelIndex index)
{
    int row = index.row();
    updateReferListByRenameFileListRow(row);
}


void RenameFileName::renameDirPathByCharMap(const QStringList &oldDirPathList, QStringList &newDirPathList)
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
        newDirPathList.append(newPathParent + name);
    }
}
void RenameFileName::renameFilePathByCharMap(const QStringList &oldFilePathList, QStringList &newFilePathList) {
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
        newFilePathList.append(newPathParent + name);
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
    backUpPath_ = dir.absolutePath() + "/" + repoPath_.split("/").last() +".backup/" + renameDirPath_.split("/").last();
    dir.mkdir(dir.absolutePath() + "/" + repoPath_.split("/").last() +".backup");
    if(!BackUp(renameDirPath_, backUpPath_, true)){
        emit sigRenameFileNameLog(QString("Data backup Failed"));
        return;
    }
    updateOldPath(renameDirPath_, oldDirPathList, oldFilePathList);
    // 2. 根据字符替换修改 Dir 和 File
    QStringList newDirPathList;
    QStringList newFilePathList;
    renameDirPathByCharMap(oldDirPathList, newDirPathList);
    renameFilePathByCharMap(oldFilePathList, newFilePathList);

    // 3. 根据修改结果更新数据信息  replaceNameDirInfoList_和 replaceNameFileInfoList_
    for(int i = 0; i < oldDirPathList.size(); ++i) {
        DirRenameInfo dirRenameInfo;
        dirRenameInfo.oldDirPath = oldDirPathList.at(i);
        replaceNameDirInfoList_.append(dirRenameInfo);
    }
    for(int i = 0; i < oldFilePathList.size(); ++i) {
        FileRenameInfo fileRenameInfo;
        fileRenameInfo.oldFilePath = oldFilePathList.at(i);
        replaceNameFileInfoList_.append(fileRenameInfo);
    }

    // 4. 创建对比文件名修改文件
    QFile::remove(backUpPath_ + "/" + NEW_FILE_NAME);
    QFile::remove(backUpPath_ + "/" + OLD_FILE_NAME);
    createCompareDirAndFileLog(backUpPath_ + "/" + OLD_FILE_NAME, oldDirPathList, oldFilePathList);
    createCompareDirAndFileLog(backUpPath_ + "/" + NEW_FILE_NAME, newDirPathList, newFilePathList);
    emit sigRenameFileNameLog(QString("Data backup and generate file list Success!"));
//    emit sigRenameFileVSCodeOpenList(backUpPath_ + "/" + OLD_FILE_NAME, backUpPath_ + "/" + NEW_FILE_NAME);
    updateReplaceInfoSlot();
}

void RenameFileName::updateReplaceInfoSlot() {

    if(!readNewFileList()){
        emit sigRenameFileNameLog(QString("Please confirm whether to Edit name information !"));
    }
    // 5. 只保留不同的项
    for(int i = 0; i < replaceNameDirInfoList_.size(); ++i) {
        if(replaceNameDirInfoList_.at(i).oldDirPath == replaceNameDirInfoList_.at(i).newDirPath) {
            replaceNameDirInfoList_.remove(i);
            --i;
        }
    }
    for(int i = 0; i < replaceNameFileInfoList_.size(); ++i) {
        if(replaceNameFileInfoList_.at(i).oldFilePath == replaceNameFileInfoList_.at(i).newFilePath) {
            replaceNameFileInfoList_.remove(i);
            --i;
        }
    }
    fileOperaton_->updateReplaceNameRefereceList(repoPath_ ,renameDirPath_, replaceNameDirInfoList_, replaceNameFileInfoList_);
    updateRenameFileList();
}


void RenameFileName::renameListClear() {
    // 只删除内容，不删除表头
    ui->renameFileListWgt->clearContents();
    ui->referFileListWgt->clearContents();
}

void RenameFileName::renameByListFile() {


}

void RenameFileName::on_EditNameFilePbn_clicked()
{
    if(ui->charRadioBtn->isChecked()){
        renameByConfFile();
    }else{
        renameByListFile();
    }
}

void RenameFileName::on_ChooseDirPbn_clicked()
{
    renameDirPath_ = QFileDialog::getExistingDirectory(this,"选择目录",repoPath_,QFileDialog::ShowDirsOnly);
    ui->RenameDirPathEdit->setText(renameDirPath_);
}

void RenameFileName::on_ChooseConfPbn_clicked()
{
    renameConfPath_ = QFileDialog::getExistingDirectory(this,"选择目录",repoPath_,QFileDialog::ShowDirsOnly);
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

///////////////////  replace   /////////////
/*
QString RenameFileName::replaceNewFile(QDir newFileDir, QDir curFileDir, QString line, QString oldFileName) {
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


bool RenameFileName::modifyRefMarkdown(const QString& refFilePath, const QString& oldFileName,
                                       const QString& newFilePath){
    QString strAll;
    QStringList strList;
    QDir curFileDir(refFilePath);
    QDir newFileDir(newFilePath);
    if(curFileDir.path() == newFileDir.path()){
        qDebug() << "This is itself";
        return true;
    }

    QFile readFile(refFilePath);		//PATH是自定义读取文件的地址
    if(!readFile.open((QIODevice::ReadOnly|QIODevice::Text)))
    {
        qDebug()<< "Can't Read the RefMarkdown file!";
        return false;
    }
    //把文件所有信息读出来
    QTextStream stream(&readFile);
    strAll = stream.readAll();
    //    readFile.close();
    QFile writeFile(refFilePath);	//PATH是自定义写文件的地址
    if(!writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug()<< "Can't write the RefMarkdown file!";
        return false;
    }
    QTextStream wStream(&writeFile);
    strList = strAll.split("\n");           //以换行符为基准分割文本
    for(int i = 0;i < strList.count();i++)    //遍历每一行
    {
        if(strList.at(i).contains(oldFileName))
        {
            qDebug() << strList.at(i);
            wStream << replaceNewFile(newFileDir, curFileDir, strList.at(i), oldFileName) << '\n';
        } //如果没有找到要替换的内容，照常写入
        else
        {
            if(i == strList.count()-1)
            {
                wStream << strList.at(i);
            }
            else
            {
                wStream << strList.at(i) << '\n';
            }
        }
    }
    writeFile.close();
    return true;
}
*/

void RenameFileName::on_ReplaceByListPbn_clicked()
{
//    bool status = true;
//    for(int i = 0; i < replaceNameDirInfoList_.size(); ++i) {
//            QDir dir = QDir(oldFilePath);
//            QString oldFilePath = renameDirPath_ + "/" + replaceNameDirInfoList_.at(i).oldDirPath;
//            QString oldFileName = dir.dirName();
//            QString newFilePath = renameDirPath_ +"/" + replaceNameDirInfoList_.at(i).newDirPath;
//            // 1. 移动之前需要检查父目录是否被修改
//            while(dir.currentPath() != renameDirPath_) {
//            dir.cdUp();
//            QString path = dir.relativeFilePath(renameDirPath_);
//            for(int j = 0; j < i; ++j) {
//                if(replaceNameDirInfoList_.at(j).oldDirPath == path) {
//                    replaceNameDirInfoList_.at(i).newDirPath
//                }
//            }
//            }


//            if(!QFile::rename(oldFilePath, newFilePath)){
//                status = false;
//                emit sigRenameFileNameLog(QString("Move ") + oldFileAndDirNameList_.at(i) + " Failed ！");
//            }else{
//                QMap<QString, int>::iterator iter = refFilePathAndNum_.begin();
//                while (iter != refFilePathAndNum_.end())
//                {
//                    modifyRefMarkdown(iter.key(), oldFileName, newFilePath);
//                    iter++;
//                }
//            }
//    }
}

