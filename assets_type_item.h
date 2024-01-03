#ifndef ASSETS_TYPE_ITEM_H
#define ASSETS_TYPE_ITEM_H

#include <QDialog>

namespace Ui {
class AssetsTypeItem;
}

class AssetsTypeItem : public QDialog
{
    Q_OBJECT

public:
    explicit AssetsTypeItem(QWidget *parent = nullptr);
    ~AssetsTypeItem();
    void showWindow();
    void initWindowSize();
    void setWidth(int w) {
        width_ = w;
    }
signals:
    void sigAssetsType(QString type);
    void sigAssetsTypeLog(QString msg);
private slots:
    void on_okPbn_clicked();

private:
    Ui::AssetsTypeItem *ui;
    int width_;
};

#endif // ASSETS_TYPE_ITEM_H
