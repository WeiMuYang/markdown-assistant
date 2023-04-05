#ifndef ASSETS_DATA_H
#define ASSETS_DATA_H
#include <QString>
#include <QDir>
#include <QVector>
#include "data_type.h"

// 获取数据，存放数据

class AssetsData
{
public:
    AssetsData();
    bool updateImgVideoFile(QString path, int index);
    QString matchOldName(QString name);
    QVector<ImgData> getAddList();
    void clearAddList();

    QVector<ImgData> getDelList();
    void clearDelList();

    bool delAddImageListByOldName(QString name, ImgData& data);

    bool delDelImageListByOldName(QString name, ImgData& data);

    bool insertDataAddImageList(ImgData data);

    bool insertDataDelImageList(ImgData data);

    bool searchAddImgListByOldName(QString oldName, ImgData& data);

    bool searchDelImgListByOldName(QString oldName, ImgData& data);

    void setAssetsTypes(QStringList assetsTypes);

private:
    QVector<ImgData> addImageList_;
    QVector<ImgData> delImageList_;

    QStringList assetsTypeList_;
};

#endif // ASSETS_DATA_H
