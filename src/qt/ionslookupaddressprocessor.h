#ifndef IONSLOOKUPADDRESSPROCESSOR_H
#define IONSLOOKUPADDRESSPROCESSOR_H

#include <QtGui>

class IONSLookupAddressProcessor;

#include "sendcoinsentry.h"

class IONSLookupAddressProcessor : public QObject
{
    Q_OBJECT
  public:
    IONSLookupAddressProcessor(QDialog * ionsDialog, SendCoinsEntry * entry, QObject * parent=0);

  public slots:
    Q_INVOKABLE
      void setAddress(QString ionsName, QString address);

  private:
    QDialog * ionsDialog;
    SendCoinsEntry * entry;

};

#endif
