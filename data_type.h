#ifndef DATA_TYPE_H
#define DATA_TYPE_H
#include <QString>
#include <QDir>
#include <QVector>
#include <QFileInfo>

// config data used
struct NamePath{
    QString key;
    QString value;
};


// Assets Data used
struct ImgData{
    QString oldPath;
    QString oldName;
    QFileInfo oldFileInfo;
};

struct IniFile{
    QString hostName;
    QVector<QString> recentFileList;
    QString version;
    QString date;
    QString iniAndJsonPath;
};

typedef enum {
    High, Low
}ScreenRes;

typedef enum {
    None, TarCombox, SubCombox, NumSpinBox
}BoxSelect;




#endif // DATA_TYPE_H
