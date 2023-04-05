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
    QString getTyporaPath(){
        return typoraExePath_;
    }

    const QStringList& getAssetsTypes(){
        return assetsType_;
    }

    const QMap<QString,QString>& getSoftWarePathMap(){
        return softWarePathMap_;
    }
private: // need to change to private
    QList<NamePath> imgNamePathMap_;
    QList<NamePath> tarNamePathMap_;
    QVector<QString> intervalArr_;
    QStringList assetsType_;
    QString version_;
    QString date_;
    QString typoraExePath_;
    QString vsCodePath_;
    QMap<QString,QString> softWarePathMap_;
};

#endif // CONF_DIALOG_H
