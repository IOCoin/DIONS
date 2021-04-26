#include "intro.h"
#include "ui_intro.h"
#include "svgiconengine.h"
#include "clickablelabel.h"
#include "guiconstants.h"
#include<QIcon>
#include<QSize>
#include<QFile>
#include<QFileDialog>
#include<QGraphicsOpacityEffect>

#include<iostream>
std::string logoSVG1 = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" width=\"119.185px\" height=\"116.22px\" viewBox=\"14.148 6.833 119.185 116.22\" enable-background=\"new 14.148 6.833 119.185 116.22\" style=\"fill:white\">"
"  <g>"
"                <path d=\"M133.333,67.107c0,41.16-41.473,52.983-41.473,52.983V99.03c0,0,21.06-7.611,21.873-31.923,c0,0,1.293-24.458-21.873-36.281V10.079C91.86,10.079,133.333,20.129,133.333,67.107z\"/>"
"                <path d=\"M14.148,63.062c0-41.16,41.494-52.983,41.494-52.983v21.06c0,0-21.06,7.611-21.873,31.923,c0,0-1.293,24.458,21.873,36.281v20.747C55.642,120.09,14.148,110.04,14.148,63.062z\"/>"
"                <path d=\"M87.148,46.214v74.689c-12.406,4.837-27.065,0-27.065,0V46.214C60.104,46.214,72.907,40.71,87.148,46.214z\""
"                    />"
"                <path d=\"M87.148,9.308v20.372c-12.469-6.235-27.065,0-27.065,0V9.308C60.104,9.308,73.866,3.74,87.148,9.308z\"/>"
"  </g>"
"</svg>";

std::string closeSVG =
"<?xml version=\"1.0\" standalone=\"no\"?>"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\""
"  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"        x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\">"
"            <g transform=\"translate(0, 0)\">"
"                <line stroke=\"white\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" x1=\"19\" y1=\"5\" x2=\"5\" y2=\"19\" stroke-linejoin=\"miter\"></line>"
"                <line stroke=\"white\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" x1=\"19\" y1=\"19\" x2=\"5\" y2=\"5\" stroke-linejoin=\"miter\"></line>"
"            </g>"
"        </svg>";

Intro::Intro(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Intro)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    this->resize(1000,900);
    ui->setupUi(this);
    ui->vl->setContentsMargins(0,0,0,0);
    ui->vl2->setContentsMargins(280,0,0,0);
    ui->hl->setContentsMargins(0,0,0,0);
    ui->splashframe->setStyleSheet("border-image: url(:/images/splashgradient); background-repeat:no-repeat;");
    QIcon* tmp = new QIcon(new SVGIconEngine(logoSVG1));
    QPixmap logoPixmap = tmp->pixmap(tmp->actualSize(QSize(200,200)));
    ui->logoleft->setPixmap(logoPixmap);
    ui->title->setStyleSheet("color:white; font-size:48px; font-weight:300");
    ui->title->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->closeicon->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->logoleft->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->logolefttext->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->logolefttext->setStyleSheet("color:white; font-size:48px; font-weight:400");
    QGraphicsOpacityEffect* e = new QGraphicsOpacityEffect(this);
    e->setOpacity(0.6);
    ui->logolefttext->setGraphicsEffect(e);
    ui->vlconfigselect->setContentsMargins(0,0,340,200);
    ui->logoright->setAttribute(Qt::WA_TranslucentBackground, true);
    QString importLabelText = "<p style=\"font-size:24pt; color:white\">"
	                      "New / Import wallet"
			      "</p>"
                              "<p style=\"font-size:12pt; color:rgba(255,255,255,100)\">"
	                      "Select new wallet directory to load from"
			      "</p>";
    
    ui->logoright->setText(importLabelText);
    ui->logoright->setCursor(QCursor(Qt::PointingHandCursor));
    ui->closeicon->setObjectName("closelabel");
    QFile qssFile3(":/icons/closesplash");
    qssFile3.open(QFile::ReadOnly);
    QString svg = QLatin1String(qssFile3.readAll());
    QIcon* closeIc = new QIcon(new SVGIconEngine(svg.toStdString()));
    QIcon* tmp1 = new QIcon(new SVGIconEngine(closeSVG));
    QPixmap closePixmap = tmp1->pixmap(tmp1->actualSize(QSize(30,30)));
    ui->closeicon->setPixmap(closePixmap);
    ui->closeicon->setCursor(QCursor(Qt::PointingHandCursor));

    connect(ui->logoright,SIGNAL(clicked(bool)),this, SLOT(config()));
    connect(ui->closeicon,SIGNAL(clicked(bool)),this, SLOT(closesplash()));
}

Intro::~Intro()
{
  delete ui;
}

void Intro::closesplash()
{
  qApp->quit();
}
void Intro::config()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose wallet directory"),
                                             "",
                                             QFileDialog::ShowDirsOnly
                                             | QFileDialog::DontResolveSymlinks);

    std::cout << "selected " << dir.toStdString() << std::endl;
    if(dir.toStdString() == "") return;

    obj->complete_init(dir);
}

void Intro::callbackobj(IocoinGUI* obj)
{
  this->obj = obj;
}
