#include "rename_file_name.h"
#include "ui_rename_file_name.h"
#include <QDebug>
#include <QMessageBox>
#include <QTextCodec>
#include <QClipboard>
#include <QSplitter>
#include <QAction>


RenameFileName::RenameFileName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameFileName)
{
    ui->setupUi(this);
    fileOperaton_ = new FileOperation(this);
    confFile_ = new confDialog;
    renameListMenu_ = new QMenu(this);
    QSplitter *splitterList = new QSplitter(Qt::Horizontal,nullptr); // 水平布置
    splitterList->addWidget(ui->referFileListWgt);
    splitterList->addWidget(ui->referContextListWgt);
    ui->referLayout->addWidget(splitterList);
    splitterList->show();
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
    ui->renameFileListWgt->setColumnCount(4);     //设置列数
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
    connect(ui->renameFileListWgt,&QTableWidget::clicked,this, &RenameFileName::updateReferListClickedSlot);
    connect(ui->renameFileListWgt,&QTableWidget::itemChanged,this, &RenameFileName::updateReferListEditSlot);
    // rightClickMenu
    connect(ui->renameFileListWgt,&QTableWidget::customContextMenuRequested,this,&RenameFileName::showMenuSlot);
// =======================================================================
    ui->referFileListWgt->setColumnCount(5);     //设置列数
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
    connect(ui->referFileListWgt,&QTableWidget::clicked,this, &RenameFileName::updateReferTextClickedSlot);
// =======================================================================
    ui->referContextListWgt->setColumnCount(3);     //设置列数
    ui->referContextListWgt->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置每行内容不可编辑
    ui->referContextListWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*去掉每行的行号*/
    ui->referContextListWgt->verticalHeader()->setHidden(true);
    ui->referContextListWgt->horizontalHeader()->setStretchLastSection(true);
    // 消除表格控件的边框
    ui->referContextListWgt->setFrameShape(QFrame::NoFrame);
    //设置表格不显示格子线
    ui->referContextListWgt->setShowGrid(false); //设置不显示格子线
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
    QAction *actCopyList = new QAction("拷贝列表",renameListMenu_);
    QAction *actCopyItem = new QAction("拷贝当前项",renameListMenu_);
    renameListMenu_->addAction(actCopyList);
    renameListMenu_->addAction(actCopyItem);
    connect(actCopyList, &QAction::triggered, this, &RenameFileName::copyOldFileListSlot);
    connect(actCopyItem, &QAction::triggered, this, &RenameFileName::copyOldFileItemSlot);
}

void RenameFileName::showMenuSlot(QPoint pos) {
    renameListMenu_->move(cursor().pos());
    renameListMenu_->show();
    QModelIndex index = ui->renameFileListWgt->indexAt(pos);
    ui->renameFileListWgt->setCurrentIndex(index);
}

void RenameFileName::copyOldFileListSlot(){
    QClipboard* clip = QApplication::clipboard();
    QString clipText;
    for(int i = 0; i < ui->renameFileListWgt->rowCount(); ++i){
        clipText += ui->renameFileListWgt->item(i,2)->text() + "\n";
    }
    clip->setText(clipText);
}
void RenameFileName::copyOldFileItemSlot(){
    QClipboard* clip = QApplication::clipboard();
    QString clipText = ui->renameFileListWgt->currentItem()->text();
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
        QTableWidgetItem *pItem1 = new QTableWidgetItem(dirInfo.newDirPath);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(dirInfo.oldDirPath);
        QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::number(dirInfo.reDirList.size()));
        pItem3->setTextAlignment(Qt::AlignCenter);
        if(dirInfo.reDirList.size() > 0){
            pItem0->setBackground(QBrush(Qt::gray));        // 设置背景色
        }
        if(dirInfo.newDirPath != dirInfo.oldDirPath) {
            pItem0->setForeground(QBrush(Qt::red));         // 设置字体颜色
        }
        pItem0->setFlags(pItem0->flags() & ~Qt::ItemIsEditable);
        pItem0->setFlags(pItem0->flags() & ~Qt::ItemIsSelectable);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        pItem3->setFlags(pItem3->flags() & ~Qt::ItemIsEditable);
        ui->renameFileListWgt->setItem(j, 0, pItem0);
        ui->renameFileListWgt->setItem(j, 1, pItem1);
        ui->renameFileListWgt->setItem(j, 2, pItem2);
        ui->renameFileListWgt->setItem(j, 3, pItem3);
        n++;
    }
    int num = replaceNameDirInfoList_.size();
    for(int j = 0; j < replaceNameFileInfoList_.size(); ++j) {
        ui->renameFileListWgt->setRowCount(num + j + 1);
        FileRenameInfo fileInfo = replaceNameFileInfoList_.at(j);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::number(n)); pItem0->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(fileInfo.newFilePath.split("/").last());
        QTableWidgetItem *pItem2 = new QTableWidgetItem(fileInfo.oldFilePath.split("/").last());
        QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::number(fileInfo.reFileList.size()));
        pItem3->setTextAlignment(Qt::AlignCenter);
        if(fileInfo.reFileList.size() > 0){
            pItem0->setBackground(QBrush(Qt::gray));        // 设置背景色
        }
        if(fileInfo.newFilePath != fileInfo.oldFilePath) {
            pItem0->setForeground(QBrush(Qt::red));         // 设置字体颜色
        }
        pItem0->setFlags(pItem0->flags() & ~Qt::ItemIsEditable);
        pItem0->setFlags(pItem0->flags() & ~Qt::ItemIsSelectable);
        pItem2->setFlags(pItem2->flags() & ~Qt::ItemIsEditable);
        pItem3->setFlags(pItem3->flags() & ~Qt::ItemIsEditable);
        ui->renameFileListWgt->setItem(num + j, 0, pItem0);
        ui->renameFileListWgt->setItem(num + j, 1, pItem1);
        ui->renameFileListWgt->setItem(num + j, 2, pItem2);
        ui->renameFileListWgt->setItem(num + j, 3, pItem3);
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
    ui->referFileListWgt->setRowCount(0);
    ui->referContextListWgt->clearContents();
    ui->referContextListWgt->setRowCount(0);
    if(reFileList.isEmpty()) {
        return;
    }
    int n = 1;
    for(int j = 0; j < reFileList.size(); ++j) {
        ui->referFileListWgt->setRowCount(j + 1);
        ReFile reFile = reFileList.at(j);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::number(n)); pItem0->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem1 = new QTableWidgetItem(reFile.reFilePath.split("/").last());
        QTableWidgetItem *pItem2 = new QTableWidgetItem(QString::number(reFile.reAsJumpCount));     pItem2->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem3 = new QTableWidgetItem(reFile.reFilePath);
        QTableWidgetItem *pItem4 = new QTableWidgetItem(reFile.lastModifyTime.toString("yyyy-MM-dd HH:mm:ss ddd"));
        ui->referFileListWgt->setItem(j, 0, pItem0);
        ui->referFileListWgt->setItem(j, 1, pItem1);
        ui->referFileListWgt->setItem(j, 2, pItem2);
        ui->referFileListWgt->setItem(j, 3, pItem3);
        ui->referFileListWgt->setItem(j, 4, pItem4);
        n++;
    }
    // 触发双击表头分割线信号，实现自动调整行宽
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(0);
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(1);
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(2);
    emit ui->referFileListWgt->horizontalHeader()->sectionHandleDoubleClicked(3);
}

QString RenameFileName::highlightDifferences(const QString& str1, const QString& str2) {
    QString highlightedStr;
    // " /\;:[]()."
    QRegularExpression regex("[\\s/\\\\;:\\[\\]\\(\\)\\.]");
    QRegularExpressionMatchIterator iterator1 = regex.globalMatch(str1);
    QRegularExpressionMatchIterator iterator2 = regex.globalMatch(str2);
    int prevIndex1 = 0;
    int prevIndex2 = 0;

    while (iterator1.hasNext() && iterator2.hasNext()) {
        QRegularExpressionMatch match1 = iterator1.next();
        QRegularExpressionMatch match2 = iterator2.next();
        int index1 = match1.capturedStart();
        int index2 = match2.capturedStart();

        // 高亮不同之处 <body style="background-color: green; color: white;">
        if (str1.mid(prevIndex1, index1 - prevIndex1) != str2.mid(prevIndex2, index2 - prevIndex2)) {
            highlightedStr += "<span style=\"background-color: rgb(76, 25, 25);color: white\">" +
                              str2.mid(prevIndex2, index2 - prevIndex2) +
                              "</span>";
        } else {
            highlightedStr += str2.mid(prevIndex2, index2 - prevIndex2);
        }

        // 添加分隔符
        highlightedStr += str2.mid(index2, match2.capturedLength());

        prevIndex1 = index1 + match1.capturedLength();
        prevIndex2 = index2 + match2.capturedLength();
    }

    // 处理剩余的字符
    highlightedStr += str2.mid(prevIndex2);

    return highlightedStr;
}

void RenameFileName::updateContextList(const QVector<ReText>& reTextList) {
// 参考 RenameFileName::updateReferList(const QVector<ReFile>& reFileList)
    ui->referContextListWgt->clearContents();
    ui->referContextListWgt->setRowCount(0);
    if(reTextList.isEmpty()) {
        return;
    }
    int n = 1;
    for(int j = 0; j < reTextList.size(); ++j) {
        ui->referContextListWgt->setRowCount(j + 1);
        ReText reTxt = reTextList.at(j);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::number(n)); pItem0->setTextAlignment(Qt::AlignCenter);
        QTableWidgetItem *pItem1 = new QTableWidgetItem("(" + QString::number(reTxt.x) + ", " + QString::number(reTxt.y) + ")");
        pItem1->setTextAlignment(Qt::AlignCenter);
        QString oldTxt = reTxt.oldText;
        QString newTxt = reTxt.newText;
        if(oldTxt.contains("src=") || newTxt.contains("src=")) {
            oldTxt.replace("<", "&lt;"); oldTxt.replace(">", "&gt;");
            newTxt.replace("<", "&lt;"); newTxt.replace(">", "&gt;");
        }
        QString strHtmlText =  "<p>" + oldTxt + "   <br> " + highlightDifferences(oldTxt, newTxt) + "  </p>";
        QLabel* pLabel = new QLabel;
        pLabel->setText(strHtmlText);

        QFontMetrics fontMetrics = pLabel->fontMetrics();
        int iHeight = fontMetrics.boundingRect(pLabel->text()).height() + 2;
        ui->referContextListWgt->setRowHeight(j, iHeight * 2);
        ui->referContextListWgt->setItem(j, 0, pItem0);
        ui->referContextListWgt->setCellWidget(j, 1, pLabel);
        ui->referContextListWgt->setItem(j, 2, pItem1);
        n++;
    }
    // 触发双击表头分割线信号，实现自动调整行宽
    emit ui->referContextListWgt->horizontalHeader()->sectionHandleDoubleClicked(0);
    emit ui->referContextListWgt->horizontalHeader()->sectionHandleDoubleClicked(1);
    emit ui->referContextListWgt->horizontalHeader()->sectionHandleDoubleClicked(2);
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

void RenameFileName::updateReferTextClickedSlot(QModelIndex item)
{
    int row = item.row();
    int rowRenameFileList = ui->renameFileListWgt->currentRow();
    int len = replaceNameDirInfoList_.size();
    QVector<ReFile> reFileList;
    if(rowRenameFileList < len) {
        reFileList = replaceNameDirInfoList_.at(rowRenameFileList).reDirList;
    }else {
        reFileList = replaceNameFileInfoList_.at(rowRenameFileList - len).reFileList;
    }

    QVector<ReText> reTextList = reFileList.at(row).reTextList;
    updateContextList(reTextList);
}

void RenameFileName::updateReferListClickedSlot(QModelIndex index)
{
    int row = index.row();
    updateReferListByRenameFileListRow(row);
    setSeletedItemStyleSheet(index);
}

void RenameFileName::setSeletedItemStyleSheet(QModelIndex item) {
//    QList<QTableWidgetItem*> items = ui->renameFileListWgt->selectedItems();
//    if(!items.empty()) //表格有某行被选中
//    {
//        //获取该行的成员数
//        int count = items.count();
//        // 打印该行所有成员内容
//        if(items.at(2)->text() != items.at(3)->text()) {
////            items.at(2)->setForeground(QBrush(QColor(72,201,176)));
//            ui->renameFileListWgt->setStyleSheet(
//                        "QTableWidget::item:selected {color: red; }"
//                        );
//        }else{
//            ui->renameFileListWgt->setStyleSheet(
//                        "QTableWidget::item:selected { color: rgb(220,220,220) }"
//                        );
////            items.at(2)->setForeground(QBrush(QColor(220,220,220)));
//        }
//        if(items.at(1)->text().toInt() != 0) {
//            ui->renameFileListWgt->setStyleSheet(
//                        "QTableWidget::item:selected {color: red; }"
//                        );
//        }else{
//            ui->renameFileListWgt->setStyleSheet(
//                        "QTableWidget::item:selected { color: rgb(220,220,220) }"
//                        );
//        }
//    }
}

void RenameFileName::updateReferListEditSlot(QTableWidgetItem *item) {
    int row = item->row();
    QString name = item->text();
    int len = replaceNameDirInfoList_.size();
    if(row < len && replaceNameDirInfoList_.at(row).newDirPath != name) {
        QString oldPathChanged = replaceNameDirInfoList_.at(row).oldDirPath;
        oldPathChanged.replace(oldPathChanged.split("/").last(), name);
        replaceNameDirInfoList_[row].newDirPath = oldPathChanged;
        QVector<ReFile> reDirList = replaceNameDirInfoList_[row].reDirList;
        replaceNameDirInfoList_[row].reDirList.clear();
        for(int i = 0; i < reDirList.size(); ++i) {
            QString markdownAbsPath = repoPath_ + "/" + reDirList.at(i).reFilePath;
            QDir curDir(markdownAbsPath);
            curDir.setCurrent(markdownAbsPath);
           // 3. 更新引用列表和内容
            fileOperaton_->updateReferDirByFilePath(markdownAbsPath, repoPath_,  curDir, renameDirPath_, replaceNameDirInfoList_, row);
        }
        if(oldPathChanged != replaceNameDirInfoList_.at(row).oldDirPath) {
            ui->renameFileListWgt->takeItem(row, 0)->setForeground(QBrush(Qt::red));
        }else{
            ui->renameFileListWgt->takeItem(row, 0)->setForeground(QBrush(QColor(220,220,220)));
        }
    }else if(row - len >= 0 && replaceNameFileInfoList_.length() > (row - len) && replaceNameFileInfoList_.at(row - len).newFilePath != name) {
        QString oldPath = replaceNameFileInfoList_.at(row - len).oldFilePath;
        oldPath.replace(oldPath.split("/").last(), name);
        replaceNameFileInfoList_[row - len].newFilePath = oldPath;
        QVector<ReFile> reFileList = replaceNameFileInfoList_[row - len].reFileList;
        // 清空引用列表
        replaceNameFileInfoList_[row - len].reFileList.clear();
        for(int i = 0; i < reFileList.size(); ++i) {
            QString markdownAbsPath = repoPath_ + "/" + reFileList.at(i).reFilePath;
            QDir curDir(markdownAbsPath);
            curDir.setCurrent(markdownAbsPath);
            // 3. 更新引用列表和内容
            fileOperaton_->updateReferFileByFilePath(markdownAbsPath, repoPath_,  curDir, renameDirPath_, replaceNameFileInfoList_, row - len);
        }
        if(oldPath != replaceNameFileInfoList_.at(row - len).oldFilePath) {
             ui->renameFileListWgt->takeItem(row, 0)->setForeground(QBrush(Qt::red));
        }else{
             ui->renameFileListWgt->takeItem(row, 0)->setForeground(QBrush(QColor(220,220,220)));
        }
    }else{
        return;
    }
     // 4. 更新界面列表
    updateRenameFileList();
    emit sigRenameFileNameLog(QString("更新文件命名数据完成!"));
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
// 1. 备份文件
void RenameFileName::backupRenameDir() {
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
}

// 只是将特殊字符进行替换
void RenameFileName::renameByConfFile() {
    // 1.读取配置文件
    if(!confFile_->readCharConfFile(renameConfPath_, replaceCharMap_)) {
        emit sigRenameFileNameLog(QString("Rename Configure file Open Error!"));
        return;
    }
    QStringList oldDirPathList;
    QStringList oldFilePathList;
    updateOldPath(renameDirPath_, oldDirPathList, oldFilePathList);
    // 2.根据字符替换修改 Dir 和 File,更新新旧文件名
    renameDirPathByCharMap(oldDirPathList);
    renameFilePathByCharMap(oldFilePathList);
}

void RenameFileName::renameListClear() {
    // 只删除内容，不删除表头
    ui->renameFileListWgt->clearContents();
    ui->renameFileListWgt->setRowCount(0);

    ui->referFileListWgt->clearContents();
    ui->referFileListWgt->setRowCount(0);

    ui->referContextListWgt->clearContents();
    ui->referContextListWgt->setRowCount(0);
}

void RenameFileName::renameByListFile() {
    fileOperaton_->updateReplaceNameByList(renameListPath_, replaceNameDirInfoList_, replaceNameFileInfoList_);
}

void RenameFileName::on_refreshPbn_clicked()
{
    replaceNameDirInfoList_.clear(); // 更新后目录列表
    replaceNameFileInfoList_.clear(); // 更新后文件名列表
    renameListClear();
    // 1. 是否备份
    if(ui->backupCheckBox->isChecked()) {
        backupRenameDir();
    }
    // 2. 根据字符串替换更新文件名
    if(ui->charRadioBtn->isChecked()){
        renameByConfFile();
    }
    // 3. 根据文件列表更新文件名
    if(ui->listRadioBtn->isChecked()){
        renameByListFile();
    }
    // 4. 是否需要包含目录或者文件
    if(!includeDir_) {
        replaceNameDirInfoList_.clear();
    }
    if(!includeFile_) {
        replaceNameFileInfoList_.clear();
    }
    // 5. 更新引用文件以及引用的内容
    fileOperaton_->updateReplaceNameRefereceList(repoPath_ ,renameDirPath_, replaceNameDirInfoList_, replaceNameFileInfoList_);
    // 6. 更新界面列表
    updateRenameFileList();
    emit sigRenameFileNameLog(QString("更新文件命名数据完成!"));
}

void RenameFileName::on_ChooseDirPbn_clicked()
{
    renameDirPath_ = QFileDialog::getExistingDirectory(this,"选择重命名目录",repoPath_,QFileDialog::ShowDirsOnly);
    ui->RenameDirPathEdit->setText(renameDirPath_);
    on_refreshPbn_clicked();
}

void RenameFileName::on_ChooseConfPbn_clicked()
{
    renameConfPath_ = QFileDialog::getOpenFileName(this,"选择配置文件",repoPath_,tr("Json files(*.json);;txt files(*.txt);;All files(*.*)"));
    ui->NameConfFilePathEdit->setText(renameConfPath_);
    on_refreshPbn_clicked();
}

void RenameFileName::on_OpenFilePathPbn_clicked()
{
    emit sigRenameFileOpenPath(renameDirPath_);
}

void RenameFileName::on_OpenConfFilePbn_clicked()
{
    emit sigRenameFileConfFile(renameConfPath_);
}

bool RenameFileName::modifyReferMarkdown(const QString& reMarkdownPath, const QVector<ReText>& reTextList) {
    QString markdownPathAbs = repoPath_ + "/" + reMarkdownPath;

    QFile data(markdownPathAbs);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return false;
    }
    QString context;
    QTextStream stream(&data);
    context = stream.readAll();
    data.close();
    // 3. 重新打开，替换写入
    QFile writeFile(markdownPathAbs);	//PATH是自定义写文件的地址
    if(!writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug()<< "Can't write the RefMarkdown file!";
        return false;
    }
    QTextStream wStream(&writeFile);
    int dt = 0;

    for(int i = 0; i < reTextList.size(); ++i) {
        int len = reTextList.at(i).y - reTextList.at(i).x + 1;
        context.replace(reTextList.at(i).x + dt, len, reTextList.at(i).newText);
        dt += (reTextList.at(i).newText.length() - reTextList.at(i).oldText.length());
    }
    wStream << context;
    writeFile.close();
    return true;
}

void RenameFileName::on_ReplaceByListPbn_clicked()
{
    int n = 0;
    // 更新修改的目录
    for(int i = 0; i < replaceNameDirInfoList_.size(); ++i, n++) {
        if(replaceNameDirInfoList_.at(i).oldDirPath != replaceNameDirInfoList_.at(i).newDirPath) {
            QString oldPathAbs = renameDirPath_ + "/" + replaceNameDirInfoList_.at(i).oldDirPath;
            QString newPathAbs = renameDirPath_ + "/" + replaceNameDirInfoList_.at(i).newDirPath;
            if(!QFile::rename(oldPathAbs, newPathAbs)) {
                emit sigRenameFileNameLog(QString("Move ") + replaceNameDirInfoList_.at(i).oldDirPath + " Failed ！");
            }else{
                QVector<ReFile> reFileList = replaceNameDirInfoList_.at(i).reDirList;
                for(int j = 0; j < reFileList.size(); ++j) {
                    ReFile reFile = reFileList.at(j);
                    QDateTime lastModifyTime = QFileInfo(repoPath_ + "/" + reFile.reFilePath).lastModified();
                    if(lastModifyTime != reFile.lastModifyTime) {
                        reFile.reTextList =
                        fileOperaton_->updateReTextDir(repoPath_ + "/" + reFile.reFilePath, renameDirPath_, replaceNameDirInfoList_, i);
                    }
                    modifyReferMarkdown(reFile.reFilePath ,reFile.reTextList);
                }
            }
        }
    }
    // 更新修改的文件
    for(int i = 0; i < replaceNameFileInfoList_.size(); ++i, n++) {
        if(replaceNameFileInfoList_.at(i).oldFilePath != replaceNameFileInfoList_.at(i).newFilePath) {
            QString oldPathAbs = renameDirPath_ + "/" + replaceNameFileInfoList_.at(i).oldFilePath;
            QString newPathAbs = renameDirPath_ + "/" + replaceNameFileInfoList_.at(i).newFilePath;
            if(!QFile::rename(oldPathAbs, newPathAbs)) {
                emit sigRenameFileNameLog(QString("Move ") + replaceNameFileInfoList_.at(i).oldFilePath + " Failed ！");
            }else{
                QVector<ReFile> reFileList = replaceNameFileInfoList_.at(i).reFileList;
                for(int j = 0; j < reFileList.size(); ++j) {
                    ReFile reFile = reFileList.at(j);
                    QDateTime lastModifyTime = QFileInfo(repoPath_ + "/" + reFile.reFilePath).lastModified();
                    if(lastModifyTime != reFile.lastModifyTime) {
                        reFile.reTextList =
                        fileOperaton_->updateReTextFile(repoPath_ + "/" + reFile.reFilePath, renameDirPath_, replaceNameFileInfoList_, i);
                    }
                    modifyReferMarkdown(reFile.reFilePath, reFile.reTextList);
                }
            }
        }
    }
    on_refreshPbn_clicked();
}

void RenameFileName::on_ChooseListPbn_clicked()
{
    renameListPath_ = QFileDialog::QFileDialog::getOpenFileName(this,"选择配置文件",repoPath_,tr("txt files(*.txt);;Json files(*.json);;All files(*.*)"));
    ui->NameListPathEdit->setText(renameListPath_);
    on_refreshPbn_clicked();
}

void RenameFileName::on_OpenListFilePbn_clicked()
{
    emit sigRenameFileConfFile(renameListPath_);
}

void RenameFileName::setSize(ScreenRes screen) {
//    pathConfBox
    if(screen == ScreenRes::High){
        this->setMinimumSize(QSize(1600, 1000));
        this->resize(QSize(1600, 1000));
        ui->pathConfBox->setMinimumHeight(200);
        ui->pathConfBox->setMaximumHeight(200);
        ui->replaceStrategyBox->setMinimumHeight(200);
        ui->replaceStrategyBox->setMaximumHeight(200);
    }else{
        this->setMinimumSize(QSize(700, 600));
        this->resize(QSize(700, 600));
        ui->pathConfBox->setMinimumHeight(100);
        ui->pathConfBox->setMaximumHeight(100);
        ui->replaceStrategyBox->setMinimumHeight(100);
        ui->replaceStrategyBox->setMaximumHeight(100);
    }
}

void RenameFileName::on_RenameDirPathEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    renameDirPath_ = arg1;
    renameListClear();
}


void RenameFileName::on_NameConfFilePathEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    renameConfPath_ = arg1;
    renameListClear();
}


void RenameFileName::on_NameListPathEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    renameListPath_ = arg1;
    renameListClear();
}

void RenameFileName::on_charRadioBtn_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    renameListClear();
}

void RenameFileName::on_listRadioBtn_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    renameListClear();
}

void RenameFileName::on_dirCheckBox_stateChanged(int arg1)
{
    if(arg1) {
        includeDir_= true;
    }else{
        includeDir_= false;
    }
    on_refreshPbn_clicked();
}

void RenameFileName::on_fileCheckBox_stateChanged(int arg1)
{
    if(arg1) {
        includeFile_= true;
    }else{
        includeFile_= false;
    }
    on_refreshPbn_clicked();
}

void RenameFileName::on_addNumRadioBtn_stateChanged(int arg1)
{
    if(arg1) {
        renameByNum(true);
    }else{
        renameByNum(false);
    }
}

void RenameFileName::renameByNum(bool status) {
    // 待完成。。。
    int len = ui->renameFileListWgt->rowCount();
    for(int i = 0; i < len; ++i) {
        QString name = ui->renameFileListWgt->item(i, 1)->text();
        // 1. 如果name前5个字符中有-，前面是数字，先删除
        if(name.length() > 4 && name.left(4).contains("-")) {
            bool isNumber = false;
            name.split("-").first().toInt(&isNumber);
            int pos;
            if(isNumber && (pos = name.indexOf("-")) < name.size() - 2) {
                name.remove(0, pos + 1);
            }
        }
        // 2. name前面加上 序号 i+1 -
        if(status) {
            name = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0'))+ "-" + name;
        }
        // 3. 刷新界面 ： 注意事项，需要看下，修改item不会自动刷新列表，如果会的话，就不用刷新界面了
        if(name != ui->renameFileListWgt->item(i, 1)->text()) {
            ui->renameFileListWgt->item(i, 1)->setText(name);
        }
    }
}
