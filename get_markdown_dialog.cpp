#include "get_markdown_dialog.h"
#include "ui_get_markdown_dialog.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>

GetMarkdownDialog::GetMarkdownDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::get_markdown_dialog)
{
    ui->setupUi(this);
}

GetMarkdownDialog::~GetMarkdownDialog()
{
    delete ui;
}

void GetMarkdownDialog::numOldMarkdownSpinBoxStatus(int flags) {
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

void GetMarkdownDialog::numNewMarkdownSpinBoxStatus(int flags) {
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

void GetMarkdownDialog::initSize() {
    double widthIn4K = 880;
    double heightIn4K = 370;
    double zoom = 1;
    if(width_ < 3840) {
        zoom = 1.2;
    }
    // 830 400
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

QString GetMarkdownDialog::exportPathSimplify(QString exportPath){
    QDir repoDir(repoPath_);
    if(exportPath.startsWith(repoPath_)) {
        QString relativePath = repoDir.relativeFilePath(exportPath);
        return relativePath;
    }
    return exportPath;
}

QString GetMarkdownDialog::getExportFullPath(QString text){
    QDir repoDir(repoPath_);
    if(text.size() >= 2 && text.mid(1,2) != ":") {
        return repoDir.absoluteFilePath(text);
    }
    return text;
}

void GetMarkdownDialog::showWindow(QString repoPath, QString subDirPath, int width, int fileNum) {
    initGetMarkdownDlg(repoPath, subDirPath, width, fileNum);
    QDir newDir(repoPath_);
    ui->newFileNameEdit->setText("");
    newDir.mkpath(repoPath_ + "/09-export/00-default");

    ui->newFilePathEdit->setText(exportPathSimplify(repoPath_ + "/09-export/00-default"));

    updateMarkdownNum();
    initSize();
    ui->numOldMarkdownSpinBox->setValue(currentFileNum_);
    show();
}

bool GetMarkdownDialog::isMarkdownFile(QString fileName){
    if(fileName.size() >= 5 && fileName.right(3) == ".md"){
        return true;
    }
    return false;
}

void GetMarkdownDialog::updateMarkdownNum()
{
    QDir dir(getExportFullPath(ui->newFilePathEdit->text()));
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
    if(max == -1) {
        max++;
    }
    ui->numNewMarkdownSpinBox->setValue(max + 1);
}

void GetMarkdownDialog::on_numNewMarkdownSpinBox_valueChanged(int fileNum)
{
    QDir dir(getExportFullPath( ui->newFilePathEdit->text()));
    QStringList filters;
    QString nameNum = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"*";
    filters << nameNum;
    dir.setFilter(QDir::AllEntries);
    dir.setNameFilters(filters);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(fileInfoList.size() > 0) {
        // 有编号为fileNum的文件
        QString msg;
        msg = QString("序号为：\"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\"的文件被占用 !").toUtf8();
        qDebug() << msg;
        numNewMarkdownSpinBoxStatus(0);
        return;
    }
    numNewMarkdownSpinBoxStatus(1);
}


void GetMarkdownDialog::on_openTempPbn_clicked()
{
    if(ui->newFileNameEdit->isEnabled()) {
        QString tempFileName = QString("%1").arg(ui->numOldMarkdownSpinBox->value(), 2, 10,
                                                 QLatin1Char('0')) + "-" + ui->templeFileNameEdit->text();
        QString templateFileAbs = subDirPath_ + "/" + tempFileName;
        emit sigOpenTempleMarkdown(templateFileAbs);
    }else{
        emit sigGetMarkdownFileLog(QString("文件名字为空无法打开!"));
    }
}

bool GetMarkdownDialog::copyMarkdownFile(QString oldfileAbs, QString newFileDir, QString fileName) {
    QString newFilePath = newFileDir + "/" + fileName;
    QFile file(oldfileAbs);
    if (!file.exists()) {
        return false;
    }
    if (!QDir(newFileDir).exists()) {
        QDir().mkdir(newFileDir);
    }
    if (!file.copy(newFilePath)) {
        return false;
    }

    return true;
}

void GetMarkdownDialog::on_yesPbn_clicked()
{
    if(oldFileName_.isEmpty() || ui->newFileNameEdit->text().isEmpty() ||
            subDirPath_.isEmpty()) {
        return ;
    }
    QString fileName = QString("%1").arg(ui->numNewMarkdownSpinBox->value(), 2, 10,
                                         QLatin1Char('0')) + "-" + ui->newFileNameEdit->text() + ".md";
    if(ui->newFileNameEdit->isEnabled()) {
        if(copyMarkdownFile(subDirPath_ + "/" + oldFileName_, getExportFullPath(ui->newFilePathEdit->text()), fileName)){
            emit sigCopyAssetsToExportDir(subDirPath_ + "/" + oldFileName_, getExportFullPath(ui->newFilePathEdit->text()), fileName);
        }
    }else{
        emit sigGetMarkdownFileLog(QString("取消导出!"));
    }

    this->close();
}

void GetMarkdownDialog::initGetMarkdownDlg(QString repoPath, QString subDirPath, int width, int fileNum)
{
    repoPath_ = repoPath;
    subDirPath_ = subDirPath;
    if(subDirPath_.contains("-export")) {
        ui->templeFileNameEdit->setText("不能从导出export目录导出!");
        oldFileName_ = "";
        numOldMarkdownSpinBoxStatus(0);
    }
    width_= width;
    currentFileNum_ = fileNum;
}

void GetMarkdownDialog::on_numOldMarkdownSpinBox_valueChanged(int fileNum)
{
    if(subDirPath_.contains("-export")) {
        ui->templeFileNameEdit->setText("不能从导出export目录导出!");
        oldFileName_ = "";
        numOldMarkdownSpinBoxStatus(0);
        return ;
    }
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
            ui->templeFileNameEdit->setText("没有文件!");
            oldFileName_ = "";
            numOldMarkdownSpinBoxStatus(0);
        } else {
            numOldMarkdownSpinBoxStatus(1);
            ui->numOldMarkdownSpinBox->setValue(0);
        }
        return;
    }

    fileName = fileInfoList.last().fileName();
    oldFileName_ = fileName;
    ui->templeFileNameEdit->setText(fileName.split(QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"-").last());
    numOldMarkdownSpinBoxStatus(1);
}

void GetMarkdownDialog::on_fileDirPbn_clicked()
{
    QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString path = QFileDialog::getExistingDirectory(this,"选择仓库目录", desktop,QFileDialog::ShowDirsOnly);
    if(path.isEmpty()) {
        emit sigGetMarkdownFileLog("路径为空，无法添加!");
        ui->newFilePathEdit->setText("");
        return ;
    }
    ui->newFilePathEdit->setText(exportPathSimplify(path));
}


void GetMarkdownDialog::on_cancelPbn_clicked()
{
    emit sigGetMarkdownFileLog(QString("取消创建!"));
    this->close();
}


void GetMarkdownDialog::on_newFilePathEdit_textChanged(const QString &arg1)
{
    updateMarkdownNum();
}

