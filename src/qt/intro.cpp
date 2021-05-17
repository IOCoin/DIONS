#include "intro.h"
#include "downloader.h"
#include "archiveextractor.h"
#include "ui_intro.h"
#include "svgiconengine.h"
#include "clickablelabel.h"
#include "guiconstants.h"
#include "initworker.h"
#include <boost/filesystem.hpp>
#include "JlCompress.h"
#include<QIcon>
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

const char* BOOTSTRAP_URL =
"https://iobootstrap.s3.amazonaws.com/bootstrap-02052021-07:20:23.zip";

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

void extract(QFileInfo fileDest,Ui::Intro** ui)
{
  QStringList zextracted = JlCompress::extractDir(fileDest.filePath(), fileDest.path(), (*ui)->progressbar);
  if(!zextracted.isEmpty())
  {
    std::cout << "Bootstrap extract successful." << std::endl;
  }
}
void initialize(IocoinGUI* obj,QString dir)
{
  obj->complete_init(dir);
}

Intro::Intro(QWidget *parent) :
    QWidget(parent),
    url(BOOTSTRAP_URL),
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
    ui->bootstrapdialog->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->bootstrapdialog->setStyleSheet("color:white; font-size: 14pt");
    ui->bootstrapdialog->setText("Chosen wallet directory empty,\n download bootstrap.dat now?");
    ui->bootstrapdialog->setCursor(QCursor(Qt::PointingHandCursor));
    ui->bootstrapdialog->hide();
    ui->next->setCursor(QCursor(Qt::PointingHandCursor));
    ui->next->hide();
    ui->progressbar->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->remaintime->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->remaintime->setStyleSheet("color:white; font-size: 12pt");
    ui->next->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->next->setStyleSheet("color:white; font-size: 12pt");
    ui->progressbar->hide();
    ui->remaintime->hide();

    ui->progressbar->setRange(0,1000000);
    ui->progressbar->setFixedWidth(300);

    connect(ui->bootstrapdialog,SIGNAL(clicked(bool)),this, SLOT(downloadbootstrap()));
    connect(ui->next,SIGNAL(clicked(bool)),this, SLOT(next()));
    connect(ui->logoright,SIGNAL(clicked(bool)),this, SLOT(config()));
    connect(ui->closeicon,SIGNAL(clicked(bool)),this, SLOT(closesplash()));

    downloadTimer = new QTimer(this);
    remainTimer.setInterval(1000);
    connect(downloadTimer, SIGNAL(timeout()), this, SLOT(timerCheckDownloadProgress()));
    connect(&remainTimer, SIGNAL(timeout()), this, SLOT(calculateRemainTime()));
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
  dir_ = QFileDialog::getExistingDirectory(this, tr("Choose wallet directory"),
    "",
    QFileDialog::ShowDirsOnly
    | QFileDialog::DontResolveSymlinks);

  if(dir_.toStdString() == "") 
    return;

  {
    bool not_initialized = true;
    boost::filesystem::path selected_directory(dir_.toStdString());
    selected_directory /= "blk0001.dat";
    if(!boost::filesystem::exists(selected_directory))
    {
      boost::filesystem::path p(dir_.toStdString());
      p /= "bootstrap.zip";
      this->setDest(p.string());
      ui->logoright->hide();
      ui->bootstrapdialog->show();
    }
    else
    {
      InitWorker* worker = new InitWorker();
      worker->object(this->obj,dir_);
      QThread* thread = new QThread();
      worker->moveToThread(thread);

      connect(thread,SIGNAL(started()),worker,SLOT(initialize()));
      connect(worker,SIGNAL(completed()),this,SLOT(initModel()));
      connect(worker,SIGNAL(completed()),worker,SLOT(deleteLater()));
      connect(thread,SIGNAL(finished()),thread,SLOT(deleteLater()));
      thread->start();
    }
  }
}

void Intro::initModel()
{
  this->obj->initModel();
}

void Intro::callbackobj(IocoinGUI* obj)
{
  this->obj = obj;
}

void Intro::next()
{
  obj->complete_init(dir_);
}

void Intro::downloadbootstrap()
{
  ui->bootstrapdialog->hide();
  ui->progressbar->show();
  ui->remaintime->show();
  startDownload();
}
void Intro::startRequest()
{
  downloadFinished = false;

  // Start the timer
  downloadTimer->start(30000);
  startDownloadingStatsRecording();

  // get() method posts a request
  // to obtain the contents of the target request
  // and returns a new QNetworkReply object
  // opened for reading which emits
  // the readyRead() signal whenever new data arrives.
  reply = manager->get(QNetworkRequest(url));
  reply->ignoreSslErrors();

  // Whenever more data is received from the network,
  // this readyRead() signal is emitted
  connect(reply, SIGNAL(readyRead()),
    this, SLOT(httpReadyRead()));

  // Also, downloadProgress() signal is emitted when data is received
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
    this, SLOT(updateDownloadProgress(qint64,qint64)));

  connect(reply, &QNetworkReply::downloadProgress, [this]() {
    this->downloadTimer->stop();
    this->downloadTimer->start();
  });

  // This signal is emitted when the reply has finished processing.
  // After this signal is emitted,
  // there will be no more updates to the reply's data or metadata.
  connect(reply, SIGNAL(finished()),
    this, SLOT(downloaderFinished()));

  // Network error
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
    this, SLOT(networkError()));

}
void Intro::startDownloadingStatsRecording()
{
  currentTotalBytes = 0;
  currentBytesRead = 0;
  currentSpeed = 0;
  last30secsSpeed.clear();
  last60secsSpeed.clear();
  remainTimer.start();
  downloadTime.start();
}
void Intro::httpReadyRead()
{
  if (file)
    file->write(reply->readAll());
}
void Intro::downloaderFinished()
{
    // Finished with timer
    if (downloadTimer->isActive())
    {
        downloadTimer->stop();
        remainTimer.stop();
    }

    // when canceled
    if (httpRequestAborted)
    {
        if (file)
        {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        return;
    }

    // get redirection url
    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    // download finished normally
    file->flush();
    file->close();

    if (reply->error())
    {
        file->remove();
        {
            QMessageBox::information(this, tr("Downloader"),
                                 tr("Download terminated: %1.").arg(reply->errorString()));
        }
    }
    else
    {
        if (!redirectionTarget.isNull())
        {
            QUrl newUrl = url.resolved(redirectionTarget.toUrl());
            url = newUrl;
            reply->deleteLater();
            file->open(QIODevice::WriteOnly);
            file->resize(0);
            startRequest();
            return;
        }
        else
        {
            ui->remaintime->setText("bootstrap.zip download successful, extracting...");
      QThread* t = QThread::create([this]{extract(fileDest,&ui);});
      t->start();
      connect(t,SIGNAL(finished()),this,SLOT(extractioncomplete()));
            //ui->remaintime->hide();
            //ui->next->show();
            //ui->next->setText(tr("Successfully extracted. Click to continue..."));
        }
    }

    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;
    manager = 0;
    downloadFinished = true;

}
void Intro::fileUnzipped()
{
  ui->remaintime->hide();
  ui->next->show();
  ui->next->setText(tr("Successfully extracted. Click to continue..."));
}
void Intro::updateDownloadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;

    currentTotalBytes = totalBytes;
    currentBytesRead = bytesRead;

    double ratio = static_cast<double>(bytesRead) / static_cast<double>(totalBytes);
    ui->progressbar->setValue(ratio * 1000000);

    // calculate the download speed
    currentSpeed = bytesRead * 1000.0 / downloadTime.elapsed();
}
void Intro::timerCheckDownloadProgress()
{
  if (!downloadFinished)
  {
      qDebug()<<__PRETTY_FUNCTION__<<": stall";
      cancelDownload();
  }
}
void Intro::networkError()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(sender());
    qDebug()<<__PRETTY_FUNCTION__<<':'<<reply->error();
    if (!downloaderQuit)
        cancelDownload();
}
void Intro::cancelDownload()
{
    if (downloadTimer->isActive())
    {
        downloadTimer->stop();
        remainTimer.stop();
    }

    if (!reply->errorString().isEmpty())
    {
        //XXXX ui->statusLabel->setText(tr("The download was canceled.\n\n%1").arg(reply->errorString()));
    }
    else
    {
        //XXXX ui->statusLabel->setText(tr("The download was canceled."));
    }
    if (reply)
    {
        reply->abort();
    }
    httpRequestAborted = true;
}
void Intro::calculateRemainTime()
{
  last30secsSpeed.push_back(currentSpeed);
  last60secsSpeed.push_back(currentSpeed);

  if(last30secsSpeed.length() > 30)
    last30secsSpeed.dequeue();

  if(last60secsSpeed.length() > 60)
    last60secsSpeed.dequeue();

  double avg30 = 0;
  for(double s : last30secsSpeed)
  {
    avg30 += s;
  }
  avg30 /= last30secsSpeed.size();

  double avg60 = 0;
  for(double s : last60secsSpeed)
  {
    avg60 += s;
  }
  avg60 /= last60secsSpeed.size();

  double speed = currentSpeed * 0.5 + avg30 * 0.3 + avg60 * 0.2;
  int remainSecs = (currentTotalBytes - currentBytesRead) / speed;

  QTime time(0, 0);
  time = time.addSecs(remainSecs);

  double s = currentSpeed;
  QString unit;
  if (currentSpeed < 1024) 
  {
    unit = "bytes/sec";
  } 
  else if(currentSpeed < 1024*1024) 
  {
    s /= 1024;
    unit = "kB/s";
  } 
  else 
  {
    s /= 1024*1024;
    unit = "MB/s";
  }

  if(remainSecs < 0)
    ui->remaintime->setText(tr("%1 %2 downloading, In N/A").arg(s, 3, 'f', 1).arg(unit));
  else
    ui->remaintime->setText(tr("%1 %2 downloading, %3 mins remaining").arg(s, 3, 'f', 1).arg(unit).arg(time.toString("mm:ss")));
}
void Intro::setDest(std::string dest)
{
  QString d = QString::fromStdString(dest);
  this->setDest(d);
}
void Intro::setDest(QString dest)
{
  fileDest = QFileInfo(dest);

    if (fileDest.exists())
    {
      //XXXX ui->status->setText(tr("The file \"%1\" already exists.\n\nPress 'Next' to continue with this file, or 'Download' to get a new one.").arg(fileDest.filePath()));
      ui->progressbar->setMaximum(100);
      ui->progressbar->setValue(100);
      downloadFinished = true;
    }
    else
    {
      //XXXX ui->statusLabel->setText(tr("Press 'Download' or 'Next' to begin."));
      //XXXX ui->continueButton->setEnabled(false);
    }
}
void Intro::startDownload()
{
    downloadFinished = false;


    QFileInfo fileInfo(url.path());
    QString fileName = fileInfo.fileName();

    if (fileName.isEmpty())
    {
            QMessageBox::information(this, tr("Downloader"),
                      tr("Filename cannot be empty.")
                      );
        return;
    }

    if (!fileDest.fileName().isEmpty())
    {
        fileName = fileDest.filePath();
    }
    fileDest = QFileInfo(fileName);

    if (fileDest.exists())
    {
        {
            if (QMessageBox::question(this, tr("Downloader"),
                tr("The file \"%1\" already exists. Overwrite it?").arg(fileName),
                QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
                == QMessageBox::No)
            {
                downloadFinished = true;
                ui->progressbar->setMaximum(100);
                ui->progressbar->setValue(100);
                return;
            }
        }
        QFile::remove(fileName);
    }

    manager = new QNetworkAccessManager(this);

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly))
    {
        {
            QMessageBox::information(this, tr("Downloader"),
                      tr("Unable to save the file \"%1\": %2.")
                      .arg(fileName).arg(file->errorString()));
        }
        delete file;
        file = 0;
        return;
    }

    // These will be set true when Cancel/Continue/Quit pressed
    downloaderQuit = false;
    httpRequestAborted = false;

    ui->progressbar->setValue(0);

    // download button disabled after requesting download.

    startRequest();
}
void Intro::extractioncomplete() 
{
            ui->remaintime->hide();
            ui->next->show();
            ui->next->setText(tr("Successfully extracted. Click to continue..."));
}
//#https://iobootstrap.s3.amazonaws.com/IOC-BOOTSTRAP-3242602.zip
