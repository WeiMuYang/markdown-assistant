﻿#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
#include <QClipboard>
#include <QDebug>
#include <QSplitter>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QDir>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QMovie>
#include <QImageReader>
#include <QScreen>
#include "debug_box.h"

int printscreeninfo()
{
    QDesktopWidget *dwsktopwidget = QApplication::desktop();
    QRect deskrect = dwsktopwidget->availableGeometry();
    QRect screenrect = dwsktopwidget->screenGeometry();
    return deskrect.width();
}

double multiple = 2;  // 1 or 12/9

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    screenWidth_ = printscreeninfo();
    videoThr_ = new VideoThr;
    audioPlayer_ = new QMediaPlayer;
    clip_ = QApplication::clipboard();
    DebugBox logBoxVideoThr;
    getAssetsDialog_ = new GetAssetsDialog(this);
    aboutDialog_ = new AboutDialog(this);
    renameFileName_ = new RenameFileName(this);
    modifyConfDlg_ = new ModifyConfDialog(this);
    createMarkdownAndSubDirDlg_ = new CreateMarkdownAndSubDir(this);
    getMarkdownFileDig_ = new GetMarkdownDialog(this);
    initScreenResNormal();
    // 0. 托盘
    initTray();
    initStatusBar();
    setWindowStyle();

    timerSync_ = new QTimer(this);
    connect(timerSync_, &QTimer::timeout, this, &MainWindow::syncAddListTimelySlot);
    timerSync_->setInterval(1500);

    ui->addList->setMovement(QListWidget::Static);
    ui->addList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->addList->setContextMenuPolicy(Qt::CustomContextMenu);
    // 拖拽调整次序
    ui->addList->setDragDropMode(QAbstractItemView::InternalMove);

    addListMenu_ = new QMenu(this);
    ui->delList->setMovement(QListWidget::Static);
    ui->delList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->delList->setContextMenuPolicy(Qt::CustomContextMenu);
    //    ui->delList->setDragDropMode(QAbstractItemView::InternalMove);
    delListMenu_ = new QMenu(this);
    QSplitter *splitterList = new QSplitter(Qt::Vertical,nullptr); // 水平布置
    splitterList->addWidget(ui->addList);
    splitterList->addWidget(ui->delList);
    ui->listLayout->addWidget(splitterList);
    splitterList->show();
    initHistoryFileList();

    ui->addList->installEventFilter(this);
    ui->delList->installEventFilter(this);
    ui->historyFileList->installEventFilter(this);

    ui->addList->setMouseTracking(true);
    // 鼠标掠过事件
    connect(ui->addList,&QListWidget::itemEntered,this, &MainWindow::itemEnteredSlot);
    connect(ui->addList,&QListWidget::clicked,this, &MainWindow::changeSelectSatusSlot);
    connect(ui->addList,&QListWidget::currentItemChanged,this, &MainWindow::itemEnteredSlot);
    ui->delList->setMouseTracking(true);
    connect(ui->delList,&QListWidget::itemEntered,this, &MainWindow::itemEnteredSlot);
    connect(ui->delList,&QListWidget::clicked,this, &MainWindow::changeSelectSatusSlot);
    connect(ui->delList,&QListWidget::currentItemChanged,this, &MainWindow::itemEnteredSlot);
    // doubleClick
    connect(ui->delList,&QListWidget::doubleClicked,this, &MainWindow::moveDelItemToAddListSlot);
    connect(ui->addList,&QListWidget::doubleClicked,this, &MainWindow::moveAddItemToDelListSlot);
    // rightClickMenu
    connect(ui->addList,&QListWidget::customContextMenuRequested,this,&MainWindow::showAddListMenuSlot);
    connect(ui->delList,&QListWidget::customContextMenuRequested,this,&MainWindow::showDelListMenuSlot);

    // Combox
    connect(ui->imgPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setImgPathSlot);
    connect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    connect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    // video
    connect(videoThr_,&VideoThr::sigCreateVideoFrame,this,&MainWindow::setImageToLabelSlot);
    connect(videoThr_,&VideoThr::sigLogBoxMsg,&logBoxVideoThr,&DebugBox::debugBoxSlot,Qt::DirectConnection);
    // log
    connect(videoThr_,&VideoThr::sigVideoThrLogText,this,&MainWindow::appendTextToLog);
    connect(&fileOp_,&FileOperation::sigFileOperationLog,this,&MainWindow::appendTextToLog);
    connect(getAssetsDialog_,&GetAssetsDialog::sigGetAssetsDlgLogText,this,&MainWindow::appendTextToLog);
    connect(&openExPro_,&OpenExProgram::sigOpenExProLog,this,&MainWindow::appendTextToLog);
    connect(renameFileName_,&RenameFileName::sigRenameFileNameLog,this,&MainWindow::appendTextToLog);
    connect(createMarkdownAndSubDirDlg_,&CreateMarkdownAndSubDir::sigCreateMarkdownAndDirLog,this,&MainWindow::appendTextToLog);
    // sigModifyConfDlgLog
    connect(modifyConfDlg_,&ModifyConfDialog::sigModifyConfDlgLog,this,&MainWindow::appendTextToLog);
    connect(modifyConfDlg_,&ModifyConfDialog::sigModifyConfigData,this,&MainWindow::modifyConfDlgSlot);
    // dialog --> fileOp
    connect(getAssetsDialog_,&GetAssetsDialog::sigSearchMarkdownCode,this,&MainWindow::searchAssetsByCodeSlot);
    // historyLineEdit
    connect(ui->historySearchLineEditor, &QLineEdit::returnPressed, this, &MainWindow::on_historySearchPbn_clicked);

    // history List click
    //    connect(ui->historyFileList,&QTableWidget::itemClicked,this,&MainWindow::ChangeToHistoryFile);
    connect(ui->historyFileList,&QTableWidget::itemDoubleClicked,this,&MainWindow::OpenHistoryFile);
    // RenameFileName --> OpenVScode
    connect(renameFileName_,&RenameFileName::sigRenameFileVSCodeOpenList,this,&MainWindow::CompareRenameFileList);
    // RenameFileName --> OpenExplore
    connect(renameFileName_,&RenameFileName::sigRenameFileOpenPath,this,&MainWindow::OpenRenameDirPath);
    // RenameFileName --> OpenrenameConf
    connect(renameFileName_,&RenameFileName::sigRenameFileConfFile,[this](QString path) {
        openExPro_.OpenJsonAndIniSlot(path);
    });
    connect(renameFileName_,&RenameFileName::sigRenameFileReferFile,[this](QString path) {
        openExPro_.OpenMarkdownAndDirSlot(path);
    });
    // createMarkdownDlg
    connect(createMarkdownAndSubDirDlg_,&CreateMarkdownAndSubDir::sigCreateType,this,&MainWindow::createMarkdownAndSubDirSlot);
    // createMarkdownDlg
    connect(createMarkdownAndSubDirDlg_,&CreateMarkdownAndSubDir::sigOpenTempleMarkdown,[this](QString path) {
        openExPro_.OpenMarkdownAndDirSlot(path);
    });
    connect(getMarkdownFileDig_,&GetMarkdownDialog::sigOpenTempleMarkdown,[this](QString path) {
        openExPro_.OpenMarkdownAndDirSlot(path);
    });

    connect(getMarkdownFileDig_,&GetMarkdownDialog::sigGetMarkdownFileLog,this,&MainWindow::appendTextToLog);
    connect(getMarkdownFileDig_,&GetMarkdownDialog::sigCopyAssetsToExportDir,this,&MainWindow::copyAssetsToExportDirSlot);

    InitMainWindowMenu();
    initAddDelListMenu();
    startSlot();
}

void MainWindow::initTray() {
    trayIcon_ = new QSystemTrayIcon(this);
    trayIcon_->setIcon(QIcon(":/qss/icon/markdown-assistant.ico"));
    trayMenu_ = new QMenu(this);
    QAction *showAction = new QAction("显         示", this);
    QAction *switchIconAction = new QAction("切 换 图 标", this);
    QAction *quitAction = new QAction("退         出", this);
    trayMenu_->addAction(showAction);
    trayMenu_->addAction(switchIconAction);
    trayMenu_->addAction(quitAction);
    trayIcon_->setContextMenu(trayMenu_);
    connect(showAction, &QAction::triggered, this, &MainWindow::showSlot);
    connect(switchIconAction, &QAction::triggered, this, &MainWindow::switchIconSlot);
    connect(quitAction, &QAction::triggered, this, &MainWindow::quitAppSlot);
    connect(trayIcon_, &QSystemTrayIcon::activated, this, &MainWindow::trayIconClickedSlot);
    trayIcon_->show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon_->isVisible()) {
        hide();
        event->ignore();
    }
}

void MainWindow::modifyConfDlgSlot(ConfigData data) {
    confData_ = data;
    confData_.writeConfJson();
    updateDataAndWidget();
}

void MainWindow::showSlot()
{
    QMainWindow::show();
}

void MainWindow::quitAppSlot() {
    QApplication::quit();
}

bool MainWindow::isIconFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.exists()) {
        return false;
    }
    QFileInfo fileInfo(file);
    return fileInfo.suffix().toLower() == "ico"; // 判断扩展名是否为 ico
}

void MainWindow::setIconByConf() {
    QIcon icon;
    QString currentIcon;
    if(icoNum_ != 0 || !isIconFile(confData_.getIconFilePath())) {
        currentIcon = ":/qss/icon/markdown-assistant.ico";
    }else {
        currentIcon = confData_.getIconFilePath();
    }
    icon.addFile(currentIcon);

    trayIcon_->setIcon(icon);
    QString log = QString("当前图标：") + currentIcon;
    ui->logText->append(log);
    this->setWindowIcon(icon);
}

void MainWindow::switchIconSlot() {
    icoNum_++;
    icoNum_ = icoNum_ % 2;
    setIconByConf();
}

void MainWindow::trayIconClickedSlot(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        showSlot();
    } else if (reason == QSystemTrayIcon::DoubleClick) {
        // 双击操作
        showSlot();
    }
}

void MainWindow::startSlot() {
    confData_.clearAll();
    if(!confData_.readIniFile()){
        appendTextToLog(QString("iniFile读取失败 !"));
    }
    updateActionConfFileList();
    // 根据用户名设置路径
    setConfigFilePathByUserName(confData_.getIniFile());
    // 必须初始化StatusBar，否则第一个菜单栏的菜单单击不到
    setStatusBar("",false);
    updateConfFileSlot();
    ui->tabWgt->setCurrentIndex(0);
}

void MainWindow::initHistoryFileList() {
    ui->historyFileList->setColumnCount(2);     //设置列数
    //    ui->historyFileList->setRowCount(40);        //设置行数/
    ui->historyFileList->setSelectionBehavior(QAbstractItemView::SelectRows);
    //设置每行内容不可编辑
    ui->historyFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    /*去掉每行的行号*/
    QHeaderView *headerView = ui->historyFileList->verticalHeader();
    headerView->setHidden(true);
    ui->historyFileList->horizontalHeader()->setStretchLastSection(true);
    // 消除表格控件的边框
    ui->historyFileList->setFrameShape(QFrame::NoFrame);
    //设置表格不显示格子线
    ui->historyFileList->setShowGrid(false); //设置不显示格子线
    // 不显示表头
    ui->historyFileList->horizontalHeader()->setHidden(true);
    ui->historyFileList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::initAddDelListMenu() {
    QAction *actDelAddList = new QAction("删除资源",addListMenu_);
    QAction *actMoveAddList = new QAction("移除资源",addListMenu_);
    QAction *actClearAddList = new QAction("删除全部",addListMenu_);
    QAction *actSelectAllAddList = new QAction("选中全部",addListMenu_);
    QAction *actCancelSelectAddList = new QAction("取消选中",addListMenu_);
    addListMenu_->addAction(actDelAddList);
    addListMenu_->addAction(actMoveAddList);
    addListMenu_->addAction(actClearAddList);
    addListMenu_->addAction(actSelectAllAddList);
    addListMenu_->addAction(actCancelSelectAddList);
    connect(actDelAddList, &QAction::triggered, this, &MainWindow::delFromAddListSlot);
    connect(actMoveAddList, &QAction::triggered, this, &MainWindow::moveFromAddListSlot);
    connect(actClearAddList, &QAction::triggered, this, &MainWindow::clearFromAddListSlot);
    connect(actSelectAllAddList, &QAction::triggered,[this](){ ui->addList->selectAll(); });
    connect(actCancelSelectAddList, &QAction::triggered,[this](){ ui->addList->clearSelection();});

    QAction *actDelDelList = new QAction("删除资源",this);
    QAction *actMoveDelList = new QAction("移除资源",this);
    QAction *actClearDelList = new QAction("删除全部",this);
    QAction *actSelectAllDelList = new QAction("选中全部",addListMenu_);
    QAction *actCancelSelectDelList = new QAction("取消选中",addListMenu_);
    QAction *actClipDelList = new QAction("剪切资源",this);
    delListMenu_->addAction(actDelDelList);
    delListMenu_->addAction(actMoveDelList);
    delListMenu_->addAction(actClearDelList);
    delListMenu_->addAction(actSelectAllDelList);
    delListMenu_->addAction(actCancelSelectDelList);
    delListMenu_->addAction(actClipDelList);
    connect(actDelDelList, &QAction::triggered, this, &MainWindow::delFromDelListSlot);
    connect(actMoveDelList, &QAction::triggered, this, &MainWindow::moveFromDelListSlot);
    connect(actClearDelList, &QAction::triggered, this, &MainWindow::clearFromDelListSlot);
    connect(actClipDelList, &QAction::triggered, this, &MainWindow::clipFromDelListSlot);
    connect(actSelectAllDelList, &QAction::triggered,[this](){ ui->delList->selectAll(); });
    connect(actCancelSelectDelList, &QAction::triggered,[this](){ ui->delList->clearSelection();});
}

void MainWindow::copyHistoryFilePathSlot()
{
    QList<QTableWidgetItem*> list = ui->historyFileList->selectedItems();
    if(list.isEmpty()){
        return;
    }
    QString path =list.at(0)->text();
    QString filePath;
    QDir curDir(fullCurrentMarkdownDirPath_);
    curDir.setCurrent(fullCurrentMarkdownDirPath_);
    QFile file(repoPath_ + "/" + path);
    if(file.exists()){
        filePath = curDir.relativeFilePath(repoPath_ +"/" + path);
    }else{
        oldPath_ = confData_.getOldPathByKey(ui->tarPathCombox->currentText());
        filePath = curDir.relativeFilePath(oldPath_ + "/" + path);
    }
    QString FileName = path.split("/").last();
    int start = FileName.indexOf("-");
    int num = FileName.lastIndexOf(".") - start - 1;
    QString clipFileName = FileName.mid(start + 1, num);
    QString text = "[" + clipFileName + "]("+filePath+")  \n\n";
    clip_->setText(text);
    appendTextToLog(u8"剪切: \"" + QString::fromStdString("[") + clipFileName + "]("+filePath+")" +  "\" 完成 !");
}

void MainWindow::copyHistoryFilePathOfMeetFileSlot()
{
    QList<QTableWidgetItem*> list = ui->historyFileList->selectedItems();
    if(list.isEmpty() || confData_.getMeetFilePath().isEmpty()){
        return;
    }
    QString path =list.at(0)->text();
    QDir curDir(confData_.getMeetFilePath());
    curDir.setCurrent(fullCurrentMarkdownDirPath_);
    QString filePath = curDir.relativeFilePath(repoPath_ + "/" + path);
    //
    QString FileName = path.split("/").last();
    int start = FileName.indexOf("-");
    int num =FileName.lastIndexOf(".") - start - 1;
    QString clipFileName =FileName.mid(start + 1, num);
    QString text = "[" + clipFileName + "]("+filePath+")  \n\n";
    clip_->setText(text);
    appendTextToLog(u8"剪切: \"" + QString::fromStdString("[") + clipFileName + "]("+filePath+")" +  "\" 完成 !");
}

void MainWindow::updateConfFileSlot()
{
    if(configFilePath_.isEmpty()){
        appendTextToLog(QString("请选择配置文件 !"));
        return;
    }
    disconnect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    disconnect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    ui->subPathComBox->clear();
    ui->tarPathCombox->clear();
    connect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    connect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    confData_.readConf(configFilePath_);
    openExPro_.setMarkdownSoftWarePath(confData_.getMarkdownSoftPath());
    openExPro_.setDataDirSoftWarePath(confData_.getDataDirSoftPath());
    addDelListData_.setAssetsTypes(confData_.getAssetsTypes());
    fileOp_.setAssetsTypes(confData_.getAssetsTypes());
    updateDataAndWidget();
}

void MainWindow::updateDataAndWidget(){
    // 1 Init Path
    initImgPathTarPathCombox();
    // 2 更新 子目录
    updateSubDirCombox();
    // 3 更新 列表数据和界面
    updateListDataAndWgtSlot();
    // 4 更新 最新的修改文件
    updateLastModifyFile();
    // 5 更新 Top20文件列表
    updateRepoHistoryFileList();
}

void MainWindow::updateRepoHistoryFileList(){
    QFileInfoList fileListTop20;
    ui->historyFileList->clear();
    fileOp_.getHistoryFileList(repoPath_, fileListTop20);
    ui->historyFileList->setRowCount(fileListTop20.size());        //设置行数/
    for(int i = 0; i < fileListTop20.size(); ++i){
        QFileInfo fileInfo = fileListTop20.at(i);
        QString name = fileInfo.filePath().split(repoPath_).last().remove(0,1);
        QString modifyTime = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss ddd");
        QTableWidgetItem *pItem1 = new QTableWidgetItem(name);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(modifyTime);
        //        pItem2->setTextAlignment(Qt::AlignCenter);
        ui->historyFileList->setItem(i, 0, pItem1);
        ui->historyFileList->setItem(i, 1, pItem2);
    }
    ui->historyFileList->setColumnWidth(0, 1300 * 0.66);
    ui->historyFileList->setColumnWidth(1, 1300 * 0.33);
}

void MainWindow::delSrcFromListSlot() {
    bool selectDel = false;
    for (int i = 0; i < ui->addList->count(); ++i) {
        if(ui->addList->item(i)->isSelected()){
            QString oldName = ui->addList->item(i)->text();
            fileOp_.delDesktopFile(getAssetsPath(), oldName);
            // 2. del ui
            // 注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
            ui->addList->takeItem(i--);
            // 3. del data
            ImgData data;
            addDelListData_.delAddImageListByOldName(oldName, data);
            selectDel = true;
        }
    }

    for (int i = 0; i < ui->delList->count(); ++i) {
        if(ui->delList->item(i)->isSelected()){
            QString oldName = ui->delList->item(i)->text();
            //            qDebug() <<ui->delList->item(i)->text();
            fileOp_.delDesktopFile(getAssetsPath(), oldName);
            // 2. del ui
            //注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
            ui->delList->takeItem(i--);
            // 3. del data
            ImgData data;
            addDelListData_.delDelImageListByOldName(oldName, data);
            selectDel = true;
        }
    }
    if(selectDel){
        ui->imgLabel->setPixmap(QString(""));
    }
}

void MainWindow::updateSubDirHistoryFileListSlot() {
    QFileInfoList fileListTop20;
    ui->historyFileList->clear();
    fileOp_.getHistorySubDirFileList(repoPath_ + "/" + subDirName_, fileListTop20);
    ui->historyFileList->setRowCount(fileListTop20.size());        //设置行数/
    for(int i = 0; i < fileListTop20.size(); ++i){
        QFileInfo fileInfo = fileListTop20.at(i);
        QString name = fileInfo.filePath().split(repoPath_).last().remove(0,1);
        QString modifyTime = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss ddd");
        QTableWidgetItem *pItem1 = new QTableWidgetItem(name);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(modifyTime);
        ui->historyFileList->setItem(i, 0, pItem1);
        ui->historyFileList->setItem(i, 1, pItem2);
    }
    ui->historyFileList->setColumnWidth(0, 1300 * 0.66);
    ui->historyFileList->setColumnWidth(1, 1300 * 0.33);
}

void MainWindow::setConfigFilePathByUserName(const IniFile& iniFile){
    QString userName = iniFile.hostName;
    bool hasUserConf = false;
    for (int i = 0; i < iniFile.recentFileList.size(); ++i) {
        if(iniFile.recentFileList.at(i).indexOf(userName) != -1){
            // todo: Lenovo  or BMW
            configFilePath_= iniFile.iniAndJsonPath+"/"+iniFile.recentFileList.at(i);
            hasUserConf = true;
            break;
        }
    }
    if(!hasUserConf){
        appendTextToLog(QString("未配置该用户文件 !"));
    }
}

void MainWindow::updateActionConfFileList()  {
    IniFile iniFile = confData_.getIniFile();
    confFileList_->clear();
    for (int i = 0; i < iniFile.recentFileList.size(); ++i) {
        QAction *actionFileName = new QAction(confFileList_);
        actionFileName->setText(iniFile.recentFileList.at(i));
        connect(actionFileName, &QAction::triggered, this, &MainWindow::switchConfFileSlot);
        confFileList_->addAction(actionFileName);
    }
    ui->menuFile->addMenu(confFileList_);
}

void MainWindow::InitMainWindowMenu(){
    // TODO： restart  about
    ui->actionGetAssets->setShortcut(QKeySequence::Find);
    ui->actionGetAssets->setShortcutContext(Qt::ApplicationShortcut);
    connect(ui->actionGetAssets, &QAction::triggered, this, &MainWindow::getAssetsSlot);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::openAboutSlot);

    ui->actionRestart->setShortcut(QKeySequence("Ctrl+R"));
    connect(ui->actionRestart,&QAction::triggered, this, &MainWindow::startSlot);

    ui->actionClearLog->setShortcut(QKeySequence::Refresh);
    ui->actionClearLog->setShortcutContext(Qt::ApplicationShortcut);
    connect(ui->actionClearLog,&QAction::triggered, this, &MainWindow::clearTabWgtSlot);

    ui->actionAddConfFile->setShortcut(QKeySequence::New);
    ui->actionAddConfFile->setShortcutContext(Qt::ApplicationShortcut);
    connect(ui->actionAddConfFile, &QAction::triggered, this, &MainWindow::newConfFileSlot);

    ui->actionModifySysFile->setShortcut(QKeySequence("Ctrl+O"));
    connect(ui->actionModifySysFile, &QAction::triggered, this, &MainWindow::modifyIniFileSlot);

    ui->actionRename->setShortcut(QKeySequence("Ctrl+F10"));
    connect(ui->actionRename, &QAction::triggered, this, &MainWindow::showModifyNameDlg);

    ui->actionStayTop->setShortcut(QKeySequence("Ctrl+F1"));
    connect(ui->actionStayTop,&QAction::triggered,[this]() {
        isStayTop_ = !isStayTop_;
        if(isStayTop_) {
            setWindowFlag(Qt::WindowStaysOnTopHint, true);
            showSlot();
        }else {
            setWindowFlag(Qt::WindowStaysOnTopHint, false);
            showSlot();
        }
    });

    if(isSimpleView_){
        setSampleView();
        ui->actionSimpleView->setText(u8"正常窗口");
        ui->actionSimpleView->setShortcut(QKeySequence("Ctrl+="));
    }else{
        setNormalView();
        ui->actionSimpleView->setText(u8"极简窗口");
        ui->actionSimpleView->setShortcut(QKeySequence("Ctrl+-"));
    }
    connect(ui->actionSimpleView, &QAction::triggered, this, &MainWindow::simpleViewSlot);

    ui->actionModifyConf->setShortcut(QKeySequence("Ctrl+M"));
    connect(ui->actionModifyConf, &QAction::triggered, this, &MainWindow::modifyConfByVsCodeSlot);

    ui->actionCopyHistoryFilePath->setShortcut(QKeySequence("Ctrl+C"));
    connect(ui->actionCopyHistoryFilePath,&QAction::triggered, this, &MainWindow::copyHistoryFilePathSlot);

    ui->actionSubDirHistoryFile->setShortcut(QKeySequence("Ctrl+L"));
    connect(ui->actionSubDirHistoryFile,&QAction::triggered, this, &MainWindow::updateSubDirHistoryFileListSlot);

    ui->actionGetRelativePathOfMeetFile->setShortcut(QKeySequence("Ctrl+P"));
    connect(ui->actionGetRelativePathOfMeetFile,&QAction::triggered, this, &MainWindow::copyHistoryFilePathOfMeetFileSlot);

    confFileList_ = new QMenu;
    confFileList_->setTitle(u8"配置文件列表");

    ui->actionOpenREADME->setShortcut(QKeySequence("F1"));
    connect(ui->actionOpenREADME, &QAction::triggered, this, &MainWindow::openReadMeSlot);

    ui->actionDelFromSrcList->setShortcut(QKeySequence::Delete);
    connect(ui->actionDelFromSrcList, &QAction::triggered, this, &MainWindow::delSrcFromListSlot);

    // 打开当前子目录或仓库目录
    ui->actionOpenCurrentDir->setShortcut(QKeySequence("Alt+D"));
    connect(ui->actionOpenCurrentDir, &QAction::triggered, this, &MainWindow::openCurrentDirSlot);
    //
    ui->actionSub2Repo->setShortcut(QKeySequence("Alt+Left"));
    connect(ui->actionSub2Repo, &QAction::triggered, this, &MainWindow::addSub2RepoSlot);

    ui->actionParent2Repo->setShortcut(QKeySequence("Alt+Right"));
    connect(ui->actionParent2Repo, &QAction::triggered, this, &MainWindow::addParent2RepoSlot);

    ui->actionDelRepo->setShortcut(QKeySequence("Alt+Delete"));
    connect(ui->actionDelRepo, &QAction::triggered, this, &MainWindow::delCurrentRepoSlot);

    ui->actionOpenConfDir->setShortcut(QKeySequence("Ctrl+/"));
    connect(ui->actionOpenConfDir, &QAction::triggered, this, &MainWindow::openConfDirSlot);

    ui->actionAddMarkDownSoftWare->setShortcut(QKeySequence("Alt+F"));
    connect(ui->actionAddMarkDownSoftWare, &QAction::triggered, this, &MainWindow::modifyMarkdownSoftSlot);

    ui->actionAddVSCodePath->setShortcut(QKeySequence("Alt+V"));
    connect(ui->actionAddVSCodePath, &QAction::triggered, this, &MainWindow::modifyDataDirSoftSlot);

    ui->actionAddRepo->setShortcut(QKeySequence("Alt+N"));
    connect(ui->actionAddRepo, &QAction::triggered, this, &MainWindow::addRepoSlot);

    //    ui.actionaddAssetsDir
    ui->actionaddAssetsDir->setShortcut(QKeySequence("Alt+A"));
    connect(ui->actionaddAssetsDir, &QAction::triggered, this, &MainWindow::addAssstsDirSlot);

    //  actionOpenAssetsDir
    ui->actionOpenAssetsDir->setShortcut(QKeySequence("Alt+O"));
    connect(ui->actionOpenAssetsDir, &QAction::triggered, this, &MainWindow::openAssstsDirSlot);

    // actionDelCurrentAssetsDir
    //    ui->actionOpenAssetsDir->setShortcut(QKeySequence("Alt+D"));
    connect(ui->actionDelCurrentAssetsDir, &QAction::triggered, this, &MainWindow::delAssstsDirSlot);

    // actionConfdata
    ui->actionConfdata->setShortcut(QKeySequence("Alt+M"));
    connect(ui->actionConfdata, &QAction::triggered, this, &MainWindow::confDataSettingSlot);

    // Export Markdown File
    ui->actionExportMarkdown->setShortcut(QKeySequence("Alt+E"));
    connect(ui->actionExportMarkdown, &QAction::triggered, this, &MainWindow::exportMarkdownSlot);
}

void MainWindow::switchConfFileSlot(){
    QAction *action=qobject_cast<QAction *>(sender());
    QString fileName = confData_.getIniFile().iniAndJsonPath + "/"+ action->text();
    configFilePath_ = fileName;
    QFileInfo configFile(configFilePath_);
    confData_.setIniFileHostName(configFile.baseName());
    confData_.writeIniFile();
    startSlot();
}

void MainWindow::clearTabWgtSlot()
{
    if(ui->tabWgt->currentIndex() == 0) {
        ui->logText->clear();
    }else{
        updateRepoHistoryFileList();
    }
}

void MainWindow::openIniFileSlot()
{
    openExPro_.OpenMarkdownAndDirSlot(confData_.getIniFile().iniAndJsonPath+ "/conf.ini");
}

void MainWindow::modifyIniFileSlot(){
    openExPro_.OpenJsonAndIniSlot(confData_.getIniFile().iniAndJsonPath+ "/conf.ini");
    startSlot();
}

void MainWindow::modifyConfByVsCodeSlot()
{
    openExPro_.OpenJsonAndIniSlot(configFilePath_);
    updateConfFileSlot();
}

void MainWindow::openReadMeSlot()
{
    QString readMeFilePath;
    if(fileOp_.getReadMePath(repoPath_,readMeFilePath)){
        openExPro_.OpenMarkdownAndDirSlot(readMeFilePath);
        appendTextToLog(QString("打开ReadMe文件: " + readMeFilePath +" 成功 !"));
    }else{
        appendTextToLog(QString("未找到ReadMe文件 !"));
    }
}

void MainWindow::initStatusBar(){
    pStatusLabelIcon_ = new QLabel("",this);
    pStatusLabelMsg_ = new QLabel("",this);
    pStatusLabelMsg_->setAlignment(Qt::AlignLeft);
    pStatusLabelCurrentFile_ = new QLabel("",this);
    pStatusLabelCurrentFile_->setAlignment(Qt::AlignHCenter);
}

void MainWindow::setInitNormalViewByScreenRes(){
    QScreen *screen = QGuiApplication::primaryScreen();
    qreal logicalDpi = screen->logicalDotsPerInch();
    double zoom = 1;
    if(logicalDpi > 150) {
        zoom = logicalDpi / 150.0;
    }
    double widthIn4K = 1400;
    double heightIn4K = 1100;

    // 高宽比
    double WindowAspect = heightIn4K / widthIn4K;
    // 占屏比
    double Proportion = widthIn4K / 3840.0;
    // 宽 高
    int width = screenWidth_ * Proportion * zoom;
    int height = width * WindowAspect * zoom;
    QDesktopWidget *dwsktopwidget = QApplication::desktop();
    QRect deskrect = dwsktopwidget->availableGeometry();
    if(width >= deskrect.width()) {
        width = deskrect.width();
    }
    if(height >= deskrect.height()- 100 ) {
        height = deskrect.height() - 100 ;
    }

    appNormalWidth_ = width;
    appNormalHeight_ = height;
    appNormalMinWidth_ = width;
    appNormalMinHeight_ = height;
}

void MainWindow::setInitSampleViewByScreenRes(){

    float zoom = 1;
    if(screenWidth_ < 3840) {
        zoom = 1.5;
    }
    // 宽高比
    double WindowAspect = 480.0 / 880.0;
    // 占屏比
    double Proportion = 880.0 / 3840.0;
    // 宽 高
    int width = screenWidth_ * Proportion ;
    int height = width * WindowAspect;
    appSampleWidth_ = width * zoom;
    appSampleHeight_ = height * zoom;
}



void MainWindow::initScreenResNormal(){
    setInitSampleViewByScreenRes();
    setInitNormalViewByScreenRes();
    this->resize(QSize(appNormalWidth_, appNormalHeight_));
    setMinimumSize(QSize(appNormalWidth_, appNormalHeight_));
    initListWgt();
}

void MainWindow::setScreenWitdh(int w)
{
    screenWidth_ = w;
}

int MainWindow::getScreenWitdh()
{
    return screenWidth_;
}

ScreenRes MainWindow::getScrrenRes()
{
    if(screenWidth_ >= 3840){
        return ScreenRes::High;
    }else{
        return ScreenRes::Low;
    }
}

void MainWindow::setStatusBar(QString msg, bool isCorrect){
    Q_UNUSED(msg)
    if(isCorrect){
        QImage image(QString(":/qss/psblack/checkbox_checked.png"));
        pStatusLabelIcon_->setPixmap(QPixmap::fromImage(image));
        pStatusLabelIcon_->setMinimumWidth(15);
        pStatusLabelMsg_->setText(u8"正常");
        pStatusLabelCurrentFile_->setText("|  " +subDirName_ +"/"+ currentMarkdownFileName_);
    }else{
        QImage image(QString(":/qss/psblack/checkbox_checked_disable.png"));
        pStatusLabelIcon_->setPixmap(QPixmap::fromImage(image));
        pStatusLabelIcon_->setMinimumWidth(15);
        pStatusLabelMsg_->setText(u8"错误");
        pStatusLabelCurrentFile_->setText("|  " + currentMarkdownFileName_);
    }

    ui->statusBar->addWidget(pStatusLabelIcon_);
    ui->statusBar->addWidget(pStatusLabelMsg_);
    ui->statusBar->addWidget(pStatusLabelCurrentFile_);
}

void MainWindow::setWindowStyle()
{
    QFile file(":/qss/psblack.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }
}

void MainWindow::appendTextToLog(QString log)
{
    ui->logText->append(log.toUtf8());
}

// 3.
void MainWindow::updateLastModifyFile(){
    // 获取最后修改的文件序号
    int num = fileOp_.getLastmodifiedTimeFileNum(repoPath_,fullCurrentMarkdownDirPath_,currentMarkdownFileName_);
    if(num == -1){
        ui->subPathComBox->setCurrentText("no file");
        ui->numSpinBox->setValue(num);
        currentMarkdownFileName_.clear();
        appendTextToLog(QString(u8"当前的目标路径不存在 !"));
        setStatusBar("", false);
        whoIsBoxSelection(BoxSelect::SubCombox);
        QFileInfo repoInfo(repoPath_);
        this->setWindowTitle("Error - " + repoInfo.fileName() + " - [" + configFilePath_ + "]");

    }else{
        QStringList strList = fullCurrentMarkdownDirPath_.split(repoPath_);
        QString subPath =strList.last().replace("/","");
        ui->subPathComBox->setCurrentText(subPath);
        ui->numSpinBox->setValue(num);
        setStatusBar("", true);
        subDirName_ = subPath;
        whoIsBoxSelection(BoxSelect::NumSpinBox);

        QFileInfo currentInfo(currentMarkdownFileName_);
        QFileInfo repoInfo(repoPath_);
        this->setWindowTitle(currentInfo.fileName() + " - " + repoInfo.fileName() + " - [" + configFilePath_ + "]");
    }
}
// 1 Init
void MainWindow::initImgPathTarPathCombox()
{
    disconnect(ui->imgPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setImgPathSlot);
    disconnect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    disconnect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    ui->subPathComBox->clear();
    ui->tarPathCombox->clear();
    ui->imgPathCombox->clear();

    for(auto it = confData_.getAssetPaths().begin();it != confData_.getAssetPaths().end(); ++it){
        ui->imgPathCombox->addItem(it->key);
    }
    for(auto it = confData_.getRepoPaths().begin();it != confData_.getRepoPaths().end(); ++it){
        ui->tarPathCombox->addItem(it->key);
    }
    assetsPath_ = confData_.getAssetsPathByKey(ui->imgPathCombox->currentText());
    repoPath_ = confData_.getRepoPathByKey(ui->tarPathCombox->currentText());
    oldPath_ = confData_.getOldPathByKey(ui->tarPathCombox->currentText());
    connect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    connect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    connect(ui->imgPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setImgPathSlot);

}

void MainWindow::setComboBoxToolTip(QComboBox * box){
    auto itemModel = qobject_cast<QStandardItemModel*>(box->model());
    for(int i = 0; i < box->count(); ++i){
        itemModel->item(i)->setToolTip(box->itemText(i));
    }
}

void MainWindow::updateSubDirCombox(){
    disconnect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    ui->subPathComBox->clear();
    ui->subPathComBox->addItems(fileOp_.getSubDirNames(this->repoPath_));
    setComboBoxToolTip(ui->subPathComBox);
    subDirName_ = ui->subPathComBox->currentText();
    whoIsBoxSelection(BoxSelect::SubCombox);
    connect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
}

void MainWindow::setImgPathSlot(QString currentStr){
    assetsPath_ = confData_.getAssetsPathByKey(ui->imgPathCombox->currentText());
}

void MainWindow::setTarPathSlot(QString currentStr){
    this->repoPath_ = confData_.getRepoPathByKey(currentStr);
    if(repoPath_.isEmpty() || !fileOp_.isPathExist(this->repoPath_)){
        ui->subPathComBox->clear();
        fullCurrentMarkdownDirPath_.clear();
        subDirName_.clear();
        repoPath_.clear();
        ui->numSpinBox->setValue(-1);
        appendTextToLog(QString(u8"当前的目标路径不存在 !"));
        currentMarkdownFileName_.clear();
        setStatusBar("", false);
        whoIsBoxSelection(BoxSelect::None);
        return;
    }
    whoIsBoxSelection(BoxSelect::TarCombox);
    updateSubDirCombox();
    updateLastModifyFile();
    updateRepoHistoryFileList();
}

void MainWindow::setSubPathSlot(QString currentStr){
    if(currentStr.isEmpty()){
        return;
    }
    subDirName_ = currentStr;
    int num = fileOp_.getLastmodifiedTimeFileNumSubDir(repoPath_,subDirName_, fullCurrentMarkdownDirPath_,currentMarkdownFileName_);
    if(num == -1){
        //        fullTarPath_.clear();
        ui->numSpinBox->setValue(num);
        appendTextToLog(QString(u8"当前的目标路径不存在 !"));
        setStatusBar("", false);
        whoIsBoxSelection(BoxSelect::SubCombox);
    }else{
        QStringList strList = fullCurrentMarkdownDirPath_.split(repoPath_);
        QString subPathName =strList.last().replace("/","");
        ui->subPathComBox->setCurrentText(subPathName);
        ui->numSpinBox->setValue(num);
        setStatusBar("", true);
        whoIsBoxSelection(BoxSelect::NumSpinBox);
    }
}

void MainWindow::setImageToLabelSlot(QImage image){
    QPixmap map = QPixmap::fromImage(image);
    map = map.scaled(ui->imgLabel->width(), ui->imgLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->imgLabel->setAlignment(Qt::AlignCenter);
    ui->imgLabel->setPixmap(map);
}

void MainWindow::changeModeSlot()
{
    if(isIconMode_){
        // 设置显示模式
        ui->addList->setViewMode(QListView::IconMode);  // ListMode  IconMode
        // 甚至随略图的大小
        ui->addList->setIconSize(QSize(100,100*multiple));  // 100,100
        // 设置图标间距
        ui->addList->setSpacing(15);
        // 设置显示模式
        ui->delList->setViewMode(QListView::IconMode);  // ListMode  IconMode
        // 甚至随略图的大小
        ui->delList->setIconSize(QSize(100,100*multiple));  // 100,100
        // 设置图标间距
        ui->delList->setSpacing(15);
    }else{
        ui->addList->setViewMode(QListView::ListMode);
        ui->addList->setIconSize(QSize(40 ,20*multiple));
        ui->addList->setSpacing(4);

        ui->delList->setViewMode(QListView::ListMode);
        ui->delList->setIconSize(QSize(40 ,20*multiple));
        ui->delList->setSpacing(4);
    }
    appendTextToLog(u8"修改显示模式完毕 !");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateListDataAndWgtSlot(){
    videoThr_->stop();
    addDelListData_.clearAddList();
    addDelListData_.clearDelList();
    QString path = getAssetsPath();

    // 更新数据层 data
    addDelListData_.updateImgVideoFile(path, ui->numSpinBox->value());// ui->numEdit->text().toInt());

    // 更新界面层
    updateListWgt();

    ui->imgLabel->setPixmap(QString(""));
    appendTextToLog(u8"更新文件列表完毕 !");
}

void MainWindow::initListWgt(){
    QFont font;
    if(getScrrenRes() == ScreenRes::High){
        multiple = 2;
    }else{
        multiple = 1;
    }
}

void MainWindow::changedPercentByLabelImg(int width) {
    if(width == AssetsZoomSize::Size100) {
        ui->RadioBtn100->setChecked(true);
    }else if(width == AssetsZoomSize::Size80) {
        ui->RadioBtn80->setChecked(true);
    }else if(width == AssetsZoomSize::Size50) {
        ui->RadioBtn50->setChecked(true);
    }else if(width == AssetsZoomSize::Size30) {
        ui->RadioBtn30->setChecked(true);
    }else {
        ui->RadioBtn5->setChecked(true);
    }
}

void MainWindow::setItemIcon(const ImgData& data, QListWidgetItem* pItem) {
    if(data.oldName.right(4) == ".mp3") {
        pItem->setIcon(QIcon(":/qss/icon/mp3.ico"));
    }else if(data.oldName.right(4) == ".mp4"){
        QIcon videoIcon;
        if(videoThr_->getFirstVideoFrame(data.oldPath, videoIcon)){
            pItem->setIcon(videoIcon);
        }else{
            pItem->setIcon(QIcon(":/qss/icon/markdown-assistant.ico"));
        }
    }else {
        pItem->setIcon(QIcon(data.oldPath));
    }

    if(isIconMode_){
        pItem->setSizeHint(QSize(200,100 * multiple));
        pItem->setText(data.oldName);
        pItem->setToolTip(data.oldName);
    }else{
        pItem->setSizeHint(QSize(20,20 * multiple));
        pItem->setText(data.oldName);
        pItem->setToolTip(data.oldName);
    }
}

void MainWindow::updateListWgt()
{
    ui->addList->clear();
    ui->delList->clear();
    for(int i = 0; i < addDelListData_.getAddList().size(); ++i){
        QListWidgetItem *pItem = new QListWidgetItem;
        ImgData data = addDelListData_.getAddList().at(i);
        setItemIcon(data, pItem);
        ui->addList->addItem(pItem);
    }
    for(int i = 0; i < addDelListData_.getDelList().size(); ++i){
        QListWidgetItem *pItem = new QListWidgetItem;
        ImgData data = addDelListData_.getDelList().at(i);
        setItemIcon(data, pItem);
        ui->delList->addItem(pItem);
    }
}

void MainWindow::on_refreshPbn_clicked()
{
    updateListDataAndWgtSlot();
}

void MainWindow::on_modePbn_clicked()
{
    clickNum_++;
    if(clickNum_ % 2 == 0){
        ui->modePbn->setText(u8"图标");
        isIconMode_ = false;
    }else {
        ui->modePbn->setText(u8"列表");
        isIconMode_ = true;
    }
    changeModeSlot();
    updateListDataAndWgtSlot();
}

void MainWindow::changeSelectSatusSlot() {
    QObject* senderObj = sender(); // 获取发信号的对象
    if (senderObj){
        QListWidget* listWgt =dynamic_cast<QListWidget *>(senderObj);
        listWgt->setSelectionMode(QAbstractItemView::MultiSelection);
    }
}

void MainWindow::playAudioMp3(const QString& path) {
    audioPlayer_->setMedia(QUrl::fromLocalFile(path));
    audioPlayer_->setVolume(50); // 设置音量
    audioPlayer_->play(); // 开始播放
}

void MainWindow::itemEnteredSlot(QListWidgetItem *item)
{
    if(item == Q_NULLPTR){
        //        qDebug() << "改变Item后，双击操作，清除了之前的item ";
        return;
    }
    QString name = item->text();
    ImgData data = addDelListData_.matchOldName(name);
    labelPath_ = data.oldPath;
    ui->imgLabel->setToolTip(QString::number(data.widthZoom) + " %");
    changedPercentByLabelImg(data.widthZoom);
    if(name.right(4) == ".mp4") {
        audioPlayer_->stop();
        videoThr_->setVideoPath(labelPath_);
        videoThr_->play();
    }else if(name.right(4) == ".mp3") {
        QMovie *movie = new QMovie(":/qss/icon/audio.gif");
        ui->imgLabel->setMovie(movie);
        ui->imgLabel->setScaledContents(false);
        adjustMovieSize(movie, ui->imgLabel->size());
        movie->start();
        //        playAudioMp3(labelPath_);
    }else{
        audioPlayer_->stop();
        videoThr_->stop();
        // 从文件中加载图片到标签
        if(!labelPath_.isEmpty()){
            QPixmap map = QPixmap(labelPath_);
            map = map.scaled(ui->imgLabel->width(), ui->imgLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->imgLabel->setAlignment(Qt::AlignCenter);
            ui->imgLabel->setPixmap(map);
        }else{
            // 路径为空，需要添加一个默认图片
            QPixmap map = QPixmap(":/qss/icon/do-not-exist.png");
            map = map.scaled(ui->imgLabel->width(), ui->imgLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->imgLabel->setAlignment(Qt::AlignCenter);
            ui->imgLabel->setPixmap(map);
            ui->imgLabel->setToolTip("图片不存在!");
            ui->RadioBtn100->setChecked(false);
            ui->RadioBtn80->setChecked(false);
            ui->RadioBtn50->setChecked(false);
            ui->RadioBtn30->setChecked(false);
            ui->RadioBtn5->setChecked(false);
        }
    }
}

void MainWindow::sycImgDataByOldName(QString oldName){
    ImgData data;
    bool sucess = false;
    if(addDelListData_.delAddImageListByOldName(oldName, data)){
        if(!addDelListData_.insertDataDelImageList(data)){
            DebugBox(__FUNCTION__, __LINE__,"add DelImageList error");
        }
        sucess = true;
    }
    if(addDelListData_.delDelImageListByOldName(oldName, data)){
        if(!addDelListData_.insertDataAddImageList(data)){
            DebugBox(__FUNCTION__, __LINE__,"add AddImageList error");
        }
        sucess = true;
    }
    if(!sucess){
        DebugBox(__FUNCTION__, __LINE__, "sync error");
    }
}

void MainWindow::writeCurrentFile(QString str) {
    QFile file(fullCurrentMarkdownDirPath_+ "/" + currentMarkdownFileName_);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream stream(&file);
        // 将文件光标移动到文件的末尾
        stream.seek(file.size());
        // 将QString写入文件
        stream << str << "\n";
        file.close();
    }
}

void MainWindow::syncAddListTimelySlot() {
    QVector<ImgData> srcList = addDelListData_.getNewAddImgVideoFile(getAssetsPath());
    for(auto it = srcList.begin(); it < srcList.end(); ++it){
        ImgData data = *it;
        addDelListData_.insertDataAddImageList(data);
        QListWidgetItem *pItem = new QListWidgetItem;

        setItemIcon(data, pItem);

        if(isIconMode_){
            pItem->setSizeHint(QSize(200,100*multiple));
            pItem->setText(data.oldName);
            pItem->setToolTip(data.oldName);
        }else{
            pItem->setSizeHint(QSize(20,20*multiple));
            pItem->setText(data.oldName);
            pItem->setToolTip(data.oldName);
        }
        ui->addList->addItem(pItem);
        appendTextToLog(u8"自动添加: " + data.oldName + " 成功 ！");
    }
}

void MainWindow::on_syncPbn_clicked()
{
    isSyncStart_ = (!isSyncStart_);
    if(isSyncStart_){
        timerSync_->start();
        appendTextToLog(QDateTime::currentDateTime().toString("[HH:mm:ss]") + u8" 启动自动同步 !");
        ui->syncPbn->setStyleSheet("border: 1px solid #d9d5d2");
    }else{
        timerSync_->stop();
        appendTextToLog(QDateTime::currentDateTime().toString("[HH:mm:ss]") + u8" 关闭自动同步 !");
        ui->syncPbn->setStyleSheet("");
    }

}

// 双击移动 DelImageList -->  AddImageList
void MainWindow::moveDelItemToAddListSlot(const QModelIndex &index)
{
    Q_UNUSED(index);
    qDebug() << ui->delList->currentItem()->text();

    QString oldName = ui->delList->currentItem()->text();
    ImgData data;
    addDelListData_.delDelImageListByOldName(oldName, data);
    addDelListData_.insertDataAddImageList(data);
    updateListWgt();
    appendTextToLog(u8"移动\""+ oldName + u8"\"文件完毕 !");
}
// 双击移动 AddImageList -->  DelImageList
void MainWindow::moveAddItemToDelListSlot(const QModelIndex &index)
{
    Q_UNUSED(index);
    QString oldName = ui->addList->currentItem()->text();
    ImgData data;
    addDelListData_.delAddImageListByOldName(oldName, data);
    addDelListData_.insertDataDelImageList(data);
    updateListWgt();
    appendTextToLog(u8"移动\""+ oldName + u8"\"文件完毕 !");
}
void MainWindow::showAddListMenuSlot(QPoint pos){
    addListMenu_->move(cursor().pos());
    addListMenu_->show();
    int x = pos.x();
    int y = pos.y();
    QModelIndex index = ui->addList->indexAt(QPoint(x,y));
    addDelListMenuRow_ = index.row();//获得QTableWidget列表点击的行数
}
void MainWindow::delFromAddListSlot() {
    bool isSelectDel = false;
    for (int i = 0; i < ui->addList->count(); ++i) {
        if(ui->addList->item(i)->isSelected()){
            QString oldName = ui->addList->item(i)->text();
            fileOp_.delDesktopFile(getAssetsPath(), oldName);
            // 2. del ui
            // 注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
            ui->addList->takeItem(i--);
            // 3. del data
            ImgData data;
            addDelListData_.delAddImageListByOldName(oldName, data);
            isSelectDel = true;
        }
    }
    if(isSelectDel) {
        ui->imgLabel->setPixmap(QString(""));
    }else if(addDelListMenuRow_ > -1 && addDelListMenuRow_ < ui->addList->count()) {
        // 1. del file
        QString oldName = ui->addList->item(addDelListMenuRow_)->text();
        fileOp_.delDesktopFile(getAssetsPath(), oldName);
        // 2. del ui
        // 注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
        ui->addList->takeItem(addDelListMenuRow_);
        // 3. del data
        ImgData data;
        addDelListData_.delAddImageListByOldName(oldName, data);
        ui->imgLabel->setPixmap(QString(""));
    }
    addDelListMenuRow_ = -1;
}

void MainWindow::moveFromAddListSlot() {
    if(addDelListMenuRow_ > -1 && addDelListMenuRow_ < ui->addList->count()) {
        QString oldName = ui->addList->item(addDelListMenuRow_)->text();
        ImgData data;
        addDelListData_.delAddImageListByOldName(oldName, data);
        addDelListData_.insertDataDelImageList(data);
        updateListWgt();
        appendTextToLog(u8"移动\""+ oldName + u8"\"文件完毕 !");
    }
    addDelListMenuRow_ = -1;
}

void MainWindow::clearFromAddListSlot(){
    for (int i = 0; i < ui->addList->count(); ++i) {
        QString oldName = ui->addList->item(i)->text();
        fileOp_.delDesktopFile(getAssetsPath(), oldName);
        // 2. del ui
        // 注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
        ui->addList->takeItem(i--);
        // 3. del data
        ImgData data;
        addDelListData_.delAddImageListByOldName(oldName, data);
        ui->imgLabel->setPixmap(QString(""));
    }
}

void MainWindow::showDelListMenuSlot(QPoint pos){
    delListMenu_->move(cursor().pos());
    delListMenu_->show();
    int x = pos.x();
    int y = pos.y();
    QModelIndex index = ui->delList->indexAt(QPoint(x,y));
    addDelListMenuRow_ = index.row();//获得QTableWidget列表点击的行数
}

void MainWindow::delFromDelListSlot() {
    bool isSelectDel = false;
    for (int i = 0; i < ui->delList->count(); ++i) {
        if(ui->delList->item(i)->isSelected()){
            QString oldName = ui->delList->item(i)->text();
            fileOp_.delDesktopFile(getAssetsPath(), oldName);
            // 2. del ui
            //注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
            ui->delList->takeItem(i--);
            // 3. del data
            ImgData data;
            addDelListData_.delDelImageListByOldName(oldName, data);
            isSelectDel = true;
        }
    }
    if(isSelectDel) {
        ui->imgLabel->setPixmap(QString(""));
    }else if(addDelListMenuRow_ > -1 && addDelListMenuRow_ < ui->delList->count()) {
        // 1. del file
        QString oldName = ui->delList->item(addDelListMenuRow_)->text();
        fileOp_.delDesktopFile(getAssetsPath(), oldName);
        // 2. del ui
        //注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
        ui->delList->takeItem(addDelListMenuRow_);
        // 3. del data
        ImgData data;
        addDelListData_.delDelImageListByOldName(oldName, data);
        ui->imgLabel->setPixmap(QString(""));
    }
    addDelListMenuRow_ = -1;
}

void MainWindow::moveFromDelListSlot() {
    if(addDelListMenuRow_ > -1 && addDelListMenuRow_ < ui->delList->count()) {
        QString oldName = ui->delList->item(addDelListMenuRow_)->text();
        ImgData data;
        addDelListData_.delDelImageListByOldName(oldName, data);
        addDelListData_.insertDataAddImageList(data);
        updateListWgt();
        appendTextToLog(u8"移动\""+ oldName + u8"\"文件完毕 !");
    }
    addDelListMenuRow_ = -1;
}

void MainWindow::clearFromDelListSlot() {
    for (int i = 0; i < ui->delList->count(); ++i) {
        QString oldName = ui->delList->item(i)->text();
        fileOp_.delDesktopFile(getAssetsPath(), oldName);
        // 2. del ui
        // 注意：删除了一个Item后，删除的Item后面所有Item的index都会发生变化。
        ui->delList->takeItem(i--);
        // 3. del data
        ImgData data;
        addDelListData_.delDelImageListByOldName(oldName, data);
        ui->imgLabel->setPixmap(QString(""));
    }
}

void MainWindow::clipFromDelListSlot() {
    if(addDelListData_.getDelList().isEmpty()){
        appendTextToLog(u8"列表为空 ! ! !");
        return;
    }
    if(fullCurrentMarkdownDirPath_.isEmpty()){
        DebugBox(__FUNCTION__, __LINE__, "path error");
        appendTextToLog(u8"当前的目标路径不存在 ! ! !");
        return;
    }
    // 防止视频还在播放无法复制MP4
    videoThr_->stop();
    // 文件剪切，并返回Markdown代码
    QStringList list;
    for(int n = 0; n < ui->delList->count(); ++n){
        list.append(ui->delList->item(n)->text());
    }

    QString clipText;
    clip_->clear();
    if(fileOp_.clipFilesByFileInfo(list, addDelListData_.getDelList(), fullCurrentMarkdownDirPath_,ui->numSpinBox->value(), clipText)){
        clip_->setText(clipText);
        appendTextToLog(u8"剪切文件完成 !");
    }else{
        clip_->setText(clipText);
        appendTextToLog(u8"剪切文件失败 ! ! !");
    }
    updateListDataAndWgtSlot();
}

void MainWindow::on_clipPbn_clicked()
{
    if(addDelListData_.getAddList().isEmpty()){
        appendTextToLog(u8"添加列表为空 ! ! !");
        return;
    }
    if(fullCurrentMarkdownDirPath_.isEmpty()){
        DebugBox(__FUNCTION__, __LINE__, "path error");
        appendTextToLog(u8"当前的目标路径不存在 ! ! !");
        return;
    }
    // 防止视频还在播放无法复制MP4
    videoThr_->stop();
    // 文件剪切，并返回Markdown代码
    QStringList list;
    for(int n = 0; n < ui->addList->count(); ++n){
        list.append(ui->addList->item(n)->text());
    }
    QString clipText;
    clip_->clear();
    if(fileOp_.clipFilesByFileInfo(list, addDelListData_.getAddList(), fullCurrentMarkdownDirPath_,ui->numSpinBox->value(), clipText)){
        clip_->setText(clipText);
        appendTextToLog(u8"剪切文件完成 !");
    }else{
        clip_->setText(clipText);
        appendTextToLog(u8"剪切文件失败 ! ! !");
    }
    updateListDataAndWgtSlot();
}

QString MainWindow::getAssetsPath(){
    QString path;
    NamePath namePath;
    if(assetsPath_ == "Desktop"){
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }else{
        path = assetsPath_;
    }
    return path;
}

void MainWindow::adjustMovieSize(QMovie* movie, const QSize& labelSize) {
    if (movie) {
        QSize newSize = labelSize;

        qreal scaleFactor = qMin(newSize.width(), newSize.height());
        QSize scaledSize = QSize(scaleFactor, scaleFactor);
        movie->setScaledSize(scaledSize);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if(labelPath_.isEmpty()) {
        // todo: 路径为空，需要添加一个默认图片
        ui->imgLabel->setAlignment(Qt::AlignCenter);
        ui->imgLabel->setPixmap(labelPath_);
    }else if(labelPath_.right(4) == ".mp4"){

    }else if (labelPath_.right(4) == ".mp3"){
        QMovie *movie = new QMovie(":/qss/icon/audio.gif");
        ui->imgLabel->setMovie(movie);
        ui->imgLabel->setScaledContents(false);
        adjustMovieSize(movie, ui->imgLabel->size());
        movie->start();
    }else{
        QPixmap map = QPixmap(labelPath_);
        map = map.scaled(ui->imgLabel->width(), ui->imgLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->imgLabel->setAlignment(Qt::AlignCenter);
        ui->imgLabel->setPixmap(map);
    }

    if(!isSimpleView_){
        appNormalHeight_ = this->height();
        appNormalWidth_ = this->width();
        qDebug() << "appNormal : "  << appNormalWidth_ << appNormalHeight_;
    }
}

void MainWindow::on_lastFileNumPbn_clicked()
{
    setSubPathSlot(subDirName_);
}

void MainWindow::on_toolPbn_clicked()
{
    switch (boxSelect_) {
    case BoxSelect::NumSpinBox:
        openExPro_.OpenMarkdownAndDirSlot(fullCurrentMarkdownDirPath_+"/" + currentMarkdownFileName_);
        break;
    case BoxSelect::SubCombox:
        openExPro_.OpenDirSlot(repoPath_+"/" + subDirName_);
        break;
    case BoxSelect::TarCombox:
        openExPro_.OpenDirSlot(repoPath_);
        break;
    default:
        appendTextToLog(u8"没有选中文件需要打开!");
        break;
    }
}

void MainWindow::addSub2RepoSlot(){
    subDirName_ = ui->subPathComBox->currentText();
    if(subDirName_.isEmpty()) {
        appendTextToLog("子目录为空无法添加!");
        return ;
    }
    QString newRepoName = subDirName_;
    QString existName;
    addRepo2Conf(newRepoName, repoPath_ + "/" + newRepoName);
}

void MainWindow::addParent2RepoSlot() {
    QDir repoDir(repoPath_);
    if(repoDir.cdUp()) {
        QString parentAbs = repoDir.absolutePath();
        QString newName = repoDir.dirName();
        if(newName.isEmpty()) { // 到了根目录
            newName = parentAbs.split("/").first();
        }
        addRepo2Conf(newName, parentAbs);
    }else{
        appendTextToLog(QString("当前仓库:\"") + repoPath_ + "\"已经是根目录!");
    }
}

bool MainWindow::addRepo2Conf(QString newName,QString pathAbs) {
    QString existName;
    if(confData_.addRepoPath(newName, pathAbs, existName)){
        confData_.writeConfJson();
        updateDataAndWidget();
        ui->tarPathCombox->setCurrentText(newName);
        appendTextToLog(QString("添加\"") + newName + "\"仓库成功!");
        return true;
    }
    ui->tarPathCombox->setCurrentText(existName);
    appendTextToLog("\"" + existName + "\"和\"" + newName + "\"是同一个仓库!");
    return false;
}

void MainWindow::addRepoSlot(){
    QString repoPath = QFileDialog::getExistingDirectory(this,"选择重命名目录",repoPath_,QFileDialog::ShowDirsOnly);
    if(repoPath.isEmpty()) {
        appendTextToLog("添加仓库\"" + repoPath + "\"为空，无法添加!");
        return ;
    }

    QDir repoDir(repoPath);
    addRepo2Conf(repoDir.dirName(),repoDir.absolutePath());
}

void MainWindow::addAssstsDirSlot() {
    QString path = QFileDialog::getExistingDirectory(this,"选择重命名目录",repoPath_,QFileDialog::ShowDirsOnly);
    if(path.isEmpty()) {
        appendTextToLog("添加资源目录\"" + path + "\"为空，无法添加!");
        return ;
    }

    QDir assetsDir(path);
    addAssetsDir2Conf(assetsDir.dirName(),assetsDir.absolutePath());
}

void MainWindow::openAssstsDirSlot(){
    openExPro_.OpenDirSlot(getAssetsPath());
}

bool MainWindow::addAssetsDir2Conf(QString newName,QString pathAbs) {
    QString existName;
    if(confData_.addAssetsPath(newName, pathAbs, existName)){
        confData_.writeConfJson();
        updateDataAndWidget();
        ui->tarPathCombox->setCurrentText(newName);
        appendTextToLog(QString("添加\"") + newName + "\"仓库成功!");
        return true;
    }
    ui->tarPathCombox->setCurrentText(existName);
    appendTextToLog("\"" + existName + "\"和\"" + newName + "\"是同一个仓库!");
    return false;
}

void MainWindow::openConfDirSlot() {
    openExPro_.OpenDirSlot(confData_.getIniFile().iniAndJsonPath);
}

void MainWindow::modifyMarkdownSoftSlot(){
    QString MarkdownSoftPath = QFileDialog::getOpenFileName(this,"选择markdown软件",QDir::homePath(),tr("EXE files(*.exe);;Shell files(*.sh);;bash files(*.bat);;All files(*.*)"));
    if(!MarkdownSoftPath.isEmpty()) {
        confData_.setMarkdownSoftWarePath(MarkdownSoftPath);
        openExPro_.setMarkdownSoftWarePath(MarkdownSoftPath);
        confData_.writeConfJson();
        appendTextToLog(QString("修改Markdown软件为\"") + MarkdownSoftPath + "\"成功!");
    }else {
        appendTextToLog(QString("MarkDown软件:\"") + MarkdownSoftPath + "\"为空，修改失败!");
    }
}
void MainWindow::modifyDataDirSoftSlot(){
    QString DataDirSoftPath = QFileDialog::getOpenFileName(this,"选择Data软件",QDir::homePath(),tr("EXE files(*.exe);;Shell files(*.sh);;bash files(*.bat);;All files(*.*)"));
    if(!DataDirSoftPath.isEmpty()) {
        confData_.writeConfJson();
        confData_.setDataDirSoftWarePath(DataDirSoftPath);
        openExPro_.setDataDirSoftWarePath(DataDirSoftPath);
        appendTextToLog(QString("修改Data软件为\"") + DataDirSoftPath + "\"成功!");
    }else {
        appendTextToLog(QString("Data软件:\"") + DataDirSoftPath + "\"为空，修改失败!");
    }
}

void MainWindow::delCurrentRepoSlot(){
    QString delRepoPath = repoPath_;
    confData_.delRepoPath(repoPath_);
    confData_.writeConfJson();
    updateDataAndWidget();
    appendTextToLog(QString("删除\"") + delRepoPath + "\"仓库成功!");
}

void MainWindow::delAssstsDirSlot(){
    QString delAssetPath = getAssetsPath();
    confData_.delAssetsPath(delAssetPath);
    confData_.writeConfJson();
    updateDataAndWidget();
    appendTextToLog(QString("删除\"") + delAssetPath + "\"资源目录成功!");
}

void MainWindow::confDataSettingSlot() {
    modifyConfDlg_->setWidth(screenWidth_);
    confData_.setVersion(VERSION);
    modifyConfDlg_->setConfigData(confData_);
    modifyConfDlg_->showWindow();
}

void MainWindow::exportMarkdownSlot()
{
    getMarkdownFileDig_->showWindow(repoPath_, repoPath_+ "/" +subDirName_, screenWidth_, ui->numSpinBox->value());
}

void MainWindow::openCurrentDirSlot(){
    switch (boxSelect_) {
    case BoxSelect::NumSpinBox:
        openExPro_.OpenDirSlot(repoPath_+"/" + subDirName_);
        break;
    case BoxSelect::SubCombox:
        openExPro_.OpenDirSlot(repoPath_+"/" + subDirName_);
        break;
    case BoxSelect::TarCombox:
        openExPro_.OpenDirSlot(repoPath_);
        break;
    default:
        appendTextToLog(u8"没有选中文件需要打开!");
        break;
    }
}

void MainWindow::on_numSpinBox_valueChanged(int num)
{
    if(num == -1){
        whoIsBoxSelection(BoxSelect::SubCombox);
        return;
    }
    if(fullCurrentMarkdownDirPath_.isEmpty()){
        appendTextToLog(u8"当前的目标路径不存在 !");
        whoIsBoxSelection(BoxSelect::SubCombox);

        QFileInfo currentInfo(currentMarkdownFileName_);
        QFileInfo repoInfo(repoPath_);
        this->setWindowTitle("Error - " + repoInfo.fileName() + " - [" + configFilePath_ + "]");
        return;
    }
    if(fileOp_.getFileNameByNum(fullCurrentMarkdownDirPath_, num, currentMarkdownFileName_)){
        if(currentMarkdownFileName_.size() >3 && currentMarkdownFileName_.right(2) == "md"){
            appendTextToLog(u8"当前文档为：" + currentMarkdownFileName_);
        }else{
            appendTextToLog(u8"当前目录为：" + currentMarkdownFileName_);
        }
        setStatusBar("",true);
        whoIsBoxSelection(BoxSelect::NumSpinBox);


        QFileInfo currentInfo(currentMarkdownFileName_);
        QFileInfo repoInfo(repoPath_);
        this->setWindowTitle(currentInfo.fileName() + " - " + repoInfo.fileName() + " - [" + configFilePath_ + "]");

    }else{
        setStatusBar("",false);
        currentMarkdownFileName_.clear();
        whoIsBoxSelection(BoxSelect::SubCombox);

        QFileInfo currentInfo(currentMarkdownFileName_);
        QFileInfo repoInfo(repoPath_);
        this->setWindowTitle("Error - " + repoInfo.fileName() + " - [" + configFilePath_ + "]");

    }
}

void MainWindow::ChangeToHistoryFile(){
    //获取当前点击的单元格的指针
    QTableWidgetItem* curItem = ui->historyFileList->currentItem(); // ui->tableWidget->currentItem();
    if(curItem->column() != 0){
        return;
    }
    //获取单元格内的内容
    QString text = curItem->text();
    QStringList testList = text.split("/");
    if(testList.size() != 2){
        return;
    }
    subDirName_ = testList.at(0);
    ui->subPathComBox->setCurrentText(subDirName_);

    QStringList nameArr = testList.at(1).split("-");
    if(nameArr.size() < 2){
        return;
    }
    int num =nameArr.at(0).toInt();
    ui->numSpinBox->setValue(num);
    currentMarkdownFileName_ = testList.at(1);
    fullCurrentMarkdownDirPath_ = repoPath_ + "/" + subDirName_;
    setStatusBar("", true);
    whoIsBoxSelection(BoxSelect::NumSpinBox);
}

void MainWindow::OpenHistoryFile(){
    //获取单元格内的内容
    QTableWidgetItem* curItem = ui->historyFileList->currentItem();
    QString text = curItem->text();
    if(text.contains("-old/") || text.contains("-export/") ) {
        QFile file(repoPath_ + "/" + text);
        if(file.exists()){
            openExPro_.OpenMarkdownAndDirSlot(repoPath_+ "/" + text);
        }else{
            openExPro_.OpenMarkdownAndDirSlot(oldPath_+ "/" + text);
        }
    }else{
        ChangeToHistoryFile();
        on_toolPbn_clicked(); // 打开文件
    }
}
void MainWindow::CompareRenameFileList(QString pathA, QString pathB){
    openExPro_.CompareFileSlot(pathA, pathB);
}

void MainWindow::OpenRenameDirPath(QString path){
    openExPro_.OpenDirSlot(path);
}

void MainWindow::whoIsBoxSelection(BoxSelect select)
{
    boxSelect_ = select;
    switch (select) {
    case BoxSelect::TarCombox:
        changTarPathStyle(true);
        changSubPathStyle(false);
        changNumStyle(false);
        break;
    case BoxSelect::SubCombox:
        changTarPathStyle(false);
        changSubPathStyle(true);
        changNumStyle(false);
        break;
    case BoxSelect::NumSpinBox:
        changTarPathStyle(false);
        changSubPathStyle(false);
        changNumStyle(true);
        break;

    default:  // None
        changTarPathStyle(false);
        changSubPathStyle(false);
        changNumStyle(false);
        break;
    }
}

void MainWindow::changNumStyle(bool flags){
    if(flags){
        ui->numSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}\nQSpinBox{\nborder: 1px solid #d9d5d2\n}\n");
    }else{
        ui->numSpinBox->setStyleSheet("QSpinBox::up-button,QSpinBox::down-button\n{\n	width:0px;\n}");
    }
}

void MainWindow::changSubPathStyle(bool flags){
    if(flags){
        ui->subPathComBox->setStyleSheet("border: 1px solid #d9d5d2");
    }else{
        ui->subPathComBox->setStyleSheet("");
    }
}

void MainWindow::changTarPathStyle(bool flags){
    if(flags){
        ui->tarPathCombox->setStyleSheet("border: 1px solid #d9d5d2");
    }else{
        ui->tarPathCombox->setStyleSheet("");
    }
}

void MainWindow::getAssetsSlot()
{
    getAssetsDialog_->setWidth(screenWidth_);
    getAssetsDialog_->initSize();
    getAssetsDialog_->show();
}

void MainWindow::newConfFileSlot()
{
    QString newFileName;
    if(fileOp_.createJsonFile(confData_.getIniFile().iniAndJsonPath, newFileName)){
        openExPro_.OpenMarkdownAndDirSlot(confData_.getIniFile().iniAndJsonPath+"/" + newFileName);
    }
}

// 极简窗口
void MainWindow::setSampleView(){
    ui->tabWgt->hide();
    ui->delList->hide();
    ui->openWgt->hide();
    ui->createMarkdownPbn->hide();
    ui->syncPbn->hide();
    ui->modePbn->hide();
    setMinimumSize(QSize(appSampleWidth_ - 50, appSampleHeight_ - 50));
    this->resize(QSize(appSampleWidth_, appSampleHeight_));
    isStayTop_ = true;
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    showSlot();
}
// 正常窗口
void MainWindow::setNormalView(){
    ui->tabWgt->show();
    ui->delList->show();
    ui->openWgt->show();
    ui->createMarkdownPbn->show();
    ui->syncPbn->show();
    ui->modePbn->show();
    this->resize(QSize(appNormalWidth_, appNormalHeight_));
    setMinimumSize(QSize(appNormalMinWidth_, appNormalMinHeight_));
    isStayTop_ = false;
    setWindowFlag(Qt::WindowStaysOnTopHint, false);
    showSlot();
}

void MainWindow::simpleViewSlot()
{
    isSimpleView_ = !isSimpleView_;
    if(isSimpleView_){
        setSampleView();
        ui->actionSimpleView->setText(u8"正常窗口");
        ui->actionSimpleView->setShortcut(QKeySequence("Ctrl+="));
    }else{
        setNormalView();
        ui->actionSimpleView->setText(u8"极简窗口");
        ui->actionSimpleView->setShortcut(QKeySequence("Ctrl+-"));
    }
}

void MainWindow::openAboutSlot()
{
    aboutDialog_->setVersion(VERSION);
    aboutDialog_->setWidth(screenWidth_);
    aboutDialog_->showWindow();
}

void MainWindow::searchAssetsByCodeSlot(QString code,QString renameList)
{
    QString result;
    fileOp_.getSearchResultFromMarkdownCode(fullCurrentMarkdownDirPath_, code,renameList, result);
}

void MainWindow::createMarkdownAndSubDirSlot(int type, QString namePathAbs)
{
    if(1 == type) {
        on_lastFileNumPbn_clicked();
        openExPro_.OpenMarkdownAndDirSlot(fullCurrentMarkdownDirPath_+"/" + currentMarkdownFileName_);
    }else if (2 == type){
        updateDataAndWidget();
        openExPro_.OpenDirSlot(namePathAbs);
    }else if(3 == type){
        QDir repoDir(namePathAbs);
        addRepo2Conf(repoDir.dirName(),repoDir.absolutePath());
        openExPro_.OpenDirSlot(namePathAbs);
    }
}

void MainWindow::copyAssetsToExportDirSlot(QString oldfileAbs, QString newFileDir, QString fileName)
{
    QString code;
    QFile file(oldfileAbs);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        code = in.readAll();
        file.close();
    }
    QString folderPath = QFileInfo(oldfileAbs).path();
    QString fileNum = fileName.split("-").first();
    QString renameList;
    QString result;
    fileOp_.getSearchResultFromMarkdownCode(folderPath, code, renameList, result, newFileDir, fileNum);
}

void MainWindow::on_createMarkdownPbn_clicked()
{
    createMarkdownAndSubDirDlg_->setSubDirPath(fullCurrentMarkdownDirPath_);
    createMarkdownAndSubDirDlg_->setWidth(screenWidth_);
    createMarkdownAndSubDirDlg_->setRepoPath(repoPath_);
    createMarkdownAndSubDirDlg_->showWindow();
}

void MainWindow::on_historySearchPbn_clicked()
{
    QString txt = ui->historySearchLineEditor->text();
    updateRepoHistoryFileListBySearchSlot(txt);
}

void MainWindow::updateRepoHistoryFileListBySearchSlot(QString txt){
    QFileInfoList fileList20ByCurRepo;
    QFileInfoList fileList20ByOldRepo;
    ui->historyFileList->clear();
    oldPath_ = confData_.getOldPathByKey(ui->tarPathCombox->currentText());
    fileOp_.getSearchFileList(repoPath_, fileList20ByCurRepo, txt);
    if(!oldPath_.isEmpty()){
        fileOp_.getSearchFileList(oldPath_, fileList20ByOldRepo, txt);
    }
    ui->historyFileList->setRowCount(fileList20ByCurRepo.size() + fileList20ByOldRepo.size());        //设置行数/
    int i = 0;
    for(; i < fileList20ByCurRepo.size(); ++i){
        QFileInfo fileInfo = fileList20ByCurRepo.at(i);
        QString name = fileInfo.filePath().split(repoPath_).last().remove(0,1);
        QString modifyTime = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss ddd");
        QTableWidgetItem *pItem1 = new QTableWidgetItem(name);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(modifyTime);
        ui->historyFileList->setItem(i, 0, pItem1);
        ui->historyFileList->setItem(i, 1, pItem2);
    }
    for(int j = 0; j < fileList20ByOldRepo.size(); ++j){
        QFileInfo fileInfo = fileList20ByOldRepo.at(j);
        QString name = fileInfo.filePath().split(oldPath_).last().remove(0,1);
        QString modifyTime = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss ddd");
        QTableWidgetItem *pItem1 = new QTableWidgetItem(name);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(modifyTime);
        ui->historyFileList->setItem(i + j, 0, pItem1);
        ui->historyFileList->setItem(i + j, 1, pItem2);
    }
    ui->historyFileList->setColumnWidth(0, 1300 * 0.66);
    ui->historyFileList->setColumnWidth(1, 1300 * 0.33);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->addList || obj == ui->delList)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QListWidget* listWgt = dynamic_cast<QListWidget*>(obj);
            listWgt->setSelectionMode(QAbstractItemView::ExtendedSelection);
        }
    }
    return QWidget::eventFilter(obj, event);
}

void MainWindow::showModifyNameDlg(){
    renameFileName_->setRepoPath(repoPath_);
    renameFileName_->setRenameDirPath(repoPath_);
    renameFileName_->setRenameConfPath(confData_.getIniFile().iniAndJsonPath + "/99-CharReplace.json");
    renameFileName_->setRenameListPath(confData_.getIniFile().iniAndJsonPath + "/99-RenameList.txt");
    renameFileName_->setWidth(screenWidth_);
    renameFileName_->initWindowSize();
    renameFileName_->renameListClear();
    renameFileName_->show();
}

void MainWindow::on_zoomPercentPtn_clicked()
{
    int zoomWidth = 0;
    if(ui->RadioBtn5->isChecked()) {
        zoomWidth = AssetsZoomSize::Size5;
    }else if(ui->RadioBtn30->isChecked()) {
        zoomWidth = AssetsZoomSize::Size30;
    }else if(ui->RadioBtn50->isChecked()) {
        zoomWidth = AssetsZoomSize::Size50;
    }else if(ui->RadioBtn80->isChecked()) {
        zoomWidth = AssetsZoomSize::Size80;
    }else {
        zoomWidth = AssetsZoomSize::Size100;
    }
    QList<QListWidgetItem*> addSelectedItems = ui->addList->selectedItems();
    QList<QListWidgetItem*> delSelectedItems = ui->delList->selectedItems();
    if(addSelectedItems.isEmpty() && delSelectedItems.isEmpty()) {
        appendTextToLog(QString("没有选中任何资源，修改失败!"));
        return;
    }
    for(int i = 0; i < addSelectedItems.size(); ++i) {
        QString name = addSelectedItems.at(i)->text();
        addDelListData_.modifyAddAssetsListZoomWidth(name, zoomWidth);
    }
    for(int i = 0; i < delSelectedItems.size(); ++i) {
        QString name = delSelectedItems.at(i)->text();
        addDelListData_.modifyDelAssetsListZoomWidth(name, zoomWidth);
    }
    changedPercentByLabelImg(zoomWidth);
    ui->addList->clearSelection();
    ui->delList->clearSelection();
}
