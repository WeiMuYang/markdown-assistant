#include "assets_data.h"
#include <QDebug>
#include <QDate>

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
        data.oldFileInfo =list.at(i);
        if(time == list.at(i).birthTime().date()){
            addImageList_.push_back(data);
        }else{
            delImageList_.push_back(data);
        }
    }
    return true;
}

// 根据oldName  -->  oldPath
QString AssetsData::matchOldName(QString name){
    QString path;
    for(int i = 0; i < delImageList_.size(); ++i){
        ImgData data = delImageList_.at(i);
        if(data.oldName == name){
            return data.oldPath;
        }
    }
    for(int i = 0; i < addImageList_.size(); ++i){
        ImgData data = addImageList_.at(i);
        if(data.oldName == name){
            return data.oldPath;
        }
    }
    return path;
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

