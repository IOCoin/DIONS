#ifndef IONSPAYMENTPROCESSOR_H
#define IONSPAYMENTPROCESSOR_H

#include <QtGui>

class IONSPaymentProcessor;

#include "bitcoingui.h"

class IONSPaymentProcessor : public QObject
{
    Q_OBJECT
  public:
    IONSPaymentProcessor(BitcoinGUI * gui, QObject * parent=0);

  public slots:
    Q_INVOKABLE
      void pay(QString address, QString fee);
    Q_INVOKABLE
      void myUsernames();

  private:
    BitcoinGUI * gui;

};

#endif
