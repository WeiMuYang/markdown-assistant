#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "assets_data.h"
#include <QListWidgetItem>
#include <QLabel>
#include <QComboBox>
#include <QShowEvent>
#include <QTimer>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QMediaPlayer>
#include "conf_data.h"
#include "video_thr.h"
#include "file_operation.h"
#include "open_ex_program.h"
#include "data_type.h"
#include "get_assets_dialog.h"
#include "about_dialog.h"
#include "rename_file_name.h"
#include "modify_conf_dialog.h"
#include "create_markdown_dir.h"
#include "get_markdown_dialog.h"


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
    void changedPercentByLabelImg(int width);
    QString matchOldName(QString name);
    void initImgPathTarPathCombox();
    void sycImgDataByOldName(QString name);
    void updateLastModifyFile();
    void updateRepoHistoryFileList();
    void setWindowStyle();
    void setItemIcon(const ImgData& data, QListWidgetItem* item);
    void initTray();
    void initStatusBar();
    void InitMainWindowMenu();
    void updateActionConfFileList();
    void initHistoryFileList();
    void initAddDelListMenu();
    void setScreenWitdh(int w);
    int getScreenWitdh();
    void initScreenResNormal();
    void setInitNormalViewByScreenRes();
    void setInitSampleViewByScreenRes();
    ScreenRes getScrrenRes();
    void updateSubDirCombox();
    void setSampleView();
    void setNormalView();
    void setConfigFilePathByUserName(const IniFile& iniFile);
    void setComboBoxToolTip(QComboBox* box);
    void writeCurrentFile(QString str);
    QString getAssetsPath();
    bool addRepo2Conf(QString name,QString path);
    bool addAssetsDir2Conf(QString newName,QString pathAbs);
    void playAudioMp3(const QString& path);
    void adjustMovieSize(QMovie* movie, const QSize& labelSize);

    void setIconNum(int num, char *argv[]);
private slots:
    void updateListDataAndWgtSlot();
    void changeModeSlot();
    void setImgPathSlot(QString currentStr);
    void setTarPathSlot(QString currentStr);
    void setSubPathSlot(QString currentStr);
    void setImageToLabelSlot(QImage image);

    void moveDelItemToAddListSlot(const QModelIndex &index);
    void moveAddItemToDelListSlot(const QModelIndex &index);
    void showAddListMenuSlot(QPoint pos);
    void delFromAddListSlot();
    void moveFromAddListSlot();
    void clearFromAddListSlot();
    void showDelListMenuSlot(QPoint pos);
    void delFromDelListSlot();
    void moveFromDelListSlot();
    void clearFromDelListSlot();
    void clipFromDelListSlot();
    void itemEnteredSlot(QListWidgetItem *item);
    void changeSelectSatusSlot();


    void ChangeToHistoryFile();
    void OpenHistoryFile();
    void CompareRenameFileList(QString , QString);
    void OpenRenameDirPath(QString path);

    void on_clipPbn_clicked();
    void on_lastFileNumPbn_clicked();
    void on_refreshPbn_clicked();
    void on_syncPbn_clicked();
    void on_modePbn_clicked();
    void on_toolPbn_clicked();
    void syncAddListTimelySlot();

    void appendTextToLog(QString log);
    void setStatusBar(QString msg, bool isCorrect);

    void on_numSpinBox_valueChanged(int arg1);

    void whoIsBoxSelection(BoxSelect select);
    void changNumStyle(bool flags);
    void changSubPathStyle(bool flags);
    void changTarPathStyle(bool flags);

    void getAssetsSlot();
    void newConfFileSlot();
    void openAboutSlot();
    void searchAssetsByCodeSlot(QString s,QString r);
    void simpleViewSlot();

    void updateConfFileSlot();
    void updateDataAndWidget();
    void copyHistoryFilePathSlot();
    void copyHistoryFilePathOfMeetFileSlot();
    void startSlot();
    void clearTabWgtSlot();
    void switchConfFileSlot();

    void openIniFileSlot();
    void modifyIniFileSlot();
    void modifyConfByVsCodeSlot();
    void openReadMeSlot();
    void delSrcFromListSlot();

    void updateSubDirHistoryFileListSlot();
    void on_createMarkdownPbn_clicked();
    void on_historySearchPbn_clicked();
    void updateRepoHistoryFileListBySearchSlot(QString txt);
    void showModifyNameDlg();

    void openCurrentDirSlot();
    void quitAppSlot();
    void switchIconSlot();
    void trayIconClickedSlot(QSystemTrayIcon::ActivationReason reason);
    void on_zoomPercentPtn_clicked();
    void addSub2RepoSlot();
    void addParent2RepoSlot();
    void addRepoSlot();
    void addAssstsDirSlot();
    void openAssstsDirSlot();
    void delCurrentRepoSlot();
    void delAssstsDirSlot();

    void openConfDirSlot();
    void modifyMarkdownSoftSlot();
    void modifyDataDirSoftSlot();

    void confDataSettingSlot();

    void exportMarkdownSlot();

    void modifyConfDlgSlot(ConfigData data);

    void createMarkdownAndSubDirSlot(int type, QString namePathAbs);

    void copyAssetsToExportDirSlot(QString oldfileAbs, QString newFileDir, QString fileName);

protected:
    // 窗口设置为随着窗口变化而变化
    virtual void resizeEvent(QResizeEvent *event) override;
    // 重写事件过滤接口
    bool eventFilter(QObject *obj, QEvent *e);

    void closeEvent(QCloseEvent *event);
public:
    void showSlot(); // 声明show函数

    void setIconByNum();
private:
    Ui::MainWindow *ui;
    AssetsData addDelListData_;
    ConfigData confData_;
    QClipboard *clip_;
    bool isIconMode_{false};
    unsigned int clickNum_{0};
    QString assetsPath_;        // Path or desktop
    QString repoPath_;          // D:/YangWeiBin/01-sync-file-level-1/02-ramses-composer
    QString oldPath_;           // E:/00-YangWeiBin/01-sync-file-level-1/02-ramses-composer
    QString subDirName_;        // 05-test
    QString fullCurrentMarkdownDirPath_;    // D:/YangWeiBin/01-sync-file-level-1/02-ramses-composer/05-test
    QString currentMarkdownFileName_;       // 63-车模2-IDC测试.md
    QString labelPath_;
    VideoThr* videoThr_;
    FileOperation fileOp_;

    QLabel* pStatusLabelIcon_;
    QLabel* pStatusLabelMsg_;
    QLabel* pStatusLabelCurrentFile_;
    OpenExProgram openExPro_;

    int screenWidth_;
    BoxSelect boxSelect_;

    GetAssetsDialog* getAssetsDialog_;
    AboutDialog* aboutDialog_;
    bool isSimpleView_{false};
    QString configFilePath_;
    QMenu *confFileList_;
    QMenu *addListMenu_;
    int addDelListMenuRow_{-1};
    QMenu *delListMenu_;
    bool isSyncStart_{false};
    QTimer *timerSync_;
    bool isStayTop_{false};

    RenameFileName* renameFileName_;
    QString clipText_;

    QSystemTrayIcon *trayIcon_;
    QMenu *trayMenu_;

    QMediaPlayer *audioPlayer_; // = new QMediaPlayer;

    ModifyConfDialog* modifyConfDlg_;
    CreateMarkdownAndSubDir* createMarkdownAndSubDirDlg_;
    GetMarkdownDialog* getMarkdownFileDig_;
    int icoNum_{0};

    int appNormalWidth_{0};
    int appNormalHeight_{0};
    int appNormalMinWidth_{0};
    int appNormalMinHeight_{0};

    int appSampleWidth_{0};
    int appSampleHeight_{0};
};

#endif // MAINWINDOW_H
