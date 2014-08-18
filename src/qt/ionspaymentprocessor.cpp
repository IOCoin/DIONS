#include "ionspaymentprocessor.h"
#include <QString>
#include <iostream>

IONSPaymentProcessor::IONSPaymentProcessor(BitcoinGUI * gui, QObject * parent)
    : QObject(parent), gui(gui)
{
}

void IONSPaymentProcessor::pay(QString address, QString fee)
{
    QString uri("iocoin:" + address + "?amount=" + fee);
    gui->handleURI(uri);
}

void IONSPaymentProcessor::myUsernames()
{
    gui->ionsMyUsernamesClicked();
}

void IONSPaymentProcessor::Register()
{
    gui->ionsRegisterClicked();
}
