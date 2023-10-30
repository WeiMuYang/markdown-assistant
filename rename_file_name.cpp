#include "rename_file_name.h"
#include "ui_rename_file_name.h"
#include <QDebug>
#include <QMessageBox>

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
    ui->PathEdit->setText(tarPath_);
}

RenameFileName::~RenameFileName()
{
    delete ui;
}

void RenameFileName::on_ChoosePbn_clicked()
{
    tarPath_ = QFileDialog::getExistingDirectory(this,"选择目录",tarPath_,QFileDialog::ShowDirsOnly);
}

// TODO:
void RenameFileName::on_ReplaceByListPbn_clicked()
{

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
                if(fileDir.isEmpty()){
                    oldFileAndDirNameList_.append(filePath);
                }
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
        emit sigRenameFileNameLog(QString(" Complete data backup Failed"));
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
            QString txt = QString("%1").arg(i + 1, 2, 10, QLatin1Char('0')) + QString(" |") + oldFileAndDirNameList_.at(i);
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
        sigRenameFileNameLog(QString(" The number of renamed files is Not Equal !"));
        status_ = StatusType::NamesNumberError;
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
        newFileAndDirNameList_.append(line.split(" |").last());
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
            ui->renameFileListWgt->setRowCount(j + 1);        //设置行数/
            QTableWidgetItem *pItem1 = new QTableWidgetItem(oldFileAndDirNameList_.at(i));
            QTableWidgetItem *pItem2 = new QTableWidgetItem(newFileAndDirNameList_.at(i));
            ui->renameFileListWgt->setItem(j, 0, pItem1);
            ui->renameFileListWgt->setItem(j, 1, pItem2);
            j++;
            //        QTableWidgetItem *pItem0 = new QTableWidgetItem(i+1);
            //        pItem0->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            //        ui->renameFileListWgt->setVerticalHeaderItem(i, pItem0);
        }
    }
}

void RenameFileName::initRenameFileList() {
    ui->renameFileListWgt->setColumnCount(2);     //设置列数
    ui->renameFileListWgt->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置每行内容不可编辑
    ui->renameFileListWgt->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*去掉每行的行号*/
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

void RenameFileName::on_ReplaceByFilePbn_clicked()
{
    switch (status_) {
    case StatusType::EditNameFileError:
        emit sigRenameFileNameLog(QString(" Please confirm whether to Edit name information !"));
        return;
    case StatusType::NamesNumberError:
        emit sigRenameFileNameLog(QString(" The number of renamed files is Not Equal !"));
        return;
    default:
        ;
    }
    bool status = true;
    for(int i = 0; i < oldFileAndDirNameList_.size(); ++i) {
        if(oldFileAndDirNameList_.at(i) != newFileAndDirNameList_.at(i)) {
            QString oldFilePath = filePath_ +"/" + oldFileAndDirNameList_.at(i);
            QString newFilePath = filePath_ +"/" + newFileAndDirNameList_.at(i);
            if(!QFile::rename(oldFilePath,newFilePath)){
                status = false;
                emit sigRenameFileNameLog(QString(" Move ") + oldFileAndDirNameList_.at(i) + " Failed ！");
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

