#ifndef WELCOME_H
#define WELCOME_H

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
  class Welcome;
}

class Welcome : public QWidget 
{
  Q_OBJECT 
  
  public:
    explicit Welcome(QWidget* parent=0);
    ~Welcome();

    void callbackobj(IocoinGUI* obj);

  public slots:
    void gotoIntroScreen();

  private:
    Ui::Welcome* ui;
    IocoinGUI* obj;
    QTimer* timer_;
};

#endif
