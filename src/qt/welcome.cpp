#include "welcome.h"
#include "downloader.h"
#include "archiveextractor.h"
#include "ui_welcome.h"
#include "svgiconengine.h"
#include "clickablelabel.h"
#include "guiconstants.h"
#include "initworker.h"
#include <boost/filesystem.hpp>
#include "JlCompress.h"
#include<QIcon>
#include<QMovie>
#include<QSize>
#include<QFile>
#include<QThread>
#include<QFileDialog>
#include<QGraphicsOpacityEffect>

#include<iostream>
#include<boost/iostreams/filtering_streambuf.hpp>
#include<boost/iostreams/copy.hpp>
#include<boost/iostreams/filter/zlib.hpp>
#include<boost/iostreams/filter/gzip.hpp>

Welcome::Welcome(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Welcome)
{
	std::cout << "Welcone" << std::endl;
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    this->resize(1000,900);
    ui->setupUi(this);

    QMovie* movie = new QMovie(":/movies/welcome", "gif", this);
    ui->movie->setMovie(movie);
    timer_ = new QTimer(this);
    connect(timer_,SIGNAL(timeout()),this,SLOT(gotoIntroScreen()));
    ui->movie->show();
    movie->start();
    timer_->start(8000);
}

Welcome::~Welcome()
{
  delete ui;
}

void Welcome::gotoIntroScreen()
{
	timer_->stop();
  this->obj->showIntroScreen();
}

void Welcome::callbackobj(IocoinGUI* obj)
{
  this->obj = obj;
}

//#https://iobootstrap.s3.amazonaws.com/IOC-BOOTSTRAP-3242602.zip
