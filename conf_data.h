#ifndef CONF_DATA_H
#define CONF_DATA_H

#include <QWidget>
#include <QList>
#include <QVector>
#include <QMap>
#include "data_type.h"

// 用来读写配置文件
class ConfigData
{
public:
    ConfigData();
    bool readConf(QString path);
    void analysisJson(QJsonObject &rootObj);
    QString getAssetsPathByKey(QString key);

    QString getRepoPathByKey(QString key);

    bool findTarPath(QString key, NamePath& data);
    bool findImgPathByName(QString key, NamePath& data);
    const QList<NamePath>& getRepoPaths(){
        return repoNamePathList_;
    }
    const QList<NamePath>& getAssetPaths(){
        return assetsNamePathList_;
    }
    QString getMeetFilePath(){
        return meetFilePath_;
    }
    const QStringList& getAssetsTypes(){
        return assetsType_;
    }

    void addAssetsTypes(const QString& s){
        assetsType_.append(s);
    }

    bool delAssetsTypes(const QString& s) {
        for(int i = 0; i < assetsType_.size(); ++i) {
            if(assetsType_.at(i) == s) {
                assetsType_.removeAt(i);
                return true;
            }
        }
        return false;
    }

    bool modifyAssetsTypes(int i, const QString& s) {
        if(i < assetsType_.size())  {
            assetsType_[i] = s;
            return true;
        }
        return false;
    }

    QString getMarkdownSoftPath() {
        return markdownSoftWarePath_;
    }
    QString getDataDirSoftPath() {
        return dataDirSoftWarePath_;
    }

    bool readIniFile();
    bool writeIniFile();
    void getRencentJsonFiles(const QString& path);

    IniFile getIniFile(){
        return iniFile_;
    }
    void confDataClear();
    void clearAll();

    bool writeConfJson();

    void setIniFileHostName(QString name){
        iniFile_.hostName = name;
    }

    bool addRepoPath(const QString& name, const QString& path, QString& existName) {
        NamePath np;
        np.key = name;
        np.value = path;
        for(int i = 0; i < repoNamePathList_.size(); ++i) {
            if(repoNamePathList_.at(i).value == path) {
                existName = repoNamePathList_.at(i).key;
                return false;
            }
        }
        repoNamePathList_.append(np);
        return true;
    }

    bool modifyRepoName(int i, const QString& name) {
        if(i < repoNamePathList_.size()) {
            repoNamePathList_[i].key= name;
            return true;
        }
        return false;
    }

    bool addAssetsPath(const QString& name, const QString& path, QString& existName) {
        NamePath np;
        np.key = name;
        np.value = path;
        for(int i = 0; i < assetsNamePathList_.size(); ++i) {
            if(assetsNamePathList_.at(i).value == path) {
                existName = assetsNamePathList_.at(i).key;
                return false;
            }
        }
        assetsNamePathList_.append(np);
        return true;
    }

    bool modifyAssetsName(int i, const QString& name) {
        if(i < assetsNamePathList_.size()) {
            assetsNamePathList_[i].key= name;
            return true;
        }
        return false;
    }

    bool delRepoPath(QString path) {
        for(int i = 0; i < repoNamePathList_.size(); ++i) {
            if(repoNamePathList_.at(i).value == path) {
                repoNamePathList_.removeAt(i);
            }
        }
        return false;
    }

    bool delAssetsPath(QString path) {
        for(int i = 0; i < assetsNamePathList_.size(); ++i) {
            if(assetsNamePathList_.at(i).value == path) {
                assetsNamePathList_.removeAt(i);
            }
        }
        return false;
    }

    void setMarkdownSoftWarePath(const QString& path){
        markdownSoftWarePath_ = path;
    }

    void setDataDirSoftWarePath(const QString& path){
        dataDirSoftWarePath_ = path;
    }

    ///////////  rename file  ////////
    bool readCharConfFile(QString path, QMap<QString, QString> &map);
    void analysisCharConfJson(QJsonObject &rootObj,QMap<QString, QString>& replaceCharListMap);

    QList<NamePath> getAssetsPathDir() {
        return assetsNamePathList_;
    }

    void setVersion(QString v) {
        iniFile_.version = v;
    }


private: // need to change to private
    QList<NamePath> assetsNamePathList_;
    QList<NamePath> repoNamePathList_;
    QVector<QString> intervalArr_;
    QStringList assetsType_;
    QString markdownSoftWarePath_;
    QString dataDirSoftWarePath_;
    QString meetFilePath_;
    IniFile iniFile_;
};

#endif // CONF_DATA_H
