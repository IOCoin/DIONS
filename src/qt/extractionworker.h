#ifndef EXTRACTIONWORKER_H
#define EXTRACTIONWORKER_H

#include <QObject>
#include <QThread>
#include <QString>
#include <QProgressBar>
#include <QFileInfo>
#include<iostream>

class ExtractionWorker : public QObject
{
  Q_OBJECT
  public:
    ExtractionWorker() = default;
    ~ExtractionWorker() = default;

    void init(QFileInfo,QProgressBar*);
  public slots:
    void extract();
  signals:
    void progress(int);
    void min(int);
    void max(int);
    void extractionempty();
    void completed();

  private:
    QFileInfo qfi_;
    QProgressBar* pb_;
};

#endif
