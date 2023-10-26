#include "rename_file_name.h"
#include "ui_rename_file_name.h"

RenameFileName::RenameFileName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameFileName)
{
    ui->setupUi(this);
}

RenameFileName::~RenameFileName()
{
    delete ui;
}
