#ifndef INITWORKER_H
#define INITWORKER_H

#include <QObject>
#include <QThread>
#include <QString>
#include<iostream>
#include "iocoingui.h"

class InitWorker : public QObject
{
  Q_OBJECT
  public:
    InitWorker() = default;
    ~InitWorker() = default;

    inline void object(IocoinGUI* obj,QString dir) { this->obj_ = obj; this->dir_ = dir; }
  public slots:
    void initialize();
  signals:
    void completed();

  private:
    IocoinGUI* obj_;
    QString dir_;
};

#endif
