#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "walletmodel.h"
#include "iocoingui.h"
#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QQueue>
#include <QDir>
#include <QMessageBox>
#include <QTimer>

//class JlCompress;

namespace Ui {
class Downloader;
}

class Downloader : public QDialog
{
    Q_OBJECT

public:
    explicit Downloader(QWidget *parent = 0);
    ~Downloader();

private:
    void showEvent(QShowEvent *e);
    void startRequest(QUrl url);
    Ui::Downloader *ui;
    QUrl url;
    QFileInfo fileDest;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
    QTimer *downloadTimer;
    QTimer remainTimer;
    QTime downloadTime;
    QFile *file;
    qint64 fileSize;

    //these variables are used to calculate download speed and estimated time to
    //download finish
    double currentSpeed;
    QQueue<double> last30secsSpeed;
    QQueue<double> last60secsSpeed;
    quint64 currentTotalBytes;
    quint64 currentBytesRead;
    void startDownloadingStatsRecording();

public:
    void setUrl(QUrl source);
    void setUrl(std::string source); // overload
    void setDest(QString dest);
    void setDest(std::string dest); // overload
    void callbackobj(IocoinGUI* obj, QString dir);

    // These will be set true when Cancel/Continue/Quit pressed
    bool httpRequestAborted;
    bool downloaderQuit;
    bool downloadFinished;

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_downloadButton_clicked();
    void on_quitButton_clicked();
    void on_continueButton_clicked();
    void on_urlEdit_returnPressed();

    // slot for readyRead() signal
    void httpReadyRead();

    // slot for finished() signal from reply
    void downloaderFinished();

    // slot for downloadProgress()
    void updateDownloadProgress(qint64, qint64);
    void timerCheckDownloadProgress();

    void enableDownloadButton();
    void networkError();
    void cancelDownload();
    void on_confCheckBox_clicked(bool checked);
    void calculateRemainTime();

  private:
    IocoinGUI* obj;
    QString dir;
};

#endif // DOWNLOADER_H
