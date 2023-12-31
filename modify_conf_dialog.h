#ifndef MODIFY_CONF_DIALOG_H
#define MODIFY_CONF_DIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QTableWidgetItem>
#include "data_type.h"
#include "conf_data.h"

namespace Ui {
class ModifyConfDialog;
}

class ModifyConfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModifyConfDialog(QWidget *parent = nullptr);
    ~ModifyConfDialog();
    void setConfigData(ConfigData data) {
        configdata_ = data;
    }
    void initWindow();
    void initWindowsSize();
    void showWindow();
    void updateAssetsDirList();
    void updateRepoList();

    void updateAssetsType();

    void updateSoftWarePath();
    void updateMeetingPath();

    void setScreenRes(ScreenRes res) {
        screenRes_ = res;
    }

private slots:
    void on_addAssetsDir_clicked();

    void on_delAssetsDir_clicked();

    void on_addRepoDir_clicked();

    void on_delRepoDir_clicked();

    void on_addAssetsTypePbn_clicked();

    void on_delAssetsTypePbn_clicked();
    void updateAssetsTypeListEditSlot(QListWidgetItem *current);
    void updateRepoListEditSlot(QTableWidgetItem *item);
    void updateAssetsDirListEditSlot(QTableWidgetItem *item);

    void on_markdownSoftPbn_clicked();

    void on_dataDirSoftPbn_clicked();

    void on_yesPbn_clicked();

    void on_cancelPbn_clicked();

    void on_applyPbn_clicked();

signals:
    void sigModifyConfDlgLog(QString log);

    void sigModifyConfigData(ConfigData data);


private:
    Ui::ModifyConfDialog *ui;

    ConfigData configdata_;

    ScreenRes screenRes_;
    int multiple_;
};

#endif // MODIFY_CONF_DIALOG_H
