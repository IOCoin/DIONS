#include "settingspage.h"
#include "ui_settingspage.h"

#include "transactionfilterproxy.h"
#include "transactionrecord.h"
#include "walletmodel.h"
#include "addresstablemodel.h"
#include "transactiontablemodel.h"
#include "bitcoinunits.h"
#include "csvmodelwriter.h"
#include "transactiondescdialog.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "guiutil.h"

#include <QScrollBar>
#include <QComboBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QPoint>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QDateTimeEdit>

SettingsPage::SettingsPage(QWidget *parent) :
    QWidget(parent), model(0), transactionProxyModel(0),
    ui(new Ui::SettingsPage),
    settingsView(0)
{
    ui->setupUi(this);
    ui->verticalLayout->setContentsMargins(0 , 0, 0, 0);
    ui->labelPageTitle->setStyleSheet("background-color: white; font-size: 34px; color:#b3b3b3; font-weight:300");
}

void SettingsPage::setModel(WalletModel *model)
{
    this->model = model;
}

void SettingsPage::chooseDate(int idx)
{
    if(!transactionProxyModel)
        return;
    QDate current = QDate::currentDate();
    dateRangeWidget->setVisible(false);
    switch(dateWidget->itemData(idx).toInt())
    {
    case All:
        transactionProxyModel->setDateRange(
                TransactionFilterProxy::MIN_DATE,
                TransactionFilterProxy::MAX_DATE);
        break;
    case Today:
        transactionProxyModel->setDateRange(
                QDateTime(current),
                TransactionFilterProxy::MAX_DATE);
        break;
    case ThisWeek: {
        // Find last Monday
        QDate startOfWeek = current.addDays(-(current.dayOfWeek()-1));
        transactionProxyModel->setDateRange(
                QDateTime(startOfWeek),
                TransactionFilterProxy::MAX_DATE);

        } break;
    case ThisMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month(), 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case LastMonth:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), current.month()-1, 1)),
                QDateTime(QDate(current.year(), current.month(), 1)));
        break;
    case ThisYear:
        transactionProxyModel->setDateRange(
                QDateTime(QDate(current.year(), 1, 1)),
                TransactionFilterProxy::MAX_DATE);
        break;
    case Range:
        dateRangeWidget->setVisible(true);
        dateRangeChanged();
        break;
    }
}

void SettingsPage::chooseType(int idx)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setTypeFilter(
        typeWidget->itemData(idx).toInt());
}

void SettingsPage::changedPrefix(const QString &prefix)
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setAddressPrefix(prefix);
}

void SettingsPage::changedAmount(const QString &amount)
{
    if(!transactionProxyModel)
        return;
    qint64 amount_parsed = 0;
    if(IocoinUnits::parse(model->getOptionsModel()->getDisplayUnit(), amount, &amount_parsed))
    {
        transactionProxyModel->setMinAmount(amount_parsed);
    }
    else
    {
        transactionProxyModel->setMinAmount(0);
    }
}

void SettingsPage::exportClicked()
{
    // CSV is currently the only supported format
    QString filename = GUIUtil::getSaveFileName(
            this,
            tr("Export Transaction Data"), QString(),
            tr("Comma separated file (*.csv)"));

    if (filename.isNull()) return;

    CSVModelWriter writer(filename);

    // name, column, role
    writer.setModel(transactionProxyModel);
    writer.addColumn(tr("Confirmed"), 0, TransactionTableModel::ConfirmedRole);
    writer.addColumn(tr("Date"), 0, TransactionTableModel::DateRole);
    writer.addColumn(tr("Type"), TransactionTableModel::Type, Qt::EditRole);
    writer.addColumn(tr("Label"), 0, TransactionTableModel::LabelRole);
    writer.addColumn(tr("Address"), 0, TransactionTableModel::AddressRole);
    writer.addColumn(tr("Amount"), 0, TransactionTableModel::FormattedAmountRole);
    writer.addColumn(tr("ID"), 0, TransactionTableModel::TxIDRole);

    if(!writer.write())
    {
        QMessageBox::critical(this, tr("Error exporting"), tr("Could not write to file %1.").arg(filename),
                              QMessageBox::Abort, QMessageBox::Abort);
    }
}

void SettingsPage::contextualMenu(const QPoint &point)
{
    QModelIndex index = settingsView->indexAt(point);
    if(index.isValid())
    {
        contextMenu->exec(QCursor::pos());
    }
}

void SettingsPage::copyAddress()
{
    GUIUtil::copyEntryData(settingsView, 0, TransactionTableModel::AddressRole);
}

void SettingsPage::copyLabel()
{
    GUIUtil::copyEntryData(settingsView, 0, TransactionTableModel::LabelRole);
}

void SettingsPage::copyAmount()
{
    GUIUtil::copyEntryData(settingsView, 0, TransactionTableModel::FormattedAmountRole);
}

void SettingsPage::copyTxID()
{
    GUIUtil::copyEntryData(settingsView, 0, TransactionTableModel::TxIDRole);
}

void SettingsPage::editLabel()
{
    if(!settingsView->selectionModel() ||!model)
        return;
    QModelIndexList selection = settingsView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        AddressTableModel *addressBook = model->getAddressTableModel();
        if(!addressBook)
            return;
        QString address = selection.at(0).data(TransactionTableModel::AddressRole).toString();
        if(address.isEmpty())
        {
            // If this transaction has no associated address, exit
            return;
        }
        // Is address in address book? Address book can miss address when a transaction is
        // sent from outside the UI.
        int idx = addressBook->lookupAddress(address);
        if(idx != -1)
        {
            // Edit sending / receiving address
            QModelIndex modelIdx = addressBook->index(idx, 0, QModelIndex());
            // Determine type of address, launch appropriate editor dialog type
            QString type = modelIdx.data(AddressTableModel::TypeRole).toString();

            EditAddressDialog dlg(type==AddressTableModel::Receive
                                         ? EditAddressDialog::EditReceivingAddress
                                         : EditAddressDialog::EditSendingAddress,
                                  this);
            dlg.setModel(addressBook);
            dlg.loadRow(idx);
            dlg.exec();
        }
        else
        {
            // Add sending address
            EditAddressDialog dlg(EditAddressDialog::NewSendingAddress,
                                  this);
            dlg.setModel(addressBook);
            dlg.setAddress(address);
            dlg.exec();
        }
    }
}

void SettingsPage::showDetails()
{
    if(!settingsView->selectionModel())
        return;
    QModelIndexList selection = settingsView->selectionModel()->selectedRows();
    if(!selection.isEmpty())
    {
        TransactionDescDialog dlg(selection.at(0));
        dlg.exec();
    }
}

QWidget *SettingsPage::createDateRangeWidget()
{
    dateRangeWidget = new QFrame();
    dateRangeWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    dateRangeWidget->setContentsMargins(1,1,1,1);
    QHBoxLayout *layout = new QHBoxLayout(dateRangeWidget);
    layout->setContentsMargins(0,0,0,0);
    layout->addSpacing(23);
    layout->addWidget(new QLabel(tr("Range:")));

    dateFrom = new QDateTimeEdit(this);
    dateFrom->setDisplayFormat("dd/MM/yy");
    dateFrom->setCalendarPopup(true);
    dateFrom->setMinimumWidth(100);
    dateFrom->setDate(QDate::currentDate().addDays(-7));
    layout->addWidget(dateFrom);
    layout->addWidget(new QLabel(tr("to")));

    dateTo = new QDateTimeEdit(this);
    dateTo->setDisplayFormat("dd/MM/yy");
    dateTo->setCalendarPopup(true);
    dateTo->setMinimumWidth(100);
    dateTo->setDate(QDate::currentDate());
    layout->addWidget(dateTo);
    layout->addStretch();

    // Hide by default
    dateRangeWidget->setVisible(false);

    // Notify on change
    connect(dateFrom, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));
    connect(dateTo, SIGNAL(dateChanged(QDate)), this, SLOT(dateRangeChanged()));

    return dateRangeWidget;
}

void SettingsPage::dateRangeChanged()
{
    if(!transactionProxyModel)
        return;
    transactionProxyModel->setDateRange(
            QDateTime(dateFrom->date()),
            QDateTime(dateTo->date()).addDays(1));
}

void SettingsPage::focusTransaction(const QModelIndex &idx)
{
    if(!transactionProxyModel)
        return;
    QModelIndex targetIdx = transactionProxyModel->mapFromSource(idx);
    settingsView->scrollTo(targetIdx);
    settingsView->setCurrentIndex(targetIdx);
    settingsView->setFocus();
}
SettingsPage::~SettingsPage()
{
    delete ui;
}
