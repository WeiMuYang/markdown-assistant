#ifndef MODIFY_CONF_DIALOG_H
#define MODIFY_CONF_DIALOG_H

#include <QDialog>
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
    void setConfigData(ConfigData* data) {
        configdata_ = data;
    }
    void initWindow();
    void showWindow();
    void updateAssetsDirList();
private slots:
    void on_addAssetsDir_clicked();

private:
    Ui::ModifyConfDialog *ui;

    ConfigData* configdata_;
};

#endif // MODIFY_CONF_DIALOG_H
