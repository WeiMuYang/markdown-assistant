#include "assets_data.h"
#include "data_type.h"
#include <QDebug>
#include <QDate>
#include <QImageReader>

AssetsData::AssetsData()
{

}

bool AssetsData::updateImgVideoFile(QString path, int index)
{
    QDir dir(path);
    if (!dir.exists()){
        return false;
    }
    dir.setFilter(QDir::Files);
    dir.setNameFilters(assetsTypeList_); // get from json
    dir.setSorting(QDir::SortFlag::Name);
    QFileInfoList list = dir.entryInfoList();
    QDate time;
    time = QDate::currentDate();//获取当前时间
    for(int i = 0; i < list.size(); ++i){
        ImgData data;
        data.oldName = list.at(i).fileName();
        data.oldPath = list.at(i).absoluteFilePath();
        data.oldFileInfo = list.at(i);
        if(data.oldName.right(4) != ".mp4"){
            data.widthZoom = getImgFileSizePercent(list.at(i).absoluteFilePath());
        }else{
            data.widthZoom = 50;
        }
        if(time == list.at(i).birthTime().date()){
            addImageList_.push_back(data);
        }else{
            delImageList_.push_back(data);
        }
    }
    return true;
}

int AssetsData::getImgFileSizePercent(const QString& path) {
    QImageReader reader(path);
    reader.setAutoTransform(true);
    int width = reader.read().width();
    // 最大宽度是1200个像素
    // 导出PDF会继续放大一倍，也就是说最大宽度是600像素
    // <img src="./img/09-1.png" alt="09-1" width=100%; />  使用width的缩放来添加图片
    if(width >= 1200 * 0.9) {
        return AssetsZoomSize::Size100;
    }else if(width >= 1200 * 0.65) {
        return AssetsZoomSize::Size80;
    }else if(width >= 1200 * 0.35) {
        return AssetsZoomSize::Size50;
    }else if(width >= 1200 * 0.2) {
        return AssetsZoomSize::Size30;
    }else{
        // 行内icon
        return AssetsZoomSize::Size5;
    }
}

QVector<ImgData> AssetsData::getNewAddImgVideoFile(QString path) {
    QDir dir(path);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(assetsTypeList_); // get from json
    dir.setSorting(QDir::SortFlag::Name);
    QFileInfoList list = dir.entryInfoList();
    QVector<ImgData> result;
    for (int i = 0; i < addImageList_.count(); ++i) {
        QString oldName = addImageList_.at(i).oldName;
        for(auto it = list.begin(); it < list.end(); ++it){
            if(it->fileName() == oldName){
                list.erase(it);
                break;
            }
        }
    }
    for (int i = 0; i < delImageList_.count(); ++i) {
        QString oldName = delImageList_.at(i).oldName;
        for(auto it = list.begin(); it < list.end(); ++it){
            if(it->fileName() == oldName){
                list.erase(it);
                break;
            }
        }
    }
    for(int i = 0; i < list.size(); ++i){
        ImgData data;
        data.oldName = list.at(i).fileName();
        data.oldPath = list.at(i).absoluteFilePath();
        data.oldFileInfo =list.at(i);
        if(data.oldName.right(4) != ".mp4"){
            data.widthZoom = getImgFileSizePercent(list.at(i).absoluteFilePath());
        }else{
            data.widthZoom = AssetsZoomSize::Size50;
        }
        result.push_back(data);
    }
    return result;
}

// 根据oldName  -->  oldPath
ImgData AssetsData::matchOldName(QString name){
    ImgData data;
    for(int i = 0; i < delImageList_.size(); ++i){
        data = delImageList_.at(i);
        if(data.oldName == name){
            return data;
        }
    }
    for(int i = 0; i < addImageList_.size(); ++i){
        data = addImageList_.at(i);
        if(data.oldName == name){
            return data;
        }
    }
    return data;
}

QVector<ImgData> AssetsData::getAddList(){
    return addImageList_;
}
void AssetsData::clearAddList(){
    addImageList_.clear();
}

QVector<ImgData> AssetsData::getDelList(){
    return delImageList_;
}
void AssetsData::clearDelList(){
    delImageList_.clear();
}

bool AssetsData::delAddImageListByOldName(QString name, ImgData& data){
    for(auto it = addImageList_.begin(); it < addImageList_.end(); ++it){
        if(it->oldName == name){
            data = *it;
            addImageList_.erase(it);
            return true;
        }
    }
    return false;
}

bool AssetsData::delDelImageListByOldName(QString name, ImgData& data){
    for(auto it = delImageList_.begin(); it < delImageList_.end(); ++it){
        if(it->oldName == name){
            data = *it;
            delImageList_.erase(it);
            return true;
        }
    }
    return false;
}

bool AssetsData::insertDataAddImageList(ImgData data){
    for(auto it = addImageList_.begin(); it < addImageList_.end(); ++it){
        if(it->oldName == data.oldName){
            return false;
        }
    }
    addImageList_.push_back(data);
    return true;
}

bool AssetsData::insertDataDelImageList(ImgData data){
    for(auto it = delImageList_.begin(); it < delImageList_.end(); ++it){
        if(it->oldName == data.oldName){
            return false;
        }
    }
    delImageList_.push_back(data);
    return true;
}

bool AssetsData::searchAddImgListByOldName(QString oldName, ImgData& data){
    for(auto it = addImageList_.begin(); it < addImageList_.end(); ++it){
        if(it->oldName == oldName){
            data =*it;
            return true;
        }
    }
    return false;
}

bool AssetsData::searchDelImgListByOldName(QString oldName, ImgData& data){
    for(auto it = delImageList_.begin(); it < delImageList_.end(); ++it){
        if(it->oldName == oldName){
            data =*it;
            return true;
        }
    }
    return false;
}

void AssetsData::setAssetsTypes(QStringList assetsTypes)
{
    assetsTypeList_.swap(assetsTypes);
}

bool AssetsData::modifyAddAssetsListZoomWidth(QString name, int zoom){
    for(auto it = addImageList_.begin(); it < addImageList_.end(); ++it){
        if(it->oldName == name){
            it->widthZoom = zoom;
            return true;
        }
    }
    return false;
}

bool AssetsData::modifyDelAssetsListZoomWidth(QString name, int zoom){
    for(auto it = delImageList_.begin(); it < delImageList_.end(); ++it){
        if(it->oldName == name){
            it->widthZoom = zoom;
            return true;
        }
    }
    return false;
}

