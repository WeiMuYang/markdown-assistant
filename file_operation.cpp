#include "file_operation.h"
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include <QtAlgorithms>
#include <QStandardPaths>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QTextStream>
#include <QDesktopServices>
#include "debug_box.h"

FileOperation::FileOperation(QObject *parent)
    : QObject{parent}
{

}
bool isNumFile(QString nameNum);

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
//        DebugBox(__FUNCTION__, __LINE__,"Directory: \"" + path + "\" does not exist!");
        emit sigFileOperationLog("\"" + path + "\" does not has sub Directory!");
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
        emit sigFileOperationLog("Directory: \"" + path +dirName + "\" does not exist files!");
        return -1;
    }
    lastModefyFile = list.first().fileName();
    //    qDebug()<<lastModefyFile;
    fullPath = list.first().path();
    //    qDebug()<<fullPath;
    QStringList nameArr = lastModefyFile.split("-");
    num = nameArr.at(0).toInt();
    return num;
}

QString getNewFileName(QString fullTarPath, ImgData data, int fileNum){  // 01-45.png  --> fileNum-AssetNum.png
    QDir newDir(fullTarPath);
    // 进入Assets目录
    if(data.oldFileInfo.suffix() == "mp4"){
        newDir.mkpath(fullTarPath + "/video");
        if(!newDir.cd("video")){
            DebugBox(__FUNCTION__, __LINE__,newDir.absolutePath() + " can't create video directory!");
            return QString("");
        }
    }else if(data.oldFileInfo.suffix() == "mp3"){
        newDir.mkpath(fullTarPath + "/audio");
        if(!newDir.cd("audio")){
            DebugBox(__FUNCTION__, __LINE__,newDir.absolutePath() + " can't create audio directory!");
            return QString("");
        }
    }else{
        newDir.mkpath(fullTarPath + "/img");
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

QString clipMarkdownCodeItem(QString newAssetsPath, int zoomWidth){
    QString clipTextItem;
    QFileInfo fileInfo(newAssetsPath);
    if(fileInfo.suffix() == "mp4"){
        clipTextItem = "<video src=./video/"+ fileInfo.fileName() +" alt=video-" + fileInfo.baseName() + " width=" + QString::number(zoomWidth) + "%;/>";
    }else if(fileInfo.suffix() == "mp3") {
        if(zoomWidth == AssetsZoomSize::Size5) {
            clipTextItem = "<audio src=./audio/"+ fileInfo.fileName() +" alt=audio-" + fileInfo.baseName() + " style=\"width:120px; height: 14px\" /></audio>";
        }else{
            clipTextItem = "<audio src=./audio/"+ fileInfo.fileName() +" alt=audio-" + fileInfo.baseName() + " style=width:" + QString::number(zoomWidth) + "%; />";
        }
    }else{
        clipTextItem = "<img src=./img/"+ fileInfo.fileName() +" alt=img-" + fileInfo.baseName() + " width=" + QString::number(zoomWidth) + "%;/>";
    }
    return clipTextItem;
}

// 将fileInfoVec里面的所有文件的，放到相应目录
bool FileOperation::clipFilesByFileInfo(const QStringList addList, QVector<ImgData> fileInfoVec, QString fullTarPath,int fileNum, QString &clipText)
{
    QString ImgClipText;
    QString VideoClipText;
    QString AudioClipText;
    for(int n = 0; n < addList.size(); n++){
        for(int i = 0; i < fileInfoVec.size(); ++i){
            ImgData data = fileInfoVec.at(i);
            if(addList.at(n) == data.oldName){
                QDir dir(data.oldPath);
                QString newAssetsPath = getNewFileName(fullTarPath, data, fileNum);
                // clip file
                if(!dir.rename(data.oldPath, newAssetsPath)){
                    if(newAssetsPath.right(4) == ".mp4"){
                        emit sigFileOperationLog( "视频文件可能在占用 !");
                    }
                    DebugBox(__FUNCTION__, __LINE__,"move file error");
                }
                // 1. 组成MP4的Markdown代码
                if(newAssetsPath.right(4) == ".mp4"){
                    VideoClipText += QString("<center>    \n") + clipMarkdownCodeItem(newAssetsPath, data.widthZoom) + QString("\n</center>    \n    \n    \n");
                }else if(newAssetsPath.right(4) == ".mp3") {
                    if(data.widthZoom == AssetsZoomSize::Size5) {
                        AudioClipText +="<p>文本&nbsp; " + clipMarkdownCodeItem(newAssetsPath, data.widthZoom)
                                         + " &nbsp;文本</p>" + QString("    \n");
                    }else{
                        AudioClipText += QString("<center>    \n") + clipMarkdownCodeItem(newAssetsPath, data.widthZoom) + QString("\n</center>    \n    \n    \n");
                    }
                }else {
                // 2. 组成IMG的Markdown代码
                    ImgClipText += clipMarkdownCodeItem(newAssetsPath, data.widthZoom) + QString("    \n");
                }
                emit sigFileOperationLog( "Clip From" + data.oldPath + " \nTo " + newAssetsPath);
                fileInfoVec.removeAt(i);
                break;
            }
        }
    }
    // 先添加MP4
    if(!VideoClipText.isEmpty()) {
        clipText = VideoClipText +"  \n\n";
    }
    // 再添加mp3
    if(!AudioClipText.isEmpty()) {
        clipText += AudioClipText + "  \n\n";
    }
    // 最后添加IMG
    if(ImgClipText.size() > 2) {
        clipText += QString("<center>    \n")+ ImgClipText.chopped(2)+QString("\n</center>    \n");
    }
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
QString FileOperation::makeDirPath(QString dirPath)
{
    QString tmpName ="Picture";
    int AssetNum = 0;
    QDir newDir(dirPath);
    while(newDir.exists(dirPath+"/"+tmpName)){
        AssetNum++;
        tmpName = "Picture";
        tmpName += "-" + QString("%1").arg(AssetNum, 2, 10, QLatin1Char('0'));
    }
    newDir.mkpath(dirPath+"/"+tmpName);
    return dirPath+"/"+tmpName;
}

bool FileOperation::copyImgVideo(QDir& CurrentPath, const QStringList& fileNameArr, const QString & nameList, QString &searchResult)
{
    QDir tmpCurPath = CurrentPath;
    QStringList nameStrList;
    if(nameList.contains(",",Qt::CaseSensitive)){
        nameStrList = nameList.split(",");
    }else if(nameList.contains(QString::fromLocal8Bit("，"),Qt::CaseSensitive)){
        nameStrList = nameList.split("，");
    }else{
        nameStrList = nameList.split(" ");
    }
    QString Path;
    if(nameList.isEmpty()){
        Path = makeDirPath(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    }else{
        Path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }

    for(int i = 0; i< fileNameArr.size(); ++i){
        QString name;
        if(!nameList.isEmpty()){
            if(nameStrList.size() > i){
                name = nameStrList.at(i);
            }else{
                name = nameStrList.last();
            }
        }
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
            desPath = Path + "/" + QString("%1").arg(i+1, 2, 10, QLatin1Char('0'))+"."+ suffix;
        }else{
            QString newName = newNumFileName(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation), name, suffix);
            desPath = Path + "/" + newName+"."+ suffix;
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


bool FileOperation::getSearchResultFromMarkdownCode(const QString &currentFullPath, const QString &codeText, const QString& nameList, QString &searchResult)
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

    return copyImgVideo(CurrentPath, fileNameArr,nameList , searchResult);
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
    for(int i = 0; i < resultFileList.size() && i < topFileCount*2; ++i){
        fileListTop20.append(resultFileList.at(i));
    }
}

void FileOperation::getHistorySubDirFileList(const QString &dirPath, QFileInfoList& fileListTop20){
    QDir dir(dirPath);
    QFileInfoList fileListTemp;
    QFileInfoList list = dir.entryInfoList(QDir::Files |QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    qSort(list.begin(),list.end(),compare);
    for(int i = 0, j = 0; i < list.size() && j < topFileCount; ++i){
//        qDebug() <<list.at(i).fileName();
        if(list.at(i).fileName() != "img" && list.at(i).fileName() != "video") {
            fileListTop20.append(list.at(i));
            j++;
        }
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

    if(!templateFileName.isEmpty()){
        QString templateFile = FullPath + "/" + templateFileName;
        if(!QFile::copy(templateFile, path))
        {
            emit sigFileOperationLog(templateFile + QString(" copy failed!"));
        }
        emit sigFileOperationLog(QString("Copy: "+templateFile+"\nTo Create:"+ path + "\nCreate File Success  !!!"));
        // 将拷贝的文件追加回车，成为最近修改文件
        QFile file(path);
        file.open(QIODevice::ReadWrite | QIODevice::Append);
        QTextStream txtOutput(&file);
        txtOutput << "\n";
        file.close();
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

bool FileOperation::createJsonFile(const QString& FullPath, QString& newFileName){
    QDir dir(FullPath);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    int max = -1;
    QString templateFileName;
    for(int i = 0; i < list.size(); ++i){
        if(list.at(i).fileName().indexOf("-新建文件.json")!=-1){
            int num = list.at(i).fileName().split("-").at(0).toInt();
            if(num > max){
                max = num;
            }
        }
        if(list.at(i).fileName().left(3) == "00-" && list.at(i).fileName().right(5) == ".json" ){
            templateFileName =list.at(i).fileName();
        }
    }
    int num = max+1;
    QString fileName = QString("%1").arg(num, 2, 10, QLatin1Char('0')) + "-新建文件.json";
    QString path =FullPath + "/" + fileName;

    if(!templateFileName.isEmpty()){
        QString templateFile = FullPath + "/" + templateFileName;
        if(!QFile::copy(templateFile, path))
        {
            emit sigFileOperationLog(templateFile + QString(" copy failed!"));
        }
        emit sigFileOperationLog(QString("Copy: "+templateFile+"\nTo Create:"+ path + "\nCreate File Success  !!!"));
        newFileName = fileName;
        return true;
    }
    emit sigFileOperationLog(QString("Copy templateFile To Create:"+ path + " Failed  !!!"));
    return false;
}

bool isNumFile(QString nameNum){
    bool ok;
    nameNum.toInt(&ok);
    return ok;
}

void FileOperation::getSearchDirFiles(const QString& path, QFileInfoList& fileListTemp, QString txt)
{
    QDir dir(path);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files|QDir::Dirs, QDir::Time);
    QFileInfo lastModifiedTimeFileName;
    if (!fileList.isEmpty())
    {
        for(int i = 0; i < fileList.size(); ++i){
            if(-1 != fileList.at(i).filePath().indexOf(txt) || fileList.at(i).filePath().contains(txt,Qt::CaseInsensitive)){
                if(isNumFile(fileList.at(i).fileName().split("-").at(0))){
                    fileListTemp.append(fileList.at(i));
                }
            }
        }
    }
}

void FileOperation::getSearchFileList(const QString &dirPath, QFileInfoList& fileList, QString txt){
    QDir dir(dirPath);
    QFileInfoList resultFileList;
    QFileInfoList fileListTemp;
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list)
    {
        if(fileInfo.isDir()) //book  study  test目录
        {
            fileListTemp.clear();
            getSearchDirFiles(fileInfo.absoluteFilePath(), fileListTemp, txt);
            for(int i = 0; i < fileListTemp.size(); ++i){
                resultFileList.append(fileListTemp.at(i));
            }
        }
    }
    qSort(resultFileList.begin(),resultFileList.end(),compare);
    for(int i = 0; i < resultFileList.size() && i < topFileCount; ++i){
        fileList.append(resultFileList.at(i));
    }
}

bool FileOperation::getReadMePath(QString dirPath, QString& markdownFile){
    QDir dir(dirPath);
    QStringList filters;
    filters << "*.md";
    dir.setNameFilters(filters);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries, QDir::Time);
    if(list.size() > 1){
        for(int i = 0; i < list.size(); ++i){
            if(list.at(i).fileName().compare("readme.md", Qt::CaseInsensitive)){
                markdownFile = list.at(i).filePath();
                return true;
            }
        }
        markdownFile = list.at(0).filePath();
        return true;
    }else if(list.size() == 1){
        markdownFile = list.at(0).filePath();
        return true;
    }
    return false;
}

bool FileOperation::delDesktopFile(QString dirPath, QString fileName){
    QDir folder(dirPath);
    QFile file(folder.filePath(fileName));
    if (file.exists()) {
        if(file.moveToTrash()){
            emit sigFileOperationLog(QString("将桌面资源: "+ dirPath+"/"+fileName+" 移动到回收站 !"));
        }else{
            emit sigFileOperationLog(QString("桌面资源: "+ dirPath+"/"+fileName+" 无法移动到回收站 !"));
        }
        return true;
    }
    emit sigFileOperationLog(QString("未找到桌面资源: "+ dirPath+"/"+fileName+" !"));
    return false;
}

///////////////////////////////  rename File   //////////////////////////

// 功能： 找到匹配成功的位置和字段  以")["为定位的基准
/// newFileDir 新修改成的文件名
/// curFileDir 当前修改的引用文件
/// oldFileAbsPath 旧文件的就对路径
void FileOperation::findReferByJump(QDir newFileDir, QDir curFileDir, QString context, QString oldFileAbsPath,QVector<ReText>& reTextList)
{
    QString oldFileName = oldFileAbsPath.split("/").last();
    curFileDir.cdUp();
    // 上一级目录
    int pos = 0;
    // 找到 "](" 位置 pos
    while ((pos = context.indexOf(']', pos)) != -1 && context.length() > pos + 1 && context.mid(pos + 1 ,1) == "(" ) {
        int endPos = context.indexOf(')', pos);
        if(endPos == -1) {
            break;
        }
        int posLeftMid = pos + 2;
        while (posLeftMid >= 0 && context.at(posLeftMid) != "[") {
            posLeftMid--;
        }
        if(posLeftMid < 0) {
            pos = endPos + 1;
            continue;
        }
        // 小括号内的字符 oldPath = ./XXXXX/XXX
        QString oldJumpPath = context.mid(pos + 2, endPos - pos - 2);
        // 排除：![](https://img.shields.io/badge/链接-视频课程-brightgreen.svg)
        if(oldJumpPath.contains("https://") || oldJumpPath.contains("http://")) {
            pos = endPos + 1;
            continue;
        }
        // 包含相对路径: 防止出现前半段相同的字符两个文件
        QString relativePath = curFileDir.relativeFilePath(oldFileAbsPath) + ")";
//        qDebug() << relativePath;
//        qDebug() << oldJumpPath;
        if(!(oldJumpPath+")").contains(relativePath) && !(oldJumpPath+")").contains(QDir::toNativeSeparators(relativePath))) {
            pos = endPos + 1;
            continue;
        }
        // 中括号内的字符 oldName = XXXX
        QString oldJumpName = context.mid(posLeftMid + 1, pos - posLeftMid - 1);
        // 找到")"的位置 endPos
        ReText reText;
        reText.x = posLeftMid;
        reText.y = endPos;
        reText.oldText = context.mid(reText.x, reText.y - reText.x + 1);
//        qDebug() << "x: " << posLeftMid << "  y: " << endPos ;
        // [新东方-写作课程-第04节课](./study/01-PART3的136真题练习/06-新东方-写作课程-第04节课.md).
        // x                                                                             y.
        // *引用的当前文件或者路径名字: 06-新东方-写作课程-第04节课.md
        QString reStrPath = oldJumpPath;
        QString reStrName = oldJumpName;
        // [新东方-写作课程-第04节课](./study/01-PART3的136真题练习/新名字.md)
        reStrPath.replace(oldFileName, newFileDir.dirName());
        QFileInfo oldFileInfo = QFileInfo(oldFileAbsPath);
        QFileInfo newFileInfo = QFileInfo(newFileDir.absolutePath());
        QString filename = oldFileInfo.fileName();
        if(oldFileInfo.baseName() == oldJumpName)  {
            reStrName = newFileInfo.baseName();
        }else if(oldFileInfo.fileName() == oldJumpName){
            reStrName = newFileInfo.fileName();
        }else{
            reStrName = oldJumpName;
        }
        // context.replace(refPos.x(), refPos.y() - refPos.x() + 1, reStr);
        reText.newText = "[" + reStrName + "](" +  reStrPath + ")";
        reTextList.append(reText);
        pos = endPos + 1;
    }
}

// 功能： 找到匹配成功的位置和字段  以"src="为定位的基准
/// newFileDir 新修改成的文件名
/// curFileDir 当前修改的引用文件
/// oldFileAbsPath 旧文件的就对路径
void FileOperation::findReferBySrc(QDir newFileDir, QDir curFileDir, QString context, QString oldFileAbsPath,QVector<ReText>& reTextList)
{
    QString oldFileName = oldFileAbsPath.split("/").last();
    QFileInfo oldFileInfo(oldFileAbsPath);
    curFileDir.cdUp();
    // 上一级目录
    QString desPath = curFileDir.relativeFilePath(newFileDir.absolutePath());
    if(!desPath.contains("/")) {
        desPath = "./" + desPath;
    }
    // 找到 "src=" 位置 pos
    int startIndex = 0;
    while (true) {
        int srcIndex = context.indexOf("src=", startIndex);
        if (srcIndex == -1)
            break;
        int openBracketIndex = context.lastIndexOf("<", srcIndex);
        int closeBracketIndex = context.indexOf(">", srcIndex);
        if (openBracketIndex != -1 && closeBracketIndex != -1) {
            QString subStr = context.mid(openBracketIndex, closeBracketIndex - openBracketIndex + 1);
            // 方式
            QString relativePath = curFileDir.relativeFilePath(oldFileAbsPath);
            if(oldFileInfo.isDir()) {
                relativePath = relativePath + "/";
            }
            if(subStr.contains(relativePath) || subStr.contains(QDir::toNativeSeparators(relativePath))) {
                ReText reText;
                reText.x = openBracketIndex;
                reText.y = closeBracketIndex;
                reText.oldText = context.mid(reText.x, reText.y - reText.x + 1);
                // <img src=./img/15-4.png alt=15-4 style=zoom:50%;/>.
                // x                                                y.
                // <audio src="./../09-MP3/高频短语速list记本/01-PART3的136真题练习/01 心情和感受.mp3"></audio>
                QString reNewPath = subStr;
                if(oldFileInfo.isFile()) {
                    reNewPath.replace("/" + oldFileName, "/" + newFileDir.dirName());
                }else {
                    reNewPath.replace(oldFileName + "/", newFileDir.dirName() + "/");
                }
                reText.newText = reNewPath;
                reTextList.append(reText);
            }
        }
        startIndex = closeBracketIndex;
    }
}
//updateReferDirByFile
void FileOperation::updateReferDirByFilePath(const QString& markdownAbsPath, const QString &repoPath,
                                   QDir curDir,const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList,int i) {
    QFile data(markdownAbsPath);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return;
    }
    QString context;
    QTextStream stream(&data);
    context = stream.readAll();
    data.close();
    updateReferDir(markdownAbsPath, repoPath, context, curDir, renameDirPath, replaceNameDirInfoList, i);
}

QVector<ReText> FileOperation::updateReTextDir(const QString& markdownAbsPath,const QString &renameDirPath,
                                               const QVector<DirRenameInfo> &replaceNameDirInfoList,int i){
    QDir curDir(markdownAbsPath);
    QFile data(markdownAbsPath);
    QVector<ReText> reTextList;
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return reTextList;
    }
    QString context;
    QTextStream stream(&data);
    context = stream.readAll();
    data.close();
    {
        QString oldNameAbsolutePath = renameDirPath + "/" + replaceNameDirInfoList.at(i).oldDirPath;
        QString newNameAbsolutePath = renameDirPath + "/" + replaceNameDirInfoList.at(i).newDirPath;
        QDir oldFileDir(oldNameAbsolutePath);
        QDir newFileDir(newNameAbsolutePath);
        if(context.contains(oldFileDir.dirName())){
            // 1. 定位基准 "]("
            findReferByJump(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
            // 2. 定位基准 "src="  <img src=./img/15-4.png alt=15-4 style=zoom:50%;/>
            findReferBySrc(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
        }
    }
    return reTextList;
}

void FileOperation::updateReferDir(const QString& markdownAbsPath, const QString &repoPath,const QString &context,
                                   QDir curDir,const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList,int i) {

    QString oldNameAbsolutePath = renameDirPath + "/" + replaceNameDirInfoList.at(i).oldDirPath;
    QString newNameAbsolutePath = renameDirPath + "/" + replaceNameDirInfoList.at(i).newDirPath;
    QDir oldFileDir(oldNameAbsolutePath);
    QDir newFileDir(newNameAbsolutePath);
    QVector<ReText> reTextList;
    if(context.contains(oldFileDir.dirName())){
        // 1. 定位基准 "]("
        findReferByJump(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
        // 2. 定位基准 "src="  <img src=./img/15-4.png alt=15-4 style=zoom:50%;/>
        findReferBySrc(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
    }
    if(!reTextList.isEmpty()) {
        ReFile reFile;
        reFile.reAsJumpCount = reTextList.size();
        QDir curDir(repoPath);
        curDir.setCurrent(repoPath);
        reFile.reFilePath = curDir.relativeFilePath(markdownAbsPath);
        QFileInfo markdownFile(markdownAbsPath);
        reFile.lastModifyTime = markdownFile.lastModified();
        reFile.reTextList = reTextList;
        replaceNameDirInfoList[i].reDirList.append(reFile);
    }
}

QVector<ReText> FileOperation::updateReTextFile(const QString& markdownAbsPath, const QString &renameDirPath,
                                                const QVector<FileRenameInfo> &replaceNameFileInfoList, int i){
    QFile data(markdownAbsPath);
    QDir curDir(markdownAbsPath);
    QVector<ReText> reTextList;
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return reTextList;
    }
    QString context;
    QTextStream stream(&data);
    context = stream.readAll();
    data.close();
    {
        QString oldNameAbsolutePath = renameDirPath + "/" + replaceNameFileInfoList.at(i).oldFilePath;
        QString newNameAbsolutePath = renameDirPath + "/" + replaceNameFileInfoList.at(i).newFilePath;
        QDir oldFileDir(oldNameAbsolutePath);
        QDir newFileDir(newNameAbsolutePath);
        if(context.contains(oldFileDir.dirName())){
            // 1. 定位基准 "]("
            findReferByJump(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
            // 2. 定位基准 "src="  <img src=./img/15-4.png alt=15-4 style=zoom:50%;/>
            findReferBySrc(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
        }
    }
    return reTextList;
}

void FileOperation::updateReferFileByFilePath(const QString& markdownAbsPath, const QString &repoPath,
                                    QDir curDir,const QString &renameDirPath, QVector<FileRenameInfo> &replaceNameFileInfoList, int i){
    QFile data(markdownAbsPath);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return;
    }
    QString context;
    QTextStream stream(&data);
    context = stream.readAll();
    data.close();
    updateReferFile(markdownAbsPath, repoPath, context, curDir,renameDirPath, replaceNameFileInfoList, i);
}

void FileOperation::updateReferFile(const QString& markdownAbsPath, const QString &repoPath,const QString &context,
                                    QDir curDir,const QString &renameDirPath, QVector<FileRenameInfo> &replaceNameFileInfoList, int i){
    QString oldNameAbsolutePath = renameDirPath + "/" + replaceNameFileInfoList.at(i).oldFilePath;
    QString newNameAbsolutePath = renameDirPath + "/" + replaceNameFileInfoList.at(i).newFilePath;
    QDir oldFileDir(oldNameAbsolutePath);
    QDir newFileDir(newNameAbsolutePath);
    QVector<ReText> reTextList;
    if(context.contains(oldFileDir.dirName())){
        // 1. 定位基准 "]("
        findReferByJump(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
        // 2. 定位基准 "src="  <img src=./img/15-4.png alt=15-4 style=zoom:50%;/>
        findReferBySrc(newFileDir, curDir, context, oldNameAbsolutePath, reTextList);
    }
    if(!reTextList.isEmpty()) {
        ReFile reFile;
        reFile.reAsJumpCount = reTextList.size();
        QDir curDir(repoPath);
        curDir.setCurrent(repoPath);
        reFile.reFilePath = curDir.relativeFilePath(markdownAbsPath);
        QFileInfo markdownFile(markdownAbsPath);
        reFile.lastModifyTime = markdownFile.lastModified();
        reFile.reTextList = reTextList;
        replaceNameFileInfoList[i].reFileList.append(reFile);
    }
}

void FileOperation::getMarkdownQString(const QString& markdownAbsPath, const QString &repoPath,
                                       const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList
                                       , QVector<FileRenameInfo> &replaceNameFileInfoList, int index) {
    QFile data(markdownAbsPath);
    QDir curDir(markdownAbsPath);
    curDir.setCurrent(markdownAbsPath);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return;
    }
    QString context;
    QTextStream stream(&data);
    context = stream.readAll();
    data.close();
    if(index == -1) { // 更新所有文件的引用
        for(int i = 0; i < replaceNameDirInfoList.size(); ++i) {
            updateReferDir(markdownAbsPath, repoPath, context, curDir,renameDirPath, replaceNameDirInfoList, i);
        }
        for(int i = 0; i < replaceNameFileInfoList.size(); ++i) {
            updateReferFile(markdownAbsPath, repoPath, context, curDir,renameDirPath, replaceNameFileInfoList, i);
        }
    }else{ // 更新下标是index的文件引用
        int len = replaceNameDirInfoList.length();
        if(index < len) {
            updateReferDir(markdownAbsPath, repoPath, context, curDir,renameDirPath, replaceNameDirInfoList, index);
        }else{
            updateReferFile(markdownAbsPath, repoPath, context, curDir,renameDirPath, replaceNameFileInfoList, index - len);
        }
    }
}

void FileOperation::getRefMarkdownFile(const QString& subPath, const QString &repoPath, const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList
                                       , QVector<FileRenameInfo> &replaceNameFileInfoList,int index)
{
    QDir dir(subPath);
    QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files, QDir::Time);
    if (!fileList.isEmpty()) {
        for(int i = fileList.size() - 1; i >= 0; --i) {
            if(isMarkdownFile(fileList.at(i).fileName())) { // 只遍历markdown文件
                getMarkdownQString(fileList.at(i).absoluteFilePath(), repoPath, renameDirPath, replaceNameDirInfoList, replaceNameFileInfoList, index);
            }
        }
    }
}

void FileOperation::updateReplaceNameRefereceList(const QString &repoPath, const QString &renameDirPath, QVector<DirRenameInfo> &replaceNameDirInfoList,
                                                  QVector<FileRenameInfo> &replaceNameFileInfoList,int index)
{
    QDir dir(repoPath);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
    foreach(QFileInfo fileInfo, list) {
        if(fileInfo.isDir()) {  //book  study  test目录
            getRefMarkdownFile(fileInfo.absoluteFilePath(), repoPath, renameDirPath, replaceNameDirInfoList, replaceNameFileInfoList, index);
        }else if(isMarkdownFile(fileInfo.fileName())) { // readme等仓库目录
            // 只遍历markdown文件
            getMarkdownQString(fileInfo.absoluteFilePath(), repoPath, renameDirPath, replaceNameDirInfoList, replaceNameFileInfoList, index);
        }
    }
}

void FileOperation::updateReplaceNameByList(const QString &listPathAbs, QVector<DirRenameInfo> &replaceNameDirInfoList, QVector<FileRenameInfo> &replaceNameFileInfoList)
{
    QFile data(listPathAbs);
    if (!data.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<< "Can't open the file!";
        return;
    }
    QString context;
    while (!data.atEnd())
    {
        bool hasFind = false;
        QByteArray line = data.readLine();
        QString LineStr(line);
        int len = LineStr.length();
        for(int i = 0; i < replaceNameDirInfoList.size(); ++i) {
            QString oldName = replaceNameDirInfoList.at(i).oldDirPath.split("/").last();
            if(oldName.length() < len && LineStr.indexOf(oldName) == 0) {
                QString newName = LineStr.remove(0, oldName.length()).trimmed();
                if(!newName.isEmpty()) {
                    //                QString newName = LineStr.split(oldName).last();
                    replaceNameDirInfoList[i].newDirPath = newName;
                    hasFind = true;
                }
                break;
            }
        }

        for(int i = 0; i < replaceNameFileInfoList.size() && !hasFind; ++i) {
            QString oldName = replaceNameFileInfoList.at(i).oldFilePath.split("/").last();
            if(oldName.length() < len && LineStr.indexOf(oldName) == 0) {
                QString newName = LineStr.remove(0, oldName.length()).trimmed();
                if(!newName.isEmpty()) {
                    replaceNameFileInfoList[i].newFilePath = newName;
                    hasFind = true;
                }
                break;
            }
        }
    }
    data.close();
}


