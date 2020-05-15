#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->rBtnBlue,SIGNAL(clicked()),this,SLOT(setTextFontColor()));
    connect(ui->rBtnRed,SIGNAL(clicked()),this,SLOT(setTextFontColor()));
    connect(ui->rbtnBlack,SIGNAL(clicked()),this,SLOT(setTextFontColor()));

    QObject::connect(ui->btnOk, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect(ui->btnClose, SIGNAL(clicked()), this, SLOT(close()));

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_chkBoxUnder_clicked(bool checked)
{
    QFont font=ui->txtEdit->font();
    font.setUnderline(checked);
    ui->txtEdit->setFont(font);
}

void MainWindow::on_chkBoxItalic_clicked(bool checked)
{
    QFont font=ui->txtEdit->font();
    font.setItalic(checked);
    ui->txtEdit->setFont(font);
}

void MainWindow::on_chkBoxBold_clicked(bool checked)
{
    QFont font=ui->txtEdit->font();
    font.setBold(checked);
    ui->txtEdit->setFont(font);
}

void MainWindow::setTextFontColor()
{
    QPalette plet=ui->txtEdit->palette();
        if (ui->rBtnBlue->isChecked())
            plet.setColor(QPalette::Text,Qt::blue);
        else if (ui->rBtnRed->isChecked())
           plet.setColor(QPalette::Text,Qt::red);
        else if (ui->rbtnBlack->isChecked())
            plet.setColor(QPalette::Text,Qt::black);
        else
           plet.setColor(QPalette::Text,Qt::black);
        ui->txtEdit->setPalette(plet);

}

