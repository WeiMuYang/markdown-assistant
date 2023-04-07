#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "assets_data.h"
#include <QListWidgetItem>
#include <QLabel>
#include "conf_dialog.h"
#include "video_thr.h"
#include "file_operation.h"
#include "open_ex_program.h"
#include "data_type.h"
#include "get_assets_dialog.h"
#include "about_dialog.h"
#include "modify_name_dialog.h"


namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void initListWgt();
    void updateListWgt();
    QString matchOldName(QString name);
    void initImgPathTarPathCombox();
    void sycImgDataByOldName(QString name);
    void updateLastModifyFile();
    void setWindowStyle();
    void initStatusBar();
    void InitMainWindowMenu();
    void setScreenWitdh(int w);
    int getScreenWitdh();
    void initScreenResNormal();
    void setNormalViewByScreenRes();
    void setSampleViewByScreenRes();
    ScreenRes getScrrenRes();
    void updateSubDirCombox();

    void setSampleView();
    void setNormalView();
    void setConfigFilePathByUserName(const IniFile& iniFile);
private slots:
    void updateListDataAndWgtSlot();
    void changeModeSlot();
    void setImgPathSlot(QString currentStr);
    void setTarPathSlot(QString currentStr);
    void setSubPathSlot(QString currentStr);
    void setImageToLabelSlot(QImage image);

    void moveDelItemToAddListSlot(const QModelIndex &index);
    void moveAddItemToDelListSlot(const QModelIndex &index);
    void itemEnteredSlot(QListWidgetItem *item);

    void on_clipPbn_clicked();
    void on_lastFileNumPbn_clicked();
    void on_refreshPbn_clicked();
    void on_syncPbn_clicked();
    void on_modePbn_clicked();
    void on_toolPbn_clicked();

    void appendTextToLog(QString log);
    void setStatusBar(QString msg, bool isCorrect);

    void on_numSpinBox_valueChanged(int arg1);

    void whoIsBoxSelection(BoxSelect select);
    void changNumStyle(bool flags);
    void changSubPathStyle(bool flags);
    void changTarPathStyle(bool flags);

    void getAssetsSlot();
    void reNameSlot();
    void openAboutSlot();
    void searchAssetsByCodeSlot(QString s,QString r);
    void simpleViewSlot();

    void startSlot();
    void clearLogSlot();

    void setConfigFilePath();

    void openRecentFileSlot();

protected:
    // 窗口设置为随着窗口变化而变化
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    AssetsData addDelListData_;
    confDialog confDialog_;
    QClipboard *clip_;
    bool isIconMode_{false};
    unsigned int clickNum_{0};
    QString imgPath_;
    QString tarPath_;
    QString subDirName_;
    QString fullTarPath_;
    QString currentFile_;
    QString labelPath_;
    VideoThr* videoThr_;
    FileOperation fileOp_;

    QLabel* pStatusLabelIcon_;
    QLabel* pStatusLabelMsg_;
    QLabel* pStatusLabelCurrentFile_;

    OpenExProgram openExPro_;

    int scrrenWidth_;
    BoxSelect boxSelect_;

    GetAssetsDialog* getAssetsDialog_;
    AboutDialog* aboutDialog_;
    ModifyNameDialog* modifyNameDialog_;
    int simpleViewNum_{0};
    QString configFilePath_;
};

#endif // MAINWINDOW_H
