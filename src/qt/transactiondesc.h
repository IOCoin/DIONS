#ifndef TRANSACTIONDESC_H
#define TRANSACTIONDESC_H

#include <QString>
#include <QObject>
#include <string>

class __wx__;
class __wx__Tx;

/** Provide a human-readable extended HTML description of a transaction.
 */
class TransactionDesc: public QObject
{
    Q_OBJECT
public:
    static QString toHTML(__wx__ *wallet, __wx__Tx &wtx);
private:
    TransactionDesc() {}

    static QString FormatTxStatus(const __wx__Tx& wtx);
};

#endif // TRANSACTIONDESC_H
