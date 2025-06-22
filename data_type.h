#ifndef DATA_TYPE_H
#define DATA_TYPE_H
#include <QString>
#include <QDir>
#include <QVector>
#include <QFileInfo>
#include <QDateTime>

#define  MARKDOWN_SOFTWARE "MarkdownSoftware"
#define  DATA_DIR_SOFTWARE "DataDirSoftware"

#define VERSION "v1.0.0.beta"

enum AssetsZoomSize{
    Size100 = 100, Size80 = 80, Size50 = 49, Size30 = 32, Size5 = 5
};


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
    int widthZoom;
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

struct ReText
{
    int x;
    int y;
    QString oldText;
    QString newText;
};

struct ReFile
{
    QString reFilePath;
    int reAsJumpCount;
    QVector<ReText> reTextList;
    QDateTime lastModifyTime;
};

struct FileRenameInfo
{
    QString oldFilePath;
    QString newFilePath;
    QVector<ReFile> reFileList;
};

struct DirRenameInfo
{
    QString oldDirPath;
    QString newDirPath;
    QVector<ReFile> reDirList;
};


#endif // DATA_TYPE_H
