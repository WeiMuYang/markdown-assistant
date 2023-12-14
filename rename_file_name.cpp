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
    status_ = StatusType::OK;
    initRenameFileList();
}

void RenameFileName::setPath(const QString& path){
    tarPath_ = path;
    filePath_ = tarPath_;
    ui->PathEdit->setText(filePath_);
}

RenameFileName::~RenameFileName()
{
    delete ui;
}

void RenameFileName::on_ChoosePbn_clicked()
{
    filePath_ = QFileDialog::getExistingDirectory(this,"选择目录",tarPath_,QFileDialog::ShowDirsOnly);
    ui->PathEdit->setText(filePath_);
}

// TODO: 仅仅支持修改当前文件夹中的文件   修改列表修改： 不包括子目录
void RenameFileName::on_ReplaceByListPbn_clicked()
{
    QStringList nameList;
    if(-1 != ui->NameListEdit->text().indexOf(',')){
        nameList = ui->NameListEdit->text().split(',');
    }else{
        nameList = ui->NameListEdit->text().split(QRegularExpression("\\s+"));;
    }
    int n = 0;
    QDir dir(filePath_);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    ui->renameFileListWgt->clear();
    foreach(QFileInfo fileInfo, list)
    {
        if(ui->dirRadioBtn->isChecked() && fileInfo.isDir() && n < nameList.size()){

            ui->renameFileListWgt->setRowCount(n + 1);
            QTableWidgetItem *pItem1 = new QTableWidgetItem(fileInfo.fileName());
            QTableWidgetItem *pItem2 = new QTableWidgetItem(nameList.at(n) + fileInfo.suffix());
            ui->renameFileListWgt->setItem(n, 0, pItem1);
            ui->renameFileListWgt->setItem(n, 1, pItem2);
            dir.rename(fileInfo.fileName(), nameList.at(n) + fileInfo.suffix());
            n++;
        }
        if(ui->fileBtn->isChecked() && fileInfo.isFile() && n < nameList.size()) //book  study  test目录
        {

            ui->renameFileListWgt->setRowCount(n + 1);
            QTableWidgetItem *pItem1 = new QTableWidgetItem(fileInfo.fileName());
            QTableWidgetItem *pItem2 = new QTableWidgetItem(nameList.at(n) +"." + fileInfo.suffix());
            ui->renameFileListWgt->setItem(n, 0, pItem1);
            ui->renameFileListWgt->setItem(n, 1, pItem2);
            dir.rename(fileInfo.fileName(), nameList.at(n) + nameList.at(n) + "."  + fileInfo.suffix());
            n++;
        }
    }
}

//拷贝文件夹
bool RenameFileName::copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
{
    QDir sourceDir(fromDir);
    QDir targetDir(toDir);
    if(!targetDir.exists()){    /**< 如果目标目录不存在，则进行创建 */
        if(!targetDir.mkdir(targetDir.absolutePath()))
            return false;
    }

    QFileInfoList fileInfoList = sourceDir.entryInfoList();
    foreach(QFileInfo fileInfo, fileInfoList){
        if(fileInfo.fileName() == "." || fileInfo.fileName() == "..")
            continue;

        if(fileInfo.isDir()){    /**< 当为目录时，递归的进行copy */
            if(!copyDirectoryFiles(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()), coverFileIfExist)){
                return false;
            }else{
                QDir curDir(tarPath_);
                curDir.setCurrent(filePath_);
                QString filePath = curDir.relativeFilePath(fileInfo.filePath());
                QDir fileDir = QDir(fileInfo.filePath());
                oldFileAndDirNameList_.append(filePath);
            }
        }
        else{
            if(coverFileIfExist && targetDir.exists(fileInfo.fileName())){
                targetDir.remove(fileInfo.fileName());
            }

            /// 进行文件copy
            if(!QFile::copy(fileInfo.filePath(), targetDir.filePath(fileInfo.fileName()))){
                return false;
            }else{
                QDir curDir(tarPath_);
                curDir.setCurrent(filePath_);
                QString filePath = curDir.relativeFilePath(fileInfo.filePath());
                oldFileAndDirNameList_.append(filePath);
            }
        }
    }
    return true;
}

void RenameFileName::on_ModifyNameFilePbn_clicked()
{
    status_ = StatusType::OK;
    oldFileAndDirNameList_.clear();
    // 1. 备份文件
    QString backUpPath = filePath_+".backup";
    if(!copyDirectoryFiles(filePath_, backUpPath, true)){
        emit sigRenameFileNameLog(QString("Data backup Failed"));
        return;
    }
    // 2. 创建文件名修改文件
    QFile::remove(backUpPath + "/" + NEW_FILE_NAME);
    QFile::remove(backUpPath + "/" + OLD_FILE_NAME);
    QFile file(backUpPath + "/" + OLD_FILE_NAME);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        QTextStream stream(&file);
        for(int i = 0; i < oldFileAndDirNameList_.size(); ++i){
            QDir curFileDir(filePath_+"/" + oldFileAndDirNameList_.at(i));
            QString str = curFileDir.dirName();
            int desiredLength = 40;
            // 使用UTF-8编码
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
            int byteLength = codec->fromUnicode(str).length();
            int numChineseChars = (byteLength - str.length()) / 2;
            QString paddedStr = str.leftJustified(desiredLength - numChineseChars);
            QString txt = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')) + QString(" |") + paddedStr + QString("|") + oldFileAndDirNameList_.at(i);
            stream << txt << "\n";
        }
        file.close();
    }

    if(!QFile::copy(backUpPath + "/" + OLD_FILE_NAME, backUpPath + "/" + NEW_FILE_NAME))
    {
        emit sigRenameFileNameLog(QString("Generate file list Failed!"));
    }
    emit sigRenameFileNameLog(QString("Data backup and generate file list Success!"));
    emit sigRenameFileVSCodeOpenList(backUpPath + "/" + OLD_FILE_NAME, backUpPath + "/" + NEW_FILE_NAME);
    if(!readNewFileList()){
        emit sigRenameFileNameLog(QString("Please confirm whether to Edit name information !"));
        status_ = StatusType::EditNameFileError;
    }
    if(newFileAndDirNameList_.size() != oldFileAndDirNameList_.size()){
        emit sigRenameFileNameLog(QString("The number of renamed files is Not Equal !"));
        status_ = StatusType::NamesNumberError;
    }

    // 只保留不同的项
    for(int i = 0; i < oldFileAndDirNameList_.size(); ++i) {
        if(oldFileAndDirNameList_.at(i) == newFileAndDirNameList_.at(i)) {
            oldFileAndDirNameList_.removeAt(i);
            newFileAndDirNameList_.removeAt(i);
            --i;
        }
    }
    if(status_ == StatusType::OK){
        updateRenameFileList();
    }
}

bool RenameFileName::readNewFileList() {
    newFileAndDirNameList_.clear();
    QFile file(filePath_+".backup/" + NEW_FILE_NAME);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit sigRenameFileNameLog(QString(" Read [" + NEW_FILE_NAME + "] Failed!"));
        return false;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if(line.split(" |").size() < 3) {
            return false;
        }
        QString name = line.split(" |").at(1);
        QString path = line.split(" |").last();
        int pos = path.lastIndexOf("/") + 1;
        QString strName = path.mid(0, pos) + name;
        strName = strName.trimmed();
        newFileAndDirNameList_.append(strName);
    }
    file.close();
    return true;
}

void RenameFileName::renameFileListClear() {
    ui->renameFileListWgt->clear();
}

void RenameFileName::updateRenameFileList(){
    ui->renameFileListWgt->clear();
    for(int i = 0, j = 0; i < oldFileAndDirNameList_.size(); ++i){
        if(oldFileAndDirNameList_.at(i) != newFileAndDirNameList_.at(i)) {
            ui->renameFileListWgt->setRowCount(j + 1);
            QTableWidgetItem *pItem1 = new QTableWidgetItem(oldFileAndDirNameList_.at(i));
            QTableWidgetItem *pItem2 = new QTableWidgetItem(newFileAndDirNameList_.at(i));
            ui->renameFileListWgt->setItem(j, 0, pItem1);
            ui->renameFileListWgt->setItem(j, 1, pItem2);
            j++;
        }
    }
}

void RenameFileName::initRenameFileList() {
    ui->renameFileListWgt->setColumnCount(2);     //设置列数
    ui->renameFileListWgt->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置每行内容不可编辑
    ui->renameFileListWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*去掉每行的行号*/
    QHeaderView *headerView = ui->renameFileListWgt->verticalHeader();
    headerView->setHidden(true);
    ui->renameFileListWgt->horizontalHeader()->setStretchLastSection(true);
    // 消除表格控件的边框
    ui->renameFileListWgt->setFrameShape(QFrame::NoFrame);
    //设置表格不显示格子线
    ui->renameFileListWgt->setShowGrid(false); //设置不显示格子线
    // 不显示表头
    ui->renameFileListWgt->horizontalHeader()->setHidden(true);
    ui->renameFileListWgt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

bool RenameFileName::isMarkdownFile(QString fileName) {
    if(fileName.size() >= 5 && fileName.right(3) == ".md"){
        return true;
    }
    return false;
}

bool getMarkdownQString(QString Path,QString oldFileName, int &n) {
    n = 0;
    QFile data(Path);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return false;
    }
    while (!data.atEnd())
    {
        QByteArray line = data.readLine();
        QString str(line);
        n += str.count(oldFileName);
    }
    data.close();
    if(n > 0){
        return true;
    }
    return false;
}

void RenameFileName::getRefMarkdownFile(const QString& path, const QString& oldFileName)
{
    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time);
    if (!fileList.isEmpty()) {
        for(int i = fileList.size() - 1; i >= 0; --i) {
            if(isMarkdownFile(fileList.at(i).fileName())) { // 只遍历markdown文件
                int num;
                if(getMarkdownQString(fileList.at(i).absoluteFilePath(), oldFileName, num)) {
                    // 引用的文件，引用了几句话
                    refFilePathAndNum_.insert(fileList.at(i).absoluteFilePath(), num);
                    qDebug() <<  fileList.at(i).absoluteFilePath() << "   " << num;
                }
            }
        }
    }
}

void RenameFileName::getDirAllFiles(const QString &oldFileName)
{
    refFilePathAndNum_.clear();
    QDir dir(tarPath_);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list)
    {
        if(fileInfo.isDir()) //book  study  test目录
        {
            getRefMarkdownFile(fileInfo.absoluteFilePath(), oldFileName);
        }else{ // readme等仓库目录
            if(isMarkdownFile(fileInfo.fileName())) { // 只遍历markdown文件
                int num;
                if(getMarkdownQString(fileInfo.absoluteFilePath(), oldFileName, num)) {
                    refFilePathAndNum_.insert(fileInfo.absoluteFilePath(), num);
                }
            }
        }
    }
}

void RenameFileName::updateMarkdownRefFileList() {
    for(int i = 0; i < oldFileAndDirNameList_.size(); ++i) {
        QDir oldFile(filePath_+"/" + oldFileAndDirNameList_.at(i));
        getDirAllFiles(oldFile.dirName());
    }
}

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

// 修改所有文件，特殊符号替换所
void RenameFileName::on_ReplaceByFilePbn_clicked()
{
    switch (status_) {
    case StatusType::EditNameFileError:
        emit sigRenameFileNameLog(QString("Please confirm whether to Edit name information !"));
        return;
    case StatusType::NamesNumberError:
        emit sigRenameFileNameLog(QString("The number of renamed files is Not Equal !"));
        return;
    default:
        ;
    }
    bool status = true;
    for(int i = 0; i < oldFileAndDirNameList_.size(); ++i) {
        if(oldFileAndDirNameList_.at(i) != newFileAndDirNameList_.at(i)) {
            QString oldFilePath = filePath_ +"/" + oldFileAndDirNameList_.at(i);
            QString oldFileName = QDir(oldFilePath).dirName();
            QString newFilePath = filePath_ +"/" + newFileAndDirNameList_.at(i);
            updateMarkdownRefFileList();
            if(!QFile::rename(oldFilePath, newFilePath)){
                status = false;
                emit sigRenameFileNameLog(QString("Move ") + oldFileAndDirNameList_.at(i) + " Failed ！");
            }else{
                QMap<QString, int>::iterator iter = refFilePathAndNum_.begin();
                while (iter != refFilePathAndNum_.end())
                {
                    modifyRefMarkdown(iter.key(), oldFileName, newFilePath);
                    iter++;
                }
            }
        }
    }
    QMessageBox msgBox;
    msgBox.setWindowTitle("Rename Solution");
    if(status){
        emit sigRenameFileNameLog(QString("File renamed Successfully!"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.setText(QString("File renamed Successfully!"));
    }else{
        emit sigRenameFileNameLog(QString("Some files renamed Error!"));
        msgBox.setIcon(QMessageBox::Icon::Critical);
        msgBox.setText(QString("Some files renamed Error!"));
    }
    msgBox.exec();
}

void RenameFileName::on_OpenFilePathPbn_clicked()
{
    emit sigRenameFileOpenPath(filePath_);
}

