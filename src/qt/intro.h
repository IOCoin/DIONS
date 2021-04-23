#ifndef INTRO_H
#define INTRO_H

#include "iocoingui.h"
#include<QWidget>
#include<QPixmap>
//#include<QSplashScreen>

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
    void config();
    void closesplash();

  private:
    Ui::Intro* ui;
    QPixmap p_;
    IocoinGUI* obj;
};

#endif
