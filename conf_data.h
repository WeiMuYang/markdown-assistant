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
    QString getImgPathByKey(QString key);

    QString getTarPathByKey(QString key);

    bool findTarPath(QString key, NamePath& data);
    bool findImgPathByName(QString key, NamePath& data);
    const QList<NamePath>& getTarPaths(){
        return tarNamePathMap_;
    }
    const QList<NamePath>& getImgPaths(){
        return imgNamePathMap_;
    }
    QString getMeetFilePath(){
        return meetFilePath_;
    }
    const QStringList& getAssetsTypes(){
        return assetsType_;
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

    bool addTarNamePath(const QString& name, const QString& path, QString& existName) {
        NamePath np;
        np.key = name;
        np.value = path;
        for(int i = 0; i < tarNamePathMap_.size(); ++i) {
            if(tarNamePathMap_.at(i).value == path) {
                existName = tarNamePathMap_.at(i).key;
                return false;
            }
        }
        tarNamePathMap_.append(np);
        return true;
    }

    bool addImgNamePath(const QString& name, const QString& path, QString& existName) {
        NamePath np;
        np.key = name;
        np.value = path;
        for(int i = 0; i < imgNamePathMap_.size(); ++i) {
            if(imgNamePathMap_.at(i).value == path) {
                existName = imgNamePathMap_.at(i).key;
                return false;
            }
        }
        imgNamePathMap_.append(np);
        return true;
    }
    bool delTarNamePath(QString path) {
        for(int i = 0; i < tarNamePathMap_.size(); ++i) {
            if(tarNamePathMap_.at(i).value == path) {
                tarNamePathMap_.removeAt(i);
            }
        }
        return false;
    }

    bool delAssetsNamePath(QString path) {
        for(int i = 0; i < imgNamePathMap_.size(); ++i) {
            if(imgNamePathMap_.at(i).value == path) {
                imgNamePathMap_.removeAt(i);
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
private: // need to change to private
    QList<NamePath> imgNamePathMap_;
    QList<NamePath> tarNamePathMap_;
    QVector<QString> intervalArr_;
    QStringList assetsType_;
    QString markdownSoftWarePath_;
    QString dataDirSoftWarePath_;
    QString meetFilePath_;
    IniFile iniFile_;
};

#endif // CONF_DATA_H
