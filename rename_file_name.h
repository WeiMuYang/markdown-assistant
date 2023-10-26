#ifndef RENAME_FILE_NAME_H
#define RENAME_FILE_NAME_H

#include <QDialog>

namespace Ui {
class RenameFileName;
}

class RenameFileName : public QDialog
{
    Q_OBJECT

public:
    explicit RenameFileName(QWidget *parent = nullptr);
    ~RenameFileName();

private:
    Ui::RenameFileName *ui;
};

#endif // RENAME_FILE_NAME_H
