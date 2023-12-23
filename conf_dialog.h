#ifndef CONF_DIALOG_H
#define CONF_DIALOG_H

#include <QWidget>
#include <QList>
#include <QVector>
#include <QMap>
#include "data_type.h"

// 用来读写配置文件
class confDialog
{
public:
    confDialog();
    bool readConf(QString path);
    void analysisJson(QJsonObject &rootObj);
    QString getImgPathByKey(QString key);

    QString getTarPathByKey(QString key);

    bool findTarPath(QString key, NamePath& data);
    bool findImgPath(QString key, NamePath& data);
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

    const QMap<QString,QString>& getSoftWarePathMap(){
        return softWarePathMap_;
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

    void addTarNamePath(QString name, QString path) {
        NamePath np;
        np.key = name;
        np.value = path;
        tarNamePathMap_.append(np);
    }
    bool delTarNamePath(QString path) {
        for(int i = 0; i < tarNamePathMap_.size(); ++i) {
            if(tarNamePathMap_.at(i).value == path) {
                tarNamePathMap_.removeAt(i);
                return true;
            }
        }
        return false;
    }

    ///////////  rename file  ////////
    bool readCharConfFile(QString path, QMap<QString, QString> &map);
    void analysisCharConfJson(QJsonObject &rootObj,QMap<QString, QString>& replaceCharListMap);
private: // need to change to private
    QList<NamePath> imgNamePathMap_;
    QList<NamePath> tarNamePathMap_;
    QVector<QString> intervalArr_;
    QStringList assetsType_;
    QMap<QString,QString> softWarePathMap_;
    QString meetFilePath_;
    IniFile iniFile_;
};

#endif // CONF_DIALOG_H
