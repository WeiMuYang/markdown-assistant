#include "mainwindow.h"
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
#include <QImageReader>
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
    scrrenWidth_ = printscreeninfo();
    videoThr_ = new VideoThr;
    clip_ = QApplication::clipboard();
    DebugBox logBoxVideoThr;
    getAssetsDialog_ = new GetAssetsDialog(this);
    aboutDialog_ = new AboutDialog(this);
    renameFileName_ = new RenameFileName(this);
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
    InitMainWindowMenu();
    initAddDelListMenu();
    startSlot();
}

void MainWindow::initTray() {
    trayIcon_ = new QSystemTrayIcon(this);
    trayIcon_->setIcon(QIcon(":/qss/icon/markdown-assistant.ico"));
    trayMenu_ = new QMenu(this);
    QAction *showAction = new QAction("显  示", this);
    QAction *quitAction = new QAction("退  出", this);
    trayMenu_->addAction(showAction);
    trayMenu_->addAction(quitAction);
    trayIcon_->setContextMenu(trayMenu_);
    connect(showAction, &QAction::triggered, this, &MainWindow::show);
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

void MainWindow::quitAppSlot() {
    QApplication::quit();
}

void MainWindow::trayIconClickedSlot(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        // 单击操作
        show();
    } else if (reason == QSystemTrayIcon::DoubleClick) {
        // 双击操作
        show();
    }
}

void MainWindow::startSlot() {
    confDialog_.clearAll();
    if(!confDialog_.readIniFile()){
        appendTextToLog(QString("iniFile读取失败 !"));
    }
    updateActionConfFileList();
    // 根据用户名设置路径
    setConfigFilePathByUserName(confDialog_.getIniFile());
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
    QDir curDir(fullTarPath_);
    curDir.setCurrent(fullTarPath_);
    QString filePath = curDir.relativeFilePath(tarPath_ + "/" + path);
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
    if(list.isEmpty() || confDialog_.getMeetFilePath().isEmpty()){
        return;
    }
    QString path =list.at(0)->text();
    QDir curDir(confDialog_.getMeetFilePath());
    curDir.setCurrent(fullTarPath_);
    QString filePath = curDir.relativeFilePath(tarPath_ + "/" + path);
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
    confDialog_.readConf(configFilePath_);
    openExPro_.setMarkdownSoftWarePath(confDialog_.getMarkdownSoftPath());
    openExPro_.setDataDirSoftWarePath(confDialog_.getDataDirSoftPath());
    addDelListData_.setAssetsTypes(confDialog_.getAssetsTypes());
    fileOp_.setAssetsTypes(confDialog_.getAssetsTypes());

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
//
}

void MainWindow::updateRepoHistoryFileList(){
    QFileInfoList fileListTop20;
    ui->historyFileList->clear();
    fileOp_.getHistoryFileList(tarPath_, fileListTop20);
    ui->historyFileList->setRowCount(fileListTop20.size());        //设置行数/
    for(int i = 0; i < fileListTop20.size(); ++i){
        QFileInfo fileInfo = fileListTop20.at(i);
        QString name = fileInfo.filePath().split(tarPath_).last().remove(0,1);
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
    fileOp_.getHistorySubDirFileList(tarPath_ + "/" + subDirName_, fileListTop20);
    ui->historyFileList->setRowCount(fileListTop20.size());        //设置行数/
    for(int i = 0; i < fileListTop20.size(); ++i){
        QFileInfo fileInfo = fileListTop20.at(i);
        QString name = fileInfo.filePath().split(tarPath_).last().remove(0,1);
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
    QFileInfo configInfo(configFilePath_);
    this->setWindowTitle(configInfo.baseName() + " - Markdown Assistant - [" + configFilePath_ + "]");
    if(!hasUserConf){
        appendTextToLog(QString("未配置该用户文件 !"));
    }
}

void MainWindow::updateActionConfFileList()  {
    IniFile iniFile = confDialog_.getIniFile();
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
    connect(ui->actionRestart,&QAction::triggered, this, &MainWindow::updateConfFileSlot);

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
            show();
        }else {
            setWindowFlag(Qt::WindowStaysOnTopHint, false);
            show();
        }
    });

    if(simpleViewNum_ % 2 != 0){
        setSampleView();
        ui->actionSimpleView->setText(u8"正常窗口");
        ui->actionSimpleView->setShortcut(QKeySequence("Ctrl+="));
    }else{
        setNormalView();
        ui->actionSimpleView->setText(u8"极简窗口");
        ui->actionSimpleView->setShortcut(QKeySequence("Ctrl+-"));
    }
    connect(ui->actionSimpleView, &QAction::triggered, this, &MainWindow::simpleViewSlot,Qt::UniqueConnection);

    ui->actionModifyConf->setShortcut(QKeySequence("Ctrl+M"));
    connect(ui->actionModifyConf, &QAction::triggered, this, &MainWindow::modifyConfSlot);

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

    ui->actionAddMarkDownSoftWare->setShortcut(QKeySequence("Alt+M"));
    connect(ui->actionAddMarkDownSoftWare, &QAction::triggered, this, &MainWindow::modifyMarkdownSoftSlot);

    ui->actionAddVSCodePath->setShortcut(QKeySequence("Alt+V"));
    connect(ui->actionAddVSCodePath, &QAction::triggered, this, &MainWindow::modifyDataDirSoftSlot);

    ui->actionAddRepo->setShortcut(QKeySequence("Alt+N"));
    connect(ui->actionAddRepo, &QAction::triggered, this, &MainWindow::addRepoSlot);
}

void MainWindow::switchConfFileSlot(){
    QAction *action=qobject_cast<QAction *>(sender());
    QString fileName = confDialog_.getIniFile().iniAndJsonPath + "/"+ action->text();
    configFilePath_ = fileName;
    QFileInfo configFile(configFilePath_);
    confDialog_.setIniFileHostName(configFile.baseName());
    confDialog_.writeIniFile();
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
    openExPro_.OpenMarkdownAndDirSlot(confDialog_.getIniFile().iniAndJsonPath+ "/conf.ini");
}

void MainWindow::modifyIniFileSlot(){
    openExPro_.OpenJsonAndIniSlot(confDialog_.getIniFile().iniAndJsonPath+ "/conf.ini");
    startSlot();
}

void MainWindow::modifyConfSlot()
{
    openExPro_.OpenJsonAndIniSlot(configFilePath_);
    updateConfFileSlot();
}

void MainWindow::openReadMeSlot()
{
    QString readMeFilePath;
    if(fileOp_.getReadMePath(tarPath_,readMeFilePath)){
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

void MainWindow::setNormalViewByScreenRes(){
    if(getScrrenRes() == ScreenRes::High){
        this->setMinimumSize(QSize(1300, 1000));
        this->resize(QSize(1300, 1000));
        ui->pathWgt->setMinimumSize(QSize(500,155));
        ui->listPbnWgt->setMinimumHeight(50);
        ui->listPbnWgt->setMaximumHeight(65);
        multiple = 2;
    }else{
        this->setMinimumSize(QSize(860, 600)); // int w, int h
        this->resize(QSize(860, 600));
        ui->pathWgt->setMaximumHeight(126);
        ui->pathWgt->setMinimumSize(QSize(0,126));
        ui->listPbnWgt->setMinimumHeight(40);
        ui->listPbnWgt->setMaximumHeight(50);
        ui->pathWgt->setMinimumWidth(440);
        multiple = 1;
    }
}

void MainWindow::setSampleViewByScreenRes(){
    if(getScrrenRes() == ScreenRes::High){
        this->setMinimumSize(QSize(440, 240));
        this->resize(QSize(880, 480));
        ui->pathWgt->setMinimumWidth(440);
    }else{
        this->setMinimumSize(QSize(650, 350));
        this->resize(QSize(650, 350));
        ui->pathWgt->setMinimumWidth(350);
    }
}

void MainWindow::initScreenResNormal(){
    setNormalViewByScreenRes();
    initListWgt();
}

void MainWindow::setScreenWitdh(int w)
{
    scrrenWidth_ = w;
}

int MainWindow::getScreenWitdh()
{
    return scrrenWidth_;
}

ScreenRes MainWindow::getScrrenRes()
{
    if(scrrenWidth_ >= 3840){
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
        pStatusLabelCurrentFile_->setText("|  " +subDirName_ +"/"+ currentFile_);
    }else{
        QImage image(QString(":/qss/psblack/checkbox_checked_disable.png"));
        pStatusLabelIcon_->setPixmap(QPixmap::fromImage(image));
        pStatusLabelIcon_->setMinimumWidth(15);
        pStatusLabelMsg_->setText(u8"错误");
        pStatusLabelCurrentFile_->setText("|  " + currentFile_);
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
    int num = fileOp_.getLastmodifiedTimeFileNum(tarPath_,fullTarPath_,currentFile_);
    if(num == -1){
        ui->subPathComBox->setCurrentText("no file");
        ui->numSpinBox->setValue(num);
//        tarPath_.clear();
//        fullTarPath_.clear();
        currentFile_.clear();
        appendTextToLog(QString(u8"当前的目标路径不存在 !"));
        setStatusBar("", false);
        whoIsBoxSelection(BoxSelect::SubCombox);
    }else{
        QStringList strList = fullTarPath_.split(tarPath_);
        QString subPath =strList.last().replace("/","");
        ui->subPathComBox->setCurrentText(subPath);
        ui->numSpinBox->setValue(num);
        setStatusBar("", true);
        subDirName_ = subPath;
        whoIsBoxSelection(BoxSelect::NumSpinBox);
    }
}
// 1 Init
void MainWindow::initImgPathTarPathCombox()
{
    disconnect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    disconnect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
    ui->subPathComBox->clear();
    ui->tarPathCombox->clear();

    for(auto it = confDialog_.getImgPaths().begin();it != confDialog_.getImgPaths().end(); ++it){
        ui->imgPathCombox->addItem(it->key);
    }
    for(auto it = confDialog_.getTarPaths().begin();it != confDialog_.getTarPaths().end(); ++it){
        ui->tarPathCombox->addItem(it->key);
    }
    this->imgPath_ = confDialog_.getImgPathByKey(ui->imgPathCombox->currentText());
    this->tarPath_ = confDialog_.getTarPathByKey(ui->tarPathCombox->currentText());
    connect(ui->tarPathCombox,&QComboBox::currentTextChanged,this,&MainWindow::setTarPathSlot);
    connect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
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
    ui->subPathComBox->addItems(fileOp_.getSubDirNames(this->tarPath_));
    setComboBoxToolTip(ui->subPathComBox);
    subDirName_ = ui->subPathComBox->currentText();
    whoIsBoxSelection(BoxSelect::SubCombox);
    connect(ui->subPathComBox,&QComboBox::currentTextChanged,this,&MainWindow::setSubPathSlot);
}

void MainWindow::setImgPathSlot(QString currentStr){
    this->imgPath_ = confDialog_.getImgPathByKey(currentStr);
}

void MainWindow::setTarPathSlot(QString currentStr){
    this->tarPath_ = confDialog_.getTarPathByKey(currentStr);
    if(tarPath_.isEmpty() || !fileOp_.isPathExist(this->tarPath_)){
        ui->subPathComBox->clear();
        fullTarPath_.clear();
        subDirName_.clear();
        tarPath_.clear();
        ui->numSpinBox->setValue(-1);
        appendTextToLog(QString(u8"当前的目标路径不存在 !"));
        currentFile_.clear();
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
    int num = fileOp_.getLastmodifiedTimeFileNumSubDir(tarPath_,subDirName_, fullTarPath_,currentFile_);
    if(num == -1){
//        fullTarPath_.clear();
        ui->numSpinBox->setValue(num);
        appendTextToLog(QString(u8"当前的目标路径不存在 !"));
        setStatusBar("", false);
        whoIsBoxSelection(BoxSelect::SubCombox);
    }else{
        QStringList strList = fullTarPath_.split(tarPath_);
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
        font = ui->addList->font();
        font.setPointSize(13);
        ui->addList->setFont(font);

        font = ui->delList->font();
        font.setPointSize(13);
        ui->delList->setFont(font);

        font = ui->historyFileList->font();
        font.setPointSize(13);
        ui->historyFileList->setFont(font);
        multiple = 2;
    }else{
        font = ui->addList->font();
        font.setPointSize(10);
        ui->addList->setFont(font);

        font = ui->delList->font();
        font.setPointSize(10);
        ui->delList->setFont(font);

        font = ui->historyFileList->font();
        font.setPointSize(10);
        ui->historyFileList->setFont(font);

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

void MainWindow::updateListWgt()
{
    ui->addList->clear();
    ui->delList->clear();

    for(int i = 0; i < addDelListData_.getAddList().size(); ++i){
        QListWidgetItem *pItem = new QListWidgetItem;
        ImgData data = addDelListData_.getAddList().at(i);
        if(data.oldName.right(4) != ".mp4"){
            pItem->setIcon(QIcon(data.oldPath));
        }else{
            QIcon videoIcon;
            if(videoThr_->getFirstVideoFrame(data.oldPath, videoIcon)){
                pItem->setIcon(videoIcon);
            }else{
                pItem->setIcon(QIcon(":/qss/icon/markdown-assistant.ico"));
            }
        }
        if(isIconMode_){
            pItem->setSizeHint(QSize(200,100* multiple));
            pItem->setText(data.oldName);
            pItem->setToolTip(data.oldName);
        }else{
            pItem->setSizeHint(QSize(20,20*multiple));
            pItem->setText(data.oldName);
            pItem->setToolTip(data.oldName);
        }
        ui->addList->addItem(pItem);
    }
    for(int i = 0; i < addDelListData_.getDelList().size(); ++i){
        QListWidgetItem *pItem = new QListWidgetItem;
        ImgData data = addDelListData_.getDelList().at(i);
        if(data.oldName.right(4) != ".mp4"){
            pItem->setIcon(QIcon(data.oldPath));
        }else{
            QIcon videoIcon;
            if(videoThr_->getFirstVideoFrame(data.oldPath, videoIcon)){
                pItem->setIcon(videoIcon);
            }else{
                pItem->setIcon(QIcon(":/qss/icon/markdown-assistant.ico"));
            }
        }
        if(isIconMode_){
            pItem->setSizeHint(QSize(200,100*multiple));
            pItem->setText(data.oldName);
            pItem->setToolTip(data.oldName);
        }else{
            pItem->setSizeHint(QSize(20,20*multiple));
            pItem->setText(data.oldName);
            pItem->setToolTip(data.oldName);
        }

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
    if(name.right(4) != ".mp4") {
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
    }else{ // mp4
        videoThr_->setVideoPath(labelPath_);
        videoThr_->play();
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
    QFile file(fullTarPath_+ "/" + currentFile_);
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
        if(data.oldName.right(4) != ".mp4"){
            pItem->setIcon(QIcon(data.oldPath));
        }else{
            QIcon videoIcon;
            if(videoThr_->getFirstVideoFrame(data.oldPath, videoIcon)){
                pItem->setIcon(videoIcon);
            }else{
                pItem->setIcon(QIcon(":/qss/icon/markdown-assistant.ico"));
            }
        }
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
    if(fullTarPath_.isEmpty()){
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
    if(fileOp_.clipFilesByFileInfo(list, addDelListData_.getDelList(), fullTarPath_,ui->numSpinBox->value(), clipText)){
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
    if(fullTarPath_.isEmpty()){
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
    if(fileOp_.clipFilesByFileInfo(list, addDelListData_.getAddList(), fullTarPath_,ui->numSpinBox->value(), clipText)){
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
    if(confDialog_.findImgPath(this->imgPath_, namePath)){
        path = namePath.value;
    }else{
        DebugBox(__FUNCTION__, __LINE__,"img path empty");
    }
    if(path == "Desktop"){
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }else{
        DebugBox(__FUNCTION__, __LINE__,"path error");
    }
    return path;
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    if(!labelPath_.isEmpty()){
        QPixmap map = QPixmap(labelPath_);
        map = map.scaled(ui->imgLabel->width(), ui->imgLabel->height(),Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->imgLabel->setAlignment(Qt::AlignCenter);
        ui->imgLabel->setPixmap(map);
    }else{
        // todo: 路径为空，需要添加一个默认图片
        ui->imgLabel->setAlignment(Qt::AlignCenter);
        ui->imgLabel->setPixmap(labelPath_);
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
        openExPro_.OpenMarkdownAndDirSlot(fullTarPath_+"/" + currentFile_);
        break;
    case BoxSelect::SubCombox:
        openExPro_.OpenDirSlot(tarPath_+"/" + subDirName_);
        break;
    case BoxSelect::TarCombox:
        openExPro_.OpenDirSlot(tarPath_);
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
    addRepo2Conf(newRepoName, tarPath_ + "/" + newRepoName);
}

void MainWindow::addParent2RepoSlot() {
    QDir repoDir(tarPath_);
    if(repoDir.cdUp()) {
        QString parentAbs = repoDir.absolutePath();
        QString newName = repoDir.dirName();
        if(newName.isEmpty()) { // 到了根目录
            newName = parentAbs.split("/").first();
        }
        addRepo2Conf(newName, parentAbs);
    }else{
        appendTextToLog(QString("当前仓库:\"") + tarPath_ + "\"已经是根目录!");
    }
}

bool MainWindow::addRepo2Conf(QString newName,QString pathAbs) {
    QString existName;
    if(confDialog_.addTarNamePath(newName, pathAbs, existName)){
        confDialog_.writeConfJson();
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
    QString repoPath = QFileDialog::getExistingDirectory(this,"选择重命名目录",tarPath_,QFileDialog::ShowDirsOnly);
    if(repoPath.isEmpty()) {
        appendTextToLog("添加仓库\"" + repoPath + "\"为空，无法添加!");
        return ;
    }

    QDir repoDir(repoPath);
    addRepo2Conf(repoDir.dirName(),repoDir.absolutePath());
}

void MainWindow::openConfDirSlot() {
    openExPro_.OpenDirSlot(confDialog_.getIniFile().iniAndJsonPath);
}

void MainWindow::modifyMarkdownSoftSlot(){
    QString MarkdownSoftPath = QFileDialog::getOpenFileName(this,"选择markdown软件",QDir::homePath(),tr("EXE files(*.exe);;Shell files(*.sh);;bash files(*.bat);;All files(*.*)"));
    if(!MarkdownSoftPath.isEmpty()) {
        confDialog_.setMarkdownSoftWarePath(MarkdownSoftPath);
        openExPro_.setMarkdownSoftWarePath(MarkdownSoftPath);
        confDialog_.writeConfJson();
        appendTextToLog(QString("修改Markdown软件为\"") + MarkdownSoftPath + "\"成功!");
    }else {
        appendTextToLog(QString("MarkDown软件:\"") + MarkdownSoftPath + "\"为空，修改失败!");
    }
}
void MainWindow::modifyDataDirSoftSlot(){
    QString DataDirSoftPath = QFileDialog::getOpenFileName(this,"选择Data软件",QDir::homePath(),tr("EXE files(*.exe);;Shell files(*.sh);;bash files(*.bat);;All files(*.*)"));
    if(!DataDirSoftPath.isEmpty()) {
        confDialog_.writeConfJson();
        confDialog_.setDataDirSoftWarePath(DataDirSoftPath);
        openExPro_.setDataDirSoftWarePath(DataDirSoftPath);
        appendTextToLog(QString("修改Data软件为\"") + DataDirSoftPath + "\"成功!");
    }else {
        appendTextToLog(QString("Data软件:\"") + DataDirSoftPath + "\"为空，修改失败!");
    }
}

void MainWindow::delCurrentRepoSlot(){
    QString delRepoName = tarPath_;
    confDialog_.delTarNamePath(tarPath_);
    confDialog_.writeConfJson();
    updateDataAndWidget();
    appendTextToLog(QString("删除\"") + delRepoName + "\"仓库成功!");
}

void MainWindow::openCurrentDirSlot(){
    switch (boxSelect_) {
    case BoxSelect::NumSpinBox:
        openExPro_.OpenDirSlot(tarPath_+"/" + subDirName_);
        break;
    case BoxSelect::SubCombox:
        openExPro_.OpenDirSlot(tarPath_+"/" + subDirName_);
        break;
    case BoxSelect::TarCombox:
        openExPro_.OpenDirSlot(tarPath_);
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
    if(fullTarPath_.isEmpty()){
        appendTextToLog(u8"当前的目标路径不存在 !");
        whoIsBoxSelection(BoxSelect::SubCombox);
        return;
    }
    if(fileOp_.getFileNameByNum(fullTarPath_, num, currentFile_)){
        if(currentFile_.size() >3 && currentFile_.right(2) == "md"){
            appendTextToLog(u8"当前文档为：" + currentFile_);
        }else{
            appendTextToLog(u8"当前目录为：" + currentFile_);
        }
        setStatusBar("",true);
        whoIsBoxSelection(BoxSelect::NumSpinBox);
    }else{
        setStatusBar("",false);
        currentFile_.clear();
        whoIsBoxSelection(BoxSelect::SubCombox);

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
    currentFile_ = testList.at(1);
    fullTarPath_ = tarPath_ + "/" + subDirName_;
    setStatusBar("", true);
    whoIsBoxSelection(BoxSelect::NumSpinBox);
}

void MainWindow::OpenHistoryFile(){
    ChangeToHistoryFile();
    on_toolPbn_clicked(); // 打开文件
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
    getAssetsDialog_->show();
}

void MainWindow::newConfFileSlot()
{
    QString newFileName;
    if(fileOp_.createJsonFile(confDialog_.getIniFile().iniAndJsonPath, newFileName)){
        openExPro_.OpenMarkdownAndDirSlot(confDialog_.getIniFile().iniAndJsonPath+"/" + newFileName);
    }
}

void MainWindow::setSampleView(){
    ui->tabWgt->hide();
    ui->delList->hide();
    ui->openWgt->hide();
    ui->createMarkdownPbn->hide();
    //    ui->filePbnWgt->hide();
    ui->syncPbn->hide();
    ui->modePbn->hide();
    setSampleViewByScreenRes();
    isStayTop_ = true;
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    show();
}

void MainWindow::setNormalView(){
    ui->tabWgt->show();
    ui->delList->show();
    ui->openWgt->show();
    ui->createMarkdownPbn->show();
    //    ui->filePbnWgt->show();
    ui->syncPbn->show();
    ui->modePbn->show();
    setNormalViewByScreenRes();
    isStayTop_ = false;
    setWindowFlag(Qt::WindowStaysOnTopHint, false);
    show();
}

void MainWindow::simpleViewSlot()
{
    simpleViewNum_++;
    if(simpleViewNum_ % 2 != 0){
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
    aboutDialog_->show();
}

void MainWindow::searchAssetsByCodeSlot(QString code,QString renameList)
{
    QString result;
    fileOp_.getSearchResultFromMarkdownCode(fullTarPath_, code,renameList, result);
}

void MainWindow::on_createMarkdownPbn_clicked()
{
    QString currentFileName;
    if(fileOp_.createMarkdownFile(fullTarPath_, currentFileName)){
        on_lastFileNumPbn_clicked();
        openExPro_.OpenMarkdownAndDirSlot(fullTarPath_+"/" + currentFile_);
    }
}

void MainWindow::on_historySearchPbn_clicked()
{
    QString txt = ui->historySearchLineEditor->text();
    updateRepoHistoryFileListBySearchSlot(txt);
}

void MainWindow::updateRepoHistoryFileListBySearchSlot(QString txt){
    QFileInfoList fileListTop20;
    ui->historyFileList->clear();
    fileOp_.getSearchFileList(tarPath_, fileListTop20, txt);
    ui->historyFileList->setRowCount(fileListTop20.size());        //设置行数/
    for(int i = 0; i < fileListTop20.size(); ++i){
        QFileInfo fileInfo = fileListTop20.at(i);
        QString name = fileInfo.filePath().split(tarPath_).last().remove(0,1);
        QString modifyTime = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss ddd");
        QTableWidgetItem *pItem1 = new QTableWidgetItem(name);
        QTableWidgetItem *pItem2 = new QTableWidgetItem(modifyTime);
        ui->historyFileList->setItem(i, 0, pItem1);
        ui->historyFileList->setItem(i, 1, pItem2);
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
    renameFileName_->setRepoPath(tarPath_);
    renameFileName_->setRenameDirPath(tarPath_);
    renameFileName_->setRenameConfPath(confDialog_.getIniFile().iniAndJsonPath + "/99-CharReplace.json");
    renameFileName_->setRenameListPath(confDialog_.getIniFile().iniAndJsonPath + "/99-RenameList.txt");
    renameFileName_->setSize(getScrrenRes());
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

