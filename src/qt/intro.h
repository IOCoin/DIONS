#ifndef INTRO_H
#define INTRO_H

#include "iocoingui.h"
#include<QWidget>
#include<QPixmap>
#include<QQueue>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QFileInfo>
#include<QTimer>

class IocoinGUI;

namespace Ui {
  class Intro;
}

//class SplashScreen : public QWidget 
class Intro : public QWidget 
{
  Q_OBJECT 
  
  public:
    explicit Intro(QWidget* parent=0);
    void setPixmap(QPixmap& p) { p_=p; }
    ~Intro();

    void callbackobj(IocoinGUI* obj);

  public slots:
    void hidewelcome();
    void extractioncomplete();
    void next();
    void downloadbootstrap();
    void config();
    void closesplash();
    void fileUnzipped();
    void initModel();

    //bootstrap download handling
    void hideall();
    void setDest(std::string);
    void setDest(QString);
    void startDownload();
    void startRequest();
    void httpReadyRead();
    void downloaderFinished();
    void updateDownloadProgress(qint64, qint64);
    void timerCheckDownloadProgress();
    void networkError();
    void cancelDownload();
    void calculateRemainTime();

  private:
    Ui::Intro* ui;
    QPixmap p_;
    IocoinGUI* obj;
    QString dir_;

    QThread* thread_;
    bool httpRequestAborted;
    bool downloaderQuit;
    bool downloadFinished;
    double currentSpeed;
    QUrl url;
    QFileInfo fileDest;
    QTimer *downloadTimer;
    QTime downloadTime;
    QTimer remainTimer;
    QNetworkAccessManager *manager;
    QMovie* movie_;
    QNetworkReply *reply;
    QQueue<double> last30secsSpeed;
    QQueue<double> last60secsSpeed;
    QFile *file;
    qint64 fileSize;
    quint64 currentTotalBytes;
    quint64 currentBytesRead;
    void startDownloadingStatsRecording();
};

#endif
