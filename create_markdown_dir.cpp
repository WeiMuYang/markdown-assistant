#include "create_markdown_dir.h"
#include "ui_create_markdown_dir.h"
#include <QDir>
#include <QDebug>

CreateMarkdownAndSubDir::CreateMarkdownAndSubDir(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateMarkdown)
{
    ui->setupUi(this);
}

CreateMarkdownAndSubDir::~CreateMarkdownAndSubDir()
{
    delete ui;
}

bool CreateMarkdownAndSubDir::isMarkdownFile(QString fileName){
    if(fileName.size() >= 5 && fileName.right(3) == ".md"){
        return true;
    }
    return false;
}

void CreateMarkdownAndSubDir::updateMarkdownWgt()
{
    QDir dir(subDirPath_);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    int max = -1;
    QString templateFileName;
    for(int i = 0; i < list.size(); ++i){
        if(isMarkdownFile(list.at(i).fileName())){
            QStringList nameArr = list.at(i).fileName().split("-");
            int num = nameArr.at(0).toInt();
            if(num > max){
                max = num;
            }
            if(num == 0 && list.at(i).fileName().right(5) != "um.md"){
                templateFileName = list.at(i).fileName();
            }
        }
    }
    ui->numOldMarkdownSpinBox->setValue(max);
    ui->numNewMarkdownSpinBox->setValue(max + 1);
}

void CreateMarkdownAndSubDir::updateSubDirWgt()
{
    QDir dir(repoPath_);
    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    QVector<int> numArr;
    for(int i = 0; i < list.size(); ++i){
        if(list.at(i).isDir()){
            QStringList nameArr = list.at(i).fileName().split("-");
            numArr.append(nameArr.at(0).toInt());
        }
    }
    int num = -1;
    std::sort(numArr.begin(), numArr.end());
    for(int i = 0; i < numArr.size() - 1; ++i) {
        if(numArr.at(i+1) - numArr.at(i) > 1  ) {
            num = numArr.at(i) + 1;
            break;
        }
    }
    if(-1 == num) {
        num = numArr.last() + 1;
    }
    ui->numSubDirSpinBox->setValue(num);
}


void CreateMarkdownAndSubDir::showWindow() {
    ui->stackedWidget->setCurrentIndex(0);
    ui->fileRadioBtn->setChecked(true);
    ui->newDirFileNameEdit->setText("");
    ui->newFileNameEdit->setText("");
    updateMarkdownWgt();
    updateSubDirWgt();
    show();
}


void CreateMarkdownAndSubDir::on_fileRadioBtn_clicked(bool checked)
{
    if(checked) {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void CreateMarkdownAndSubDir::on_dirRadioBtn_clicked(bool checked)
{
    if(checked) {
        ui->stackedWidget->setCurrentIndex(1);
    }
}


void CreateMarkdownAndSubDir::numOldMarkdownSpinBoxStatus(int flags) {
    if(flags == 1){
        ui->numOldMarkdownSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}");
        ui->templeFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}");
        ui->newFileNameEdit->setEnabled(true);
    }else if(flags == 2) {
        ui->numOldMarkdownSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}\nQSpinBox{\nborder: 1px solid #ffff00\n}\n");
        ui->templeFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}\nQLineEdit{\nborder: 1px solid #ffff00\n}\n");
        ui->newFileNameEdit->setEnabled(true);
    }else{
        ui->numOldMarkdownSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}\nQSpinBox{\nborder: 1px solid #ff0000\n}\n");
        ui->templeFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}\nQLineEdit{\nborder: 1px solid #ff0000\n}\n");
        ui->newFileNameEdit->setEnabled(false);
    }
}

void CreateMarkdownAndSubDir::numNewMarkdownSpinBoxStatus(int flags) {
    if(flags == 1){
        ui->numNewMarkdownSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}");
        ui->newFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}");
        ui->newFileNameEdit->setEnabled(true);
    }else{
        ui->numNewMarkdownSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}\nQSpinBox{\nborder: 1px solid #ff0000\n}\n");
        ui->newFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}\nQLineEdit{\nborder: 1px solid #ff0000\n}\n");
        ui->newFileNameEdit->setEnabled(false);
    }
}

void CreateMarkdownAndSubDir::numDirSpinBoxStatus(int flags) {
    if(flags == 1){
        ui->numSubDirSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}");
        ui->newDirFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}");
        ui->newDirFileNameEdit->setEnabled(true);
    }else{
        ui->numSubDirSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}\nQSpinBox{\nborder: 1px solid #ff0000\n}\n");
        ui->newDirFileNameEdit->setStyleSheet("QLineEdit::up-button,QLineEdit::down-button\n{\n	width:0px;\n}\nQLineEdit{\nborder: 1px solid #ff0000\n}\n");
        ui->newDirFileNameEdit->setEnabled(false);
    }
}

void CreateMarkdownAndSubDir::on_numOldMarkdownSpinBox_valueChanged(int fileNum)
{
    QDir dir(subDirPath_);
    QString fileName;
    QStringList filters;
    QString nameNum = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"*";
    filters << nameNum;
    dir.setFilter(QDir::AllEntries);
    dir.setNameFilters(filters);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(fileInfoList.size() > 1) {
        // 编号为fileNum的文件有多个
        QString msg;
        msg = QString("包含") + QString::number(fileInfoList.size())+ QString("个序号为： \"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\" 的markdown文件：\n");
        for(auto it = fileInfoList.begin(); it < fileInfoList.end(); it++){
            msg += it->fileName() + ", ";
        }
        msg[msg.size()-2] = '!';
        qDebug() << msg;
        numOldMarkdownSpinBoxStatus(2);
    }else if(fileInfoList.size() < 1){
        // 编号为fileNum的文件没有
        QString msg;
        msg = QString("序号为：\"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\"的文件or目录不存在！").toUtf8();
        if(fileNum == 0) {
            ui->templeFileNameEdit->setText("没有模板文件!");
            numOldMarkdownSpinBoxStatus(0);
        } else {
            numOldMarkdownSpinBoxStatus(1);
            ui->numOldMarkdownSpinBox->setValue(0);
        }
        return;
    }

    fileName = fileInfoList.last().fileName();
    ui->templeFileNameEdit->setText(fileName.split(QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"-").last());
    numOldMarkdownSpinBoxStatus(1);
}


void CreateMarkdownAndSubDir::on_numSubDirSpinBox_valueChanged(int fileNum)
{
    QDir dir(repoPath_);
    QStringList filters;
    QString nameNum = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"*";
    filters << nameNum;
    dir.setFilter(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    dir.setNameFilters(filters);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(fileInfoList.size() > 0) {
        QString msg;
        msg = QString("序号为：\"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\"的目录被占用！").toUtf8();
        qDebug() << msg;
        numDirSpinBoxStatus(0);
        ui->oldDirFileNameEdit->setText(fileInfoList.first().fileName());
        return;
    }
    ui->oldDirFileNameEdit->setText("编号没有被占用。");
    numDirSpinBoxStatus(1);
}

void CreateMarkdownAndSubDir::on_numNewMarkdownSpinBox_valueChanged(int fileNum)
{
    QDir dir(subDirPath_);
    QStringList filters;
    QString nameNum = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"*";
    filters << nameNum;
    dir.setFilter(QDir::AllEntries);
    dir.setNameFilters(filters);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(fileInfoList.size() > 0) {
        // 有编号为fileNum的文件
        QString msg;
        msg = QString("序号为：\"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\"的文件被占用！").toUtf8();
        qDebug() << msg;
        numNewMarkdownSpinBoxStatus(0);
        return;
    }
    numNewMarkdownSpinBoxStatus(1);
}

void CreateMarkdownAndSubDir::on_yesPbn_clicked()
{
    if(ui->fileRadioBtn->isChecked() && ui->newFileNameEdit->isEnabled()) {
        QString newFilePathAbs;
        if(createMarkdown(newFilePathAbs)){
            emit sigCreateType(1, newFilePathAbs);
        }
    }else if(ui->dirRadioBtn->isChecked() && ui->newDirFileNameEdit->isEnabled()){
        QString newSunDir;
        if(createSubDir(newSunDir)) {
            emit sigCreateType(2, newSunDir);
        }
    }else{
        emit sigCreateMarkdownAndDirLog(QString("取消创建!"));
    }

    this->close();
}

bool CreateMarkdownAndSubDir::createMarkdown(QString& path) {
    QString tempFileName = QString("%1").arg(ui->numOldMarkdownSpinBox->value(), 2, 10,
                                             QLatin1Char('0')) + "-" + ui->templeFileNameEdit->text();
    QString fileName = QString("%1").arg(ui->numNewMarkdownSpinBox->value(), 2, 10,
                                         QLatin1Char('0')) + "-" + ui->newFileNameEdit->text() + ".md";
    QString newFilePathAbs = subDirPath_ + "/" + fileName;
    path = newFilePathAbs;
    if(!tempFileName.isEmpty()){
        QString templateFileAbs = subDirPath_ + "/" + tempFileName;
        if(!QFile::copy(templateFileAbs, newFilePathAbs))
        {
            emit sigCreateMarkdownAndDirLog(templateFileAbs + QString(" copy failed!"));
        }
        emit sigCreateMarkdownAndDirLog(QString("Copy: "+templateFileAbs+"\nTo Create:"+ newFilePathAbs + "\nCreate File Success  !!!"));
        // 将拷贝的文件追加回车，成为最近修改文件
        QFile file(newFilePathAbs);
        file.open(QIODevice::ReadWrite | QIODevice::Append);
        QTextStream txtOutput(&file);
        txtOutput << "\n";
        file.close();
        return true;
    }

    return false;
}

bool CreateMarkdownAndSubDir::createSubDir(QString& path)
{
    QString dirName = QString("%1").arg(ui->numSubDirSpinBox->value(), 2, 10,
                                        QLatin1Char('0')) + "-" + ui->newDirFileNameEdit->text();
    QString newDirPathAbs = repoPath_ + "/" + dirName;
    path = newDirPathAbs;
    QDir newDir(repoPath_);
    if(newDir.mkpath(newDirPathAbs)) {
        emit sigCreateMarkdownAndDirLog(newDirPathAbs + QString(" Create Success!"));
        QString path = newDirPathAbs + "/00-模板文件.md";
        QFile file(path);
        if (file.exists()) {
            emit sigCreateMarkdownAndDirLog(QString("新建文件已存在！"));
            return false;
        }
        file.open(QIODevice::WriteOnly);
        QString text = "# [新建文件](./)  [img](./img)   \n" \
                       "\n" \
            "> ######  _标签:_   ![](https://img.shields.io/badge/技术类-yellowgreen.svg)   ![ ](https://img.shields.io/badge/Protobuf-编译和使用-blue.svg)    [![](https://img.shields.io/badge/链接-github仓库-brightgreen.svg)](https://github.com/protocolbuffers/protobuf#protocol-compiler-installation)    [![](https://img.shields.io/badge/链接-代码文件-orange.svg)](../02-code/)    [![](https://img.shields.io/badge/链接-本地仓库-orange.svg)](../04-repo/)    [![](https://img.shields.io/badge/链接-数据文件-orange.svg)](../03-data/)  \n"   \
                       ">  \n\n\n";
        QByteArray str = text.toUtf8();
        file.write(str);
        emit sigCreateMarkdownAndDirLog(QString("Create File") + path + "\nCreate File Success");
        file.close();
    }else {
        emit sigCreateMarkdownAndDirLog(newDirPathAbs + QString(" Create failed!"));
        return false;
    }
    return true;
}

void CreateMarkdownAndSubDir::on_cancelPbn_clicked()
{
    emit sigCreateMarkdownAndDirLog(QString("取消创建!"));
    this->close();
}

void CreateMarkdownAndSubDir::on_openTempPbn_clicked()
{
    if(ui->fileRadioBtn->isChecked() && ui->newFileNameEdit->isEnabled()) {
        QString tempFileName = QString("%1").arg(ui->numOldMarkdownSpinBox->value(), 2, 10,
                                                 QLatin1Char('0')) + "-" + ui->templeFileNameEdit->text();
        QString templateFileAbs = subDirPath_ + "/" + tempFileName;
        emit sigOpenTempleMarkdown(templateFileAbs);
    }else{
        emit sigCreateMarkdownAndDirLog(QString("参考文件名字为空无法打开!"));
    }
}
