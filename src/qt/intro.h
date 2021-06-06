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
#ifdef __APPLE__
  #include"OSXHideTitleBar.h"
#endif

class IocoinGUI;

namespace Ui {
  class Intro;
}


//class SplashScreen : public QWidget 
class Intro : public QWidget 
{
  Q_OBJECT 
  
  public:
    explicit Intro(IocoinGUI*,QWidget* parent=0);
    void setPixmap(QPixmap& p) { p_=p; }
    ~Intro();

    void callbackobj(IocoinGUI* obj);

  public slots:
    void extractioncomplete();
    void extractionempty();
    void next();
    void downloadbootstrap();
    void directoryemptydialog();
    void config();
    void closesplash();
    void fileUnzipped();
    void initModel();
    void minimizeApp();
    void maximizeApp();

    //bootstrap download handling
    void hideall();
    void setBase(std::string);
    void setDest(std::string);
    void setBase(QString);
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

  protected:
    void closeEvent(QCloseEvent *event);

  private:
    Ui::Intro* ui;
    QPixmap p_;
    IocoinGUI* obj;
    QString dir_;
    IocoinGUI* iocgui_;


    QThread* thread_;
    bool httpRequestAborted;
    bool downloaderQuit;
    bool downloadFinished;
    double currentSpeed;
    QUrl url;
    QFileInfo fileBase;
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
