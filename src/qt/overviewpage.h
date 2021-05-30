#ifndef OVERVIEWPAGE_H
#define OVERVIEWPAGE_H

#include <QWidget>
#include "transactionview.h"
#include "iocoingui.h"

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
    class OverviewPage;
}
class WalletModel;
class TxViewDelegate;
class TransactionFilterProxy;

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(IocoinGUI*,QWidget *parent = 0);
    ~OverviewPage();

    void setModel(WalletModel *model);
    void showOutOfSyncWarning(bool fShow);

    void unencrypted(); 
    void locked() ;
    void unlockedstakingonly() ;

public slots:
    void setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance);
    void gotoSendPage();

signals:
    void transactionClicked(const QModelIndex &index);

private:
    WalletModel *model;
    qint64 currentBalance;
    qint64 currentStake;
    qint64 currentUnconfirmedBalance;
    qint64 currentImmatureBalance;

    bool unencrypted_;
    bool locked_;
    bool unlockedstakeonly_;

    
    TransactionView* txv_;
    TxViewDelegate *txdelegate;
    TransactionFilterProxy *filter;

private slots:
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
private:
    Ui::OverviewPage *ui;
    IocoinGUI* iocgui_;
};

#endif // OVERVIEWPAGE_H
