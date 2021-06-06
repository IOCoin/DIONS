#ifndef DELETEWORKER_H
#define DELETEWORKER_H

#include<QObject>
#include<QThread>
#include<QString>
#include<QFileInfo>
#include<QDir>
#include<QFile>
#include<iostream>

class DeleteWorker : public QObject
{
  Q_OBJECT
  public:
    DeleteWorker() = default;
    ~DeleteWorker() = default;

    void init(QFileInfo&);
  public slots:
    void removeFiles();
  signals:
    void completed();

  private:
    QString qfi1_;
    QString qfi2_;
};

#endif
