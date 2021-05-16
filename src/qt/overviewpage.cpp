#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "clickablelabel.h"
#include "walletmodel.h"
#include "iocoingui.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "transactionfilterproxy.h"
#include "guiutil.h"
#include "guiconstants.h"
#include <iostream>
#include <QAbstractItemDelegate>
#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include "svgiconengine.h"

#define DECORATION_SIZE 64
#define NUM_ITEMS 3

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(IocoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = IocoinUnits::formatWithUnit(unit, amount, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};
#include "overviewpage.moc"

OverviewPage::OverviewPage(IocoinGUI* i,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    currentBalance(-1),
    iocgui_(i),
    currentStake(0),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    txdelegate(new TxViewDelegate()),
    filter(0)
{
    ui->setupUi(this);

    txv_ = new TransactionView(this);
    txv_->setObjectName("txv");

    connect(ui->sendbutton,SIGNAL(clicked()),SLOT(gotoSendPage()));

    // Recent transactions
    //TRANS ui->listTransactions->setItemDelegate(txdelegate);
    //TRANS ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    //TRANS ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    //TRANS ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    //TRANS connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    QGraphicsDropShadowEffect* ef = new QGraphicsDropShadowEffect();
    ef->setBlurRadius(10);
    ef->setXOffset(2);
    ef->setYOffset(2);
    //ef->setColor(Qt::black);
    QColor col = QColor("#d3d3d3");
    //ef->setColor(Qt::gray);
    ef->setColor(col);
    ui->available->setGraphicsEffect(ef);
    QGraphicsDropShadowEffect* ef2 = new QGraphicsDropShadowEffect();
    ef2->setBlurRadius(10);
    ef2->setXOffset(2);
    ef2->setYOffset(2);
    ef2->setColor(col);
    ui->pending->setGraphicsEffect(ef2);
    QGraphicsDropShadowEffect* ef3 = new QGraphicsDropShadowEffect();
    ef3->setBlurRadius(10);
    ef3->setXOffset(2);
    ef3->setYOffset(2);
    ef3->setColor(col);
    ui->staked->setGraphicsEffect(ef3);

    // init "out of sync" warning labels
    //ui->labelWalletStatus->setText("(" + tr("out of sync") + ")");
    //ui->labelTransactionsStatus->setText("(" + tr("out of sync") + ")");

    ui->vl->setContentsMargins(0 , 0, 0, 0);
    ui->statusGL->setSpacing(0);
    ui->statusGL->setContentsMargins(10 , 0, 10, 0);
    ui->statusHB->setContentsMargins(0 , 0, 0, 0);
    ui->statusGL->setColumnStretch(0,0);

    ui->labelIntBalance->setMargin(0);
    ui->labelIntBalance->sizePolicy().setHorizontalStretch(1);
    ui->labelIntBalance->setContentsMargins(0,0,0,0);
    ui->labelFracBalance->setMargin(0);

    ui->unconfirmedGL->setSpacing(0);
    ui->unconfirmedGL->setContentsMargins(10 , 0, 10, 0);
    ui->unconfirmedGL->setColumnStretch(0,0);
    ui->labelIntUnconfirmed->setMargin(0);
    ui->labelIntUnconfirmed->sizePolicy().setHorizontalStretch(1);
    ui->labelIntUnconfirmed->setContentsMargins(0,0,0,0);
    ui->labelFracUnconfirmed->setMargin(0);

    ui->stakedGL->setSpacing(0);
    ui->stakedGL->setContentsMargins(10 , 0, 10, 0);
    ui->stakedGL->setColumnStretch(0,0);
    ui->labelIntStaked->setMargin(0);
    ui->labelIntStaked->sizePolicy().setHorizontalStretch(1);
    ui->labelIntStaked->setContentsMargins(0,0,0,0);
    ui->labelFracStaked->setMargin(0);

    ui->hl4->setContentsMargins(10 , 0, 10, 0);

    ui->available->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->pending->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    ui->staked->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);

    //ui->availableAmountLayout->setMargin(0);
    //ui->availableAmountLayout->setSpacing(0);
    //QVBoxLayout *vbox = new QVBoxLayout();
    //QLabel* tmp = new QLabel();
    //tmp->setText("hello");
    //vbox->addWidget(txv_);
    //vbox->addWidget(tmp);
    //ui->txv->setLayout(vbox);

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
}

void OverviewPage::gotoSendPage()
{
  iocgui_->gotoSendCoinsPage();
}

void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        emit transactionClicked(filter->mapToSource(index));
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelIntBalance->setText(IocoinUnits::intFormatWithUnit(unit, balance));
    ui->labelFracBalance->setText(IocoinUnits::fracFormatWithUnit(unit, balance));
    ui->labelIntStaked->setText(IocoinUnits::intFormatWithUnit(unit, stake));
    ui->labelFracStaked->setText(IocoinUnits::fracFormatWithUnit(unit, stake));
    ui->labelIntUnconfirmed->setText(IocoinUnits::intFormatWithUnit(unit, unconfirmedBalance));
    ui->labelFracUnconfirmed->setText(IocoinUnits::fracFormatWithUnit(unit, unconfirmedBalance));

    std::string check = "<svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"        x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\">"
"            <g transform=\"translate(0, 0)\">"
"                <circle fill=\"#45e283\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" cx=\"12\" cy=\"12\" r=\"11\" stroke-linejoin=\"miter\"></circle>"
"                <circle fill=\"white\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" cx=\"12\" cy=\"12\" r=\"9\" stroke-linejoin=\"miter\"></circle>"
"                <polyline data-color=\"color-2\" fill=\"#45e283\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\" 6,12 10,16 18,8 \" stroke-linejoin=\"miter\"></polyline>"
"                <polyline data-color=\"color-2\" fill=\"white\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\" 6,9 10,13 18,5 \" stroke-linejoin=\"miter\"></polyline>"
"            </g>"
"        </svg>";
    std::string pending = "<?xml version=\"1.0\" standalone=\"no\"?>"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\""
"  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"        x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\">"
"            <g transform=\"translate(0, 0)\">"
"                <circle fill=\"#ffcd9d\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" cx=\"12\" cy=\"12\" r=\"11\" stroke-linejoin=\"miter\"></circle>"
"                <circle fill=\"white\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" cx=\"12\" cy=\"12\" r=\"9\" stroke-linejoin=\"miter\"></circle>"
"                <polyline data-color=\"color-2\" fill=\"#ffcd9d\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"12,6 12,12 18,12\" stroke-linejoin=\"miter\"></polyline>"
"                <polyline data-color=\"color-2\" fill=\"white\" stroke-width=\"2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"14,4 14,10 20,10\" stroke-linejoin=\"miter\"></polyline>"
"            </g>"
"        </svg>";

    QIcon i = QIcon(new SVGIconEngine(check));
    QPixmap p = i.pixmap(i.actualSize(QSize(48,48)));
    //p.setPixmap(new SVGIconEngine(check)); 
    ui->balanceIcon->setPixmap(p);
    QIcon pend = QIcon(new SVGIconEngine(pending));
    QPixmap p1 = pend.pixmap(pend.actualSize(QSize(48,48)));
    ui->pendingIcon->setPixmap(p1);
    QIcon stakedIcon = QIcon(new SVGIconEngine(pending));
    QPixmap p2 = stakedIcon.pixmap(stakedIcon.actualSize(QSize(48,48)));
    ui->stakedIcon->setPixmap(p2);
    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
}

void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;

    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter = new TransactionFilterProxy();
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Status, Qt::DescendingOrder);

        //TRANS ui->listTransactions->setModel(filter);
        //TRANS ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }
    txv_->setModel(model);
    QVBoxLayout *vbox = new QVBoxLayout();
    vbox->addWidget(txv_);
    ui->txv->setLayout(vbox);
    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        //TRANS ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    //ui->labelWalletStatus->setVisible(fShow);
    //ui->labelTransactionsStatus->setVisible(fShow);
}
