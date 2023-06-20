#include "file_operation.h"
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QtAlgorithms>
#include <QStandardPaths>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include "debug_box.h"

FileOperation::FileOperation(QObject *parent)
    : QObject{parent}
{

}

bool isEditFileDir(QString dirName){
    QStringList editFileDirList;
    editFileDirList << "-study" << "-Study" << "-book" << "-Book" << "-test";
    for(int i = 0;i < editFileDirList.size(); ++i){
        if(dirName.indexOf(editFileDirList.at(i)) == 2){
            return true;
        }
    }
    return false;
}

bool isMarkdownFile(QString fileName){
    if(fileName.size() >= 5 && fileName.right(3) == ".md"){
        return true;
    }
    return false;
}

void FileOperation::getDirAllFiles(const QString &dirPath)
{
    QDir dir(dirPath);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list)
    {
        if(fileInfo.isDir()) //book  study  test目录
        {
            fileList_.append(getLastmodifiedTimeFileName(fileInfo.absoluteFilePath()));
            //            getDirAllFiles(fileInfo.absoluteFilePath());
        }
    }
}

// 获取path目录下最新修改的文件
QFileInfo FileOperation::getLastmodifiedTimeFileName(const QString& path)
{
    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time);
    QFileInfo lastModifiedTimeFileName;
    if (fileList.isEmpty() == false)
    {
        for(int i = fileList.size()-1; i >=0; --i){
            if(isMarkdownFile(fileList.at(i).fileName())){ // 只遍历markdown文件
                lastModifiedTimeFileName = fileList.first();
                return lastModifiedTimeFileName;
            }
        }
    }
    return lastModifiedTimeFileName;
}

int FileOperation::getLastmodifiedTimeFileNum(const QString &path, QString& fullPath,QString& lastModefyFile)
{
    int num;
    QDir dir(path);

    fileList_.clear();
    getDirAllFiles(dir.absolutePath());
    qSort(fileList_.begin(), fileList_.end(),&FileOperation::sortFileByInfo);
    if(fileList_.isEmpty()){
        DebugBox(__FUNCTION__, __LINE__,"Directory: \"" + path + "\" does not exist!");
        emit sigFileOperationLog("Directory: \"" + path + "\" does not exist!");
        return -1;
    }
    lastModefyFile = fileList_.last().fileName();
    //    QStringList strList = fileList_.last().path().split(path);
    fullPath = fileList_.last().path();
    fileList_.clear();

    QStringList nameArr = lastModefyFile.split("-");
    num = nameArr.at(0).toInt();

    return num;
}

int FileOperation::getLastmodifiedTimeFileNumSubDir(const QString &path,const QString &dirName, QString& fullPath,QString& lastModefyFile)
{
    int num;
    QDir dir(path+"/"+dirName);
    QStringList filters;
    filters << "*.md"<< "\?\?-*";
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries, QDir::Time);
    if(list.isEmpty()){
        DebugBox(__FUNCTION__, __LINE__,"Directory: \"" + path + dirName + "\" does not exist markdown files!");
        emit sigFileOperationLog("Directory: \"" + path +dirName + "\" does not exist markdown files!");
        return -1;
    }
    lastModefyFile = list.first().fileName();
    qDebug()<<lastModefyFile;
    fullPath = list.first().path();
    qDebug()<<fullPath;
    QStringList nameArr = lastModefyFile.split("-");
    num = nameArr.at(0).toInt();
    return num;
}

QString getNewFileName(QString fullTarPath, ImgData data, int fileNum){  // 01-45.png  --> fileNum-AssetNum.png
    QDir newDir(fullTarPath);
    // 进入Assets目录
    if(data.oldFileInfo.suffix() == "mp4"){
        if(!newDir.cd("video")){
            DebugBox(__FUNCTION__, __LINE__,newDir.absolutePath() + " don't have video directory!");
            return QString("");
        }
    }else{
        if(!newDir.cd("img")){
            DebugBox(__FUNCTION__, __LINE__,newDir.absolutePath() + " don't have img directory!");
            return QString("");
        }
    }

    // 获取 new fileName
    int AssetNum = 1;
    QString newNamePri = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"-"+QString::number(AssetNum);
    QString newName = newNamePri+"." + data.oldFileInfo.suffix();
    QString newFilePath = newDir.absolutePath() + "/" + newName;
    while(newDir.exists(newFilePath)){
        AssetNum++;
        newNamePri = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"-"+QString::number(AssetNum);
        newName = newNamePri +"." + data.oldFileInfo.suffix();
        newFilePath = newDir.absolutePath() + "/" + newName;
    }
    return newFilePath;
}

QString clipMarkdownCodeItem(QString newAssetsPath){
    QString clipTextItem;
    QFileInfo fileInfo(newAssetsPath);
    if(fileInfo.suffix() == "mp4"){
        clipTextItem = "<video src=./video/"+ fileInfo.fileName() +" alt=" + fileInfo.baseName() + " width=50%;/>";
    }else{
        clipTextItem = "<img src=./img/"+ fileInfo.fileName() +" alt=" + fileInfo.baseName() + " style=zoom:50%;/>";
    }
    return clipTextItem;
}

// 将fileInfoVec里面的所有文件的，放到相应目录
bool FileOperation::clipFilesByFileInfo(const QVector<ImgData> &fileInfoVec, QString fullTarPath,int fileNum, QString &clipText)
{
    for(int i = 0; i < fileInfoVec.size(); ++i){
        ImgData data = fileInfoVec.at(i);
        QDir dir(data.oldPath);
        QString newAssetsPath = getNewFileName(fullTarPath, data, fileNum);
        // clip file
        if(!dir.rename(data.oldPath, newAssetsPath)){
            if(newAssetsPath.right(3) == "mp4"){
                emit sigFileOperationLog( "视频文件可能在占用 !");
            }
            DebugBox(__FUNCTION__, __LINE__,"move file error");
            clipText = QString("<center>    \n")+clipText.chopped(2)+QString("\n</center>    \n");
            return false;
        }
        emit sigFileOperationLog( "Clip From" + data.oldPath + " \nTo " + newAssetsPath);
        // Clip Markdown Code
        clipText +=  clipMarkdownCodeItem(newAssetsPath) + QString("    \n");
    }
    clipText = QString("<center>    \n")+clipText.chopped(2)+QString("\n</center>    \n");
    return true;
}

bool FileOperation::getFileNameByNum(QString fullPath, int fileNum, QString& fileName){
    QDir dir(fullPath);
    QStringList filters;
    QString nameNum = QString("%1").arg(fileNum, 2, 10, QLatin1Char('0'))+"*";
    filters << nameNum;
    dir.setFilter(QDir::AllEntries);
    dir.setNameFilters(filters);
    QFileInfoList fileInfoList = dir.entryInfoList();
    if(fileInfoList.size() > 1) {
        QString msg;
        msg = QString("包含") + QString::number(fileInfoList.size())+ QString("个序号为： \"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\" 的markdown文件：\n");
        for(auto it = fileInfoList.begin(); it < fileInfoList.end(); it++){
            msg += it->fileName() + ", ";
        }
        msg[msg.size()-2] = '!';
        emit sigFileOperationLog(msg);
    }else if(fileInfoList.size() < 1){
        QString msg;
        msg = QString("序号为：\"") + QString("%1").arg(fileNum, 2, 10, QLatin1Char('0')) + QString("\"的文件or目录不存在！").toUtf8();
        emit sigFileOperationLog(msg);
        return false;
    }

    fileName = fileInfoList.last().fileName();
    //    emit sigFileOperationLog(QString("当前文档/目录为：")+fileName);
    return true;
}

QStringList FileOperation::getSubDirNames(QString TarPath){
    QStringList dirList;
    QDir Root(TarPath);
    QFileInfoList fileList = Root.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs, QDir::Time);
    for(auto it = fileList.begin(); it < fileList.end(); ++it){
        dirList <<it->fileName();
    }

    return dirList;
}

bool FileOperation::isPathExist(const QString &path){
    QDir dir;
    return dir.exists(path);
}

void FileOperation::setAssetsTypes(QStringList types)
{
    assetTypes_.swap(types);
}

// 如果有重名，就在后面加上-num
QString FileOperation::newNumFileName(QString path, QString fileName, QString suffix)
{
    int AssetNum = 0;
    QString tmpName =fileName;
    QDir newDir(path);
    while(newDir.exists(path+"/"+tmpName+"."+suffix)){
        AssetNum++;
        tmpName = fileName;
        tmpName += "-" + QString("%1").arg(AssetNum, 2, 10, QLatin1Char('0'));
    }
    return tmpName;
}

bool FileOperation::copyImgVideo(QDir& CurrentPath, const QStringList& fileNameArr, const QString & name, QString &searchResult)
{
    QDir tmpCurPath = CurrentPath;
    for(int i = 0; i< fileNameArr.size(); ++i){
        CurrentPath = tmpCurPath;
        QString fileName = fileNameArr.at(i);
        if(fileName.right(3) != "mp4"){
            if(!CurrentPath.cd("img")){
                emit sigFileOperationLog(CurrentPath.absolutePath() + QString(" don't have img directory!"));
                return false;
            }
        }else{
            if(!CurrentPath.cd("video")){
                emit sigFileOperationLog(CurrentPath.absolutePath() + QString(" don't have video directory!"));
                return false;
            }
        }
        searchResult =CurrentPath.absolutePath()+"/" + fileName;
        QString suffix =fileName.split(".").last();
        QString desPath;
        if(name.isEmpty()){
            QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss.zzz");
            desPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/99-" + time + "-" + fileName;

        }else{
            QString newName = newNumFileName(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), name, suffix);
            desPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + newName+"."+ suffix;
        }
        if(!QFile::copy(searchResult, desPath))
        {
            emit sigFileOperationLog(searchResult + QString(" copy failed!"));
            continue;
        }
        emit sigFileOperationLog(QString("Copy: "+searchResult+"\nTo "+ desPath + "\nCopy Success !!!"));
    }
    return true;
}

bool FileOperation::getSearchResultFromMarkdownCode(const QString &currentFullPath, const QString &codeText, const QString& name, QString &searchResult)
{
    if(currentFullPath.isEmpty()){
        emit sigFileOperationLog("Current FullPath is empty!");
        return false;
    }
    bool hasImgVideo = false;
    QDir CurrentPath(currentFullPath);
    QStringList codeList = codeText.split(" ");
    QStringList fileNameArr;
    for(int i = 0; i < codeList.size(); ++i){
        for(int j = 0; j < assetTypes_.size(); ++j){
            if(codeList.at(i).size()>4 && codeList.at(i).right(3) == assetTypes_.at(j).right(3)){
                QStringList pathList = codeList.at(i).split("/");
                fileNameArr.push_back(pathList.last());
                hasImgVideo = true;
                break;
            }
        }
    }
    if(!hasImgVideo){
        emit sigFileOperationLog(QString("markdown code don't have img/video file!"));
        return false;
    }

    return copyImgVideo(CurrentPath, fileNameArr,name , searchResult);
}

// -------------------------- get file List of last two weeks 一共  15 files
// 获取path目录下最新修改的文件
const int topFileCount = 20;
void FileOperation::getLastmodifiedTop20Files(const QString& path, QFileInfoList& fileListTemp)
{
    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time);
    QFileInfo lastModifiedTimeFileName;
    if (!fileList.isEmpty())
    {
        for(int i = 0; i < fileList.size() && i < topFileCount; ++i){
            if(isMarkdownFile(fileList.at(i).fileName())){ // 只遍历markdown文件
                fileListTemp.append(fileList.at(i));
            }
        }
    }
}

//bool FileOperation::compareTimeData(const QFileInfo &stu1, const QFileInfo &stu2)
//{
//    return stu1.lastModified() > stu2.lastModified();
//}

bool compare(const QFileInfo &file1, const QFileInfo &file2){
    return file1.lastModified().toTime_t() > file2.lastModified().toTime_t();
}

void FileOperation::getHistoryFileList(const QString &dirPath, QFileInfoList& fileListTop20){
    QDir dir(dirPath);
    QFileInfoList resultFileList;
    QFileInfoList fileListTemp;
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list)
    {
        if(fileInfo.isDir()) //book  study  test目录
        {
            fileListTemp.clear();
            getLastmodifiedTop20Files(fileInfo.absoluteFilePath(),fileListTemp);
            for(int i = 0; i < fileListTemp.size(); ++i){
                resultFileList.append(fileListTemp.at(i));
            }
        }
    }
    qSort(resultFileList.begin(),resultFileList.end(),compare);
    for(int i = 0; i < resultFileList.size() && i < topFileCount; ++i){
        fileListTop20.append(resultFileList.at(i));
    }
}

bool FileOperation::createMarkdownFile(const QString& FullPath, QString& currentFileName){
    QDir dir(FullPath);
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
                templateFileName =list.at(i).fileName();
            }
        }
    }
    int num = max+1;
    QString fileName = QString("%1").arg(num, 2, 10, QLatin1Char('0')) + "-新建文件.md";
    QString path =FullPath + "/" + fileName;

    if(templateFileName.isEmpty()){
        QString templateFile = FullPath + "/" + templateFileName;
        if(!QFile::copy(templateFile, path))
        {
            emit sigFileOperationLog(templateFile + QString(" copy failed!"));
        }
        emit sigFileOperationLog(QString("Copy: "+templateFile+"\nTo Create:"+ path + "\nCreate File Success  !!!"));
        currentFileName = fileName;
    }else{
        QFile file(path);
        if (file.exists()) {
            emit sigFileOperationLog(QString("新建文件已存在！"));
            return false;
        }
        file.open(QIODevice::WriteOnly);
        QString text = "# [新建文件](./)  [img](./img)   \n" \
                "\n" \
                "> ######  _标签:_   ![](https://img.shields.io/badge/技术类-yellowgreen.svg)   ![ ](https://img.shields.io/badge/Protobuf-编译和使用-blue.svg)    [![](https://img.shields.io/badge/链接-github仓库-brightgreen.svg)](https://github.com/protocolbuffers/protobuf#protocol-compiler-installation)    [![](https://img.shields.io/badge/链接-代码文件-orange.svg)](../02-code/)    [![](https://img.shields.io/badge/链接-本地仓库-orange.svg)](../04-repo/)    [![](https://img.shields.io/badge/链接-数据文件-orange.svg)](../03-data/)  \n"   \
                ">  \n\n\n";
        QByteArray str = text.toUtf8();
        file.write(str);
        emit sigFileOperationLog(QString("Create File") + path + "\nCreate File Success");
        currentFileName = fileName;
        file.close();
    }
    return true;
}

