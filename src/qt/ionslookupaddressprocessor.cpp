#include "ionslookupaddressprocessor.h"
#include <QString>
#include <QDialog>
#include <iostream>

IONSLookupAddressProcessor::IONSLookupAddressProcessor(QDialog * ionsDialog, SendCoinsEntry * entry, QObject * parent)
    : QObject(parent), ionsDialog(ionsDialog), entry(entry)
{
}

void IONSLookupAddressProcessor::setAddress(QString address)
{
    entry->setPaymentAddress(address);
    ionsDialog->accept();
}
