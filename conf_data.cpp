#include "conf_data.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include "debug_box.h"

ConfigData::ConfigData()
{
}

bool ConfigData::readConf(QString path)
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

void ConfigData::analysisJson(QJsonObject &rootObj){
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
        if(key == MARKDOWN_SOFTWARE) {
            markdownSoftWarePath_ = SoftPathMap[key].toString();
        }else if(key == DATA_DIR_SOFTWARE) {
            dataDirSoftWarePath_ = SoftPathMap[key].toString();
        }
    }
    meetFilePath_ = rootObj["MeetFilePath"].toString();
}

QString ConfigData::getImgPathByKey(QString key){
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

QString ConfigData::getTarPathByKey(QString key){
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

bool ConfigData::findTarPath(QString key, NamePath& data){
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

bool ConfigData::findImgPathByName(QString key, NamePath& data){
    for(auto it = imgNamePathMap_.begin(); it != imgNamePathMap_.end(); ++it)
    {
        if(it->value == key)
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
            iniAndjsonPath = dir1.absolutePath();
            iniPath = dir1.absolutePath() + "/" + iniFileName;
            return true;
        }
    }
    return false;
}


void ConfigData::getRencentJsonFiles(const QString& path) {
    QDir dir(path);
    QStringList filters;
    filters << "*.json";
    dir.setNameFilters(filters);
    QFileInfoList fileInfoList = dir.entryInfoList();

    QStringList fileList;
    for (const QFileInfo& fileInfo : fileInfoList) {
        if(fileInfo.fileName().left(3) != "00-" && fileInfo.fileName().left(3) != "99-") {
            iniFile_.recentFileList.append(fileInfo.fileName());
        }
    }
}

bool ConfigData::readIniFile()
{
    QString iniPath;
    QString iniAndjsonDir;
    if(!iniFilePath(iniPath, iniAndjsonDir)){
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
    //0.HostName
    iniFile_.hostName = rootObj["HostName"].toString();
    iniFile_.version = rootObj["Version"].toString();
    iniFile_.date = rootObj["Date"].toString();
    iniFile_.iniAndJsonPath = iniAndjsonDir;
    // 更新recent列表
    getRencentJsonFiles(iniAndjsonDir);

    return true;
}


bool ConfigData::writeIniFile() {
    QJsonObject jsonObject;
    jsonObject["HostName"] = iniFile_.hostName;
    QJsonArray recentFileArray;
    for(int i = 0; i < iniFile_.recentFileList.size(); ++i) {
        recentFileArray.append(iniFile_.recentFileList.at(i));
    }
    jsonObject["RecentFile"] = recentFileArray;
    jsonObject["Version"] = iniFile_.version;
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy.MM.dd");
    jsonObject["Date"] = current_date;

    // 创建JSON文档
    QJsonDocument jsonDoc(jsonObject);

    // 将JSON文档写入文件
    QFile file(iniFile_.iniAndJsonPath + "/conf.ini");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write(jsonDoc.toJson());
        file.close();
        qDebug() << "JSON文件写入成功";
        return true;
    } else {
        qDebug() << "无法写入JSON文件";
        return false;
    }
}

void ConfigData::confDataClear(){
    imgNamePathMap_.clear();
    tarNamePathMap_.clear();
    intervalArr_.clear();
    assetsType_.clear();
    meetFilePath_.clear();
    markdownSoftWarePath_.clear();
    dataDirSoftWarePath_.clear();
}

void ConfigData::clearAll(){
    imgNamePathMap_.clear();
    tarNamePathMap_.clear();
    intervalArr_.clear();
    assetsType_.clear();
    meetFilePath_.clear();
    markdownSoftWarePath_.clear();
    dataDirSoftWarePath_.clear();
    iniFile_.iniAndJsonPath.clear();
    iniFile_.recentFileList.clear();
    iniFile_.version.clear();
    iniFile_.date.clear();
}

bool ConfigData::readCharConfFile(QString path, QMap<QString, QString>& map)
{
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly)){
        DebugBox(__FUNCTION__, __LINE__,"read json error");
        return false;
    }
    map.clear();
    QByteArray data(file.readAll());
    file.close();

    QJsonParseError jError;	//创建QJsonParseError对象
    QJsonDocument jDoc = QJsonDocument::fromJson(data, &jError);
    if(jError.error != QJsonParseError::NoError){
        DebugBox(__FUNCTION__, __LINE__,"json format error");
        return false;
    }
    QJsonObject jObj = jDoc.object();
    analysisCharConfJson(jObj, map);
    return true;
}

void ConfigData::analysisCharConfJson(QJsonObject &rootObj,QMap<QString, QString>& replaceCharListMap){
    //1. replaceCharList
    QJsonArray replaceCharList = rootObj["replaceCharList"].toArray();
    for(int i = 0; i < replaceCharList.size(); ++i){
        QJsonObject data = replaceCharList.at(i).toObject();
        for(int j = 0; j < data.size(); ++j) {
            if(!data.keys().at(j).contains("?")){
                replaceCharListMap.insert(data.keys().at(j), data[data.keys().at(j)].toString());
                break;
            }
        }
    }
}

bool ConfigData::writeConfJson() {
    // 创建一个 JSON 对象
    QJsonObject jsonObject;
    // 添加路径数组
    QJsonArray imagePathArray;
    for(int i = 0; i < imgNamePathMap_.size(); ++i){
        QJsonObject imagePathObject;
        imagePathObject[imgNamePathMap_.at(i).key] = imgNamePathMap_.at(i).value;
        imagePathArray.append(imagePathObject);
    }
    jsonObject["ImagePath"] = imagePathArray;

    // 添加目标路径数组
    QJsonArray targetPathArray;
    for(int i = 0; i < tarNamePathMap_.size(); ++i){
        QJsonObject targetPathObject;
        targetPathObject[tarNamePathMap_.at(i).key] = tarNamePathMap_.at(i).value;
        targetPathArray.append(targetPathObject);
    }
    jsonObject["TargetPath"] = targetPathArray;

    // 添加时间间隔数组
    QJsonArray intervalArray;
    for(int i = 0; i < intervalArr_.size(); ++i) {
        intervalArray.append(intervalArr_.at(i));
    }
    jsonObject["Interval"] = intervalArray;

    // 添加资源类型数组
    QJsonArray assetsTypeArray;
    for(int i = 0; i < assetsType_.size(); ++i){
        assetsTypeArray.append(assetsType_.at(i));
    }
    jsonObject["AssetsType"] = assetsTypeArray;

    // 添加软件对象
    QJsonObject softwareObject;
    softwareObject[MARKDOWN_SOFTWARE] = markdownSoftWarePath_;
    softwareObject[DATA_DIR_SOFTWARE] = dataDirSoftWarePath_;

    jsonObject["Software"] = softwareObject;

    // 添加其他属性
    jsonObject["Version"] = iniFile_.version;
    QDateTime current_date_time =QDateTime::currentDateTime();
    QString current_date =current_date_time.toString("yyyy.MM.dd");
    jsonObject["Date"] = current_date;
    // 生成 JSON 文档
    QJsonDocument jsonDoc(jsonObject);

    // 将 JSON 文档转换为字符串
    QString jsonString = jsonDoc.toJson(QJsonDocument::Indented);

    // 将 JSON 字符串保存到文件
    QFile file( iniFile_.iniAndJsonPath + "/" + iniFile_.hostName + ".json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << jsonString;
        file.close();
        qDebug() << "JSON file saved successfully.";
    }
    else
    {
        qDebug() << "Failed to save JSON file.";
    }


    return true;
}
