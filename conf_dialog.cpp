#include "conf_dialog.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include "debug_box.h"

confDialog::confDialog()
{
}

bool confDialog::readConf(QString path)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        DebugBox(__FUNCTION__, __LINE__,"read json error");
        return false;
    }
    confDataClear();
    QByteArray data(file.readAll());
    file.close();

    QJsonParseError jError;	//创建QJsonParseError对象
    //使用QJsonDocument的fromJson函数读取json串，并将QJsonParseError对象传入获取错误值
    QJsonDocument jDoc = QJsonDocument::fromJson(data, &jError);
    //判断QJsonParseError对象获取的error是否包含错误，包含则返回0
    if(jError.error != QJsonParseError::NoError){
        DebugBox(__FUNCTION__, __LINE__,"json format error");
        return false;
    }
    QJsonObject jObj = jDoc.object();
    analysisJson(jObj);
    return true;
}

void confDialog::analysisJson(QJsonObject &rootObj){
    //1. 读取ImagePath
    QJsonArray ImagePathArr = rootObj["ImagePath"].toArray();
    //2. 获取ImagePathObj中指定键的值（值对应的类型直接使用对应函数转）
    for(int i = 0; i < ImagePathArr.size(); ++i){
        QJsonObject  data = ImagePathArr.at(i).toObject();
        NamePath namePath;
        namePath.key = data.keys().at(0);  // 只有一个
        namePath.value = data[namePath.key].toString();
        imgNamePathMap_.push_back(namePath);
    }
    QJsonArray TarPathArr = rootObj["TargetPath"].toArray();
    for(int i = 0; i < TarPathArr.size(); ++i){
        QJsonObject data = TarPathArr.at(i).toObject();
        NamePath namePath;
        namePath.key = data.keys().at(0);
        namePath.value = data[namePath.key].toString();
        tarNamePathMap_.push_back(namePath);
    }
    QJsonArray IntervalArr = rootObj["Interval"].toArray();
    for(int i = 0; i < IntervalArr.size(); ++i){
        QString value = IntervalArr.at(i).toString();
        intervalArr_.push_back(value);
    }
    QJsonArray AssetsType = rootObj["AssetsType"].toArray();
    for(int i = 0; i < AssetsType.size(); ++i){
        QString value = AssetsType.at(i).toString();
        assetsType_ << value;
    }

    QJsonObject SoftPathMap = rootObj["Software"].toObject();
    for(int i = 0; i < SoftPathMap.keys().size(); ++i){
        QString key = SoftPathMap.keys().at(i);
        QString value = SoftPathMap[key].toString();
        softWarePathMap_.insert(key, value);
    }

    meetFilePath_ = rootObj["MeetFilePath"].toString();
}

QString confDialog::getImgPathByKey(QString key){
    for(auto it = imgNamePathMap_.begin(); it != imgNamePathMap_.end(); ++it)
    {
        if(it->key == key)
        {
            return it->value;
        }
    }
    DebugBox(__FUNCTION__, __LINE__,"can't find "+ key + " from imgNamePathMap");
    return QString("");
}

QString confDialog::getTarPathByKey(QString key){
    if(key.isEmpty()){
        return QString("");
    }
    for(auto it = tarNamePathMap_.begin(); it != tarNamePathMap_.end(); ++it)
    {
        if(it->key == key)
        {
            return it->value;
        }
    }
    DebugBox(__FUNCTION__, __LINE__,"can't find "+ key + " from tarNamePathMap");
    return QString("");
}

bool confDialog::findTarPath(QString key, NamePath& data){
    for(auto it = tarNamePathMap_.begin(); it != tarNamePathMap_.end(); ++it)
    {
        if(it->key == key)
        {
            data = *it;
            return true;
        }
    }
    return false;
}

bool confDialog::findImgPath(QString key, NamePath& data){
    for(auto it = imgNamePathMap_.begin(); it != imgNamePathMap_.end(); ++it)
    {
        if(it->key == key)
        {
            data = *it;
            return true;
        }
    }
    return false;
}

bool iniFilePath(QString& iniPath, QString& iniAndjsonPath){
    QString iniFileName = "conf.ini";
    QString iniFileDir = "conf";
    QString dotPath = QDir::currentPath();;
    QDir dir1(dotPath);
    while(!dir1.exists(iniFileDir) && !dir1.isRoot()){
        dir1.cdUp();
    }
    if(dir1.exists(iniFileDir)){
        dir1.cd(iniFileDir);
        if(dir1.exists(iniFileName)){
            iniAndjsonPath =dir1.absolutePath();
            iniPath = dir1.absolutePath()+"/"+iniFileName;
            return true;
        }
    }
    return false;
}

bool confDialog::readIniFile()
{
    QString iniPath;
    QString iniAndjsonPath;
    if(!iniFilePath(iniPath,iniAndjsonPath)){
        return false;
    }
    QFile file(iniPath);
    if(!file.open(QIODevice::ReadOnly)){
        DebugBox(__FUNCTION__, __LINE__,"read json error");
        return false;
    }
    QByteArray data(file.readAll());
    file.close();

    QJsonParseError jError;	//创建QJsonParseError对象
    //使用QJsonDocument的fromJson函数读取json串，并将QJsonParseError对象传入获取错误值
    QJsonDocument jDoc = QJsonDocument::fromJson(data, &jError);
    //判断QJsonParseError对象获取的error是否包含错误，包含则返回0
    if(jError.error != QJsonParseError::NoError){
        DebugBox(__FUNCTION__, __LINE__,"json format error");
        return false;
    }
    QJsonObject rootObj = jDoc.object();
    QJsonArray RecentFile = rootObj["RecentFile"].toArray();
    for(int i = 0; i < RecentFile.size(); ++i){
        QString value = RecentFile.at(i).toString();
        iniFile_.recentFileList.push_back(value);
    }
    //0.HostName
    iniFile_.hostName = rootObj["HostName"].toString();
    iniFile_.version = rootObj["Version"].toString();
    iniFile_.date = rootObj["Date"].toString();
    iniFile_.iniAndJsonPath = iniAndjsonPath;
    return true;
}

void confDialog::confDataClear(){
    imgNamePathMap_.clear();
    tarNamePathMap_.clear();
    intervalArr_.clear();
    assetsType_.clear();
//    typoraExePath_.clear();
//    vsCodePath_.clear();
    meetFilePath_.clear();
    softWarePathMap_.clear();
    //    iniFile_.iniAndjsonPath.clear();
    //    iniFile_.recentFileList.clear();
    //    iniFile_.version.clear();
    //    iniFile_.date.clear();
}

void confDialog::clearAll(){
    imgNamePathMap_.clear();
    tarNamePathMap_.clear();
    intervalArr_.clear();
    assetsType_.clear();
//    typoraExePath_.clear();
    meetFilePath_.clear();
//    vsCodePath_.clear();
    softWarePathMap_.clear();
    iniFile_.iniAndJsonPath.clear();
    iniFile_.recentFileList.clear();
    iniFile_.version.clear();
    iniFile_.date.clear();
}
