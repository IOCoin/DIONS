#ifndef IOCOINGUI_H
#define IOCOINGUI_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QSystemTrayIcon>
#include "actionwatcher.h"
#include "buttoneventhandler.h"
#include "clickablelabel.h"
#include "welcome.h"
#include "intro.h"

#include <stdint.h>

class Welcome;
class Intro;
class IocoinGUI;
class TransactionTableModel;
class ClientModel;
class WalletModel;
class TransactionView;
class OverviewPage;
class SettingsPage;
class AddressBookPage;
class SendCoinsDialog;
class SignVerifyMessageDialog;
class Notificator;
class RPCConsole;
class ButtonHoverWatcher;
class QToolButton;
class QRoundButton;


QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QTableView;
class QAbstractItemModel;
class QModelIndex;
class QProgressBar;
class QStackedWidget;
class QUrl;
QT_END_NAMESPACE

/**
  Bitcoin GUI main class. This class represents the main window of the Bitcoin UI. It communicates with both the client and
  wallet models to give the user an up-to-date view of the current core state.
*/

class IocoinGUI : public QMainWindow
{
    Q_OBJECT
public:
    explicit IocoinGUI(QWidget *parent = 0);
    ~IocoinGUI();

    /** Set the client model.
        The client model represents the part of the core that communicates with the P2P network, and is wallet-agnostic.
    */
    void setClientModel(ClientModel *clientModel);
    /** Set the wallet model.
        The wallet model represents a bitcoin wallet, and offers access to the list of transactions, address book and sending
        functionality.
    */
    void setWalletModel(WalletModel *walletModel);
    void initModel();
    void showIntroScreen();
    void complete_init(QString&);

protected:
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void mouseMoveEvent(QMouseEvent*);
    void changeEvent(QEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void checkBorderDragging(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    //Custom drag cursors
    QCursor* dragtopleft;
    QCursor* dragtopright;
    QCursor* dragbottomright;
    QCursor* dragbottomleft;
    QCursor* draghorizontal;
    QCursor* dragvertical;

  bool leftBorderHit(const QPoint &pos);
  bool rightBorderHit(const QPoint &pos);
  bool topBorderHit(const QPoint &pos);
  bool bottomBorderHit(const QPoint &pos);
  QRect m_StartGeometry;
  const quint8 CONST_DRAG_BORDER_SIZE = 15;
  bool m_bMousePressed;
  bool m_bDragTop;
  bool m_bDragLeft;
  bool m_bDragRight;
  bool m_bDragBottom;
    ClientModel *clientModel;
    WalletModel *walletModel;

    QStackedWidget *introWidget;
    QStackedWidget *centralWidget;

    Welcome      *welcome;
    Intro        *intro;
    OverviewPage *overviewPage;
    SettingsPage *settingsPage;
    QWidget *transactionsPage;
    AddressBookPage *addressBookPage;
    AddressBookPage *receiveCoinsPage;
    SendCoinsDialog *sendCoinsPage;
    SignVerifyMessageDialog *signVerifyMessageDialog;

    ClickableLabel *labelUnencryptedIcon;
    ClickableLabel *labelLockedIcon;
    ClickableLabel *labelUnlockedIcon;

    ClickableLabel *labelMinimizeIcon;
    ClickableLabel *labelMaximizeIcon;
    ClickableLabel *labelCloseIcon;
    QLabel *labelStakingIcon;
    QLabel *labelConnectionsIcon;
    QLabel *labelBlocksIcon;
    QLabel *progressBarLabel;
    QProgressBar *progressBar;

    QIcon* unlockedUnencryptedIcon;
    QIcon* unlockedEncryptedIcon;
    QIcon* lockedEncryptedIcon;

    QMenuBar *appMenuBar;
    QAction *profileImageAction;
    QAction *overviewAction;
    QAction *settingsAction;
    QAction *dionsAction;
    QAction *securegroupsAction;
    QAction *historyAction;
    QAction *quitAction;
    QAction *sendCoinsAction;
    QAction *addressBookAction;
    QAction *signMessageAction;
    QAction *verifyMessageAction;
    QAction *aboutAction;
    QAction *receiveCoinsAction;
    QAction *optionsAction;
    QAction *toggleHideAction;
    QAction *exportAction;
    QAction *encryptWalletAction;
    QAction *backupWalletAction;
    QAction *changePassphraseAction;
    QAction *unlockWalletAction;
    QAction *lockWalletAction;
    QAction *aboutQtAction;
    QAction *openRPCConsoleAction;

    ButtonHoverWatcher* overviewbuttonwatcher;
    ButtonHoverWatcher* sendbuttonwatcher;
    ButtonHoverWatcher* receivebuttonwatcher;
    ButtonHoverWatcher* historybuttonwatcher;
    ButtonHoverWatcher* addressbookbuttonwatcher;
    ButtonHoverWatcher* dionsbuttonwatcher;
    ButtonHoverWatcher* settingsbuttonwatcher;
    ButtonHoverWatcher* securegroupsbuttonwatcher;
    QToolButton* profileimagebutton;
    QToolButton* overviewbutton;
    QToolButton* sendbutton;
    QToolButton* receivebutton;
    QToolButton* historybutton;
    QToolButton* addressbookbutton;
    QToolButton* dionsbutton;
    QToolButton* settingsbutton;
    QToolButton* securegroupsbutton;
    QToolBar* toolbar0;
    QToolBar* toolbar;
    QWidget*  qw;


    QPoint basePos_;

    QSystemTrayIcon *trayIcon;
    Notificator *notificator;
    TransactionView *transactionView;
    RPCConsole *rpcConsole;

    QMovie *syncIconMovie;

    uint64_t nWeight;


    /** Create the main UI actions. */
    void createActions();
    /** Create the menu bar and sub-menus. */
    void createMenuBar();
    /** Create the toolbars */
    void createToolBars();
    /** Create system tray (notification) icon */
    void createTrayIcon();

public slots:
    /** Set number of connections shown in the UI */
    void setNumConnections(int count);
    /** Set number of blocks shown in the UI */
    void setNumBlocks(int count, int nTotalBlocks);
    /** Set the encryption status as shown in the UI.
       @param[in] status            current encryption status
       @see WalletModel::EncryptionStatus
    */
    void setEncryptionStatus(int status);

    /** Notify the user of an error in the network or transaction handling code. */
    void error(const QString &title, const QString &message, bool modal);
    /** Asks the user whether to pay the transaction fee or to cancel the transaction.
       It is currently not possible to pass a return value to another thread through
       BlockingQueuedConnection, so an indirected pointer is used.
       https://bugreports.qt-project.org/browse/QTBUG-10440

      @param[in] nFeeRequired       the required fee
      @param[out] payFee            true to pay the fee, false to not pay the fee
    */
    void askFee(qint64 nFeeRequired, bool *payFee);
    void handleURI(QString strURI);

public slots:
    /** Switch to send coins page */
    void gotoSendCoinsPage();
private slots:
    /** Switch to overview (home) page */
    void gotoProfileImageChooser();
    /** Switch to overview (home) page */
    void gotoWelcomePage();
    void gotoIntroPage();
    void gotoOverviewPage();
    /** Switch to history (transactions) page */
    void gotoHistoryPage();
    /** Switch to address book page */
    void gotoAddressBookPage();
    /** Switch to receive coins page */
    void gotoReceiveCoinsPage();
    /** Switch to settings page */
    void gotoSettingsPage();

    /** Show Sign/Verify Message dialog and switch to sign message tab */
    void gotoSignMessageTab(QString addr = "");
    /** Show Sign/Verify Message dialog and switch to verify message tab */
    void gotoVerifyMessageTab(QString addr = "");

    /** Show configuration dialog */
    void optionsClicked();
    /** Show about dialog */
    void aboutClicked();

//#ifndef Q_OS_MAC
    /** Handle tray icon clicked */
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
//#endif
    /** Show incoming transaction notification for new transactions.

        The new items are those between start and end inclusive, under the given parent item.
    */
    void incomingTransaction(const QModelIndex & parent, int start, int end);
    /** Encrypt the wallet */
    void encryptWallet(bool status);
    /** Backup the wallet */
    void backupWallet();
    /** Change encrypted wallet passphrase */
    void changePassphrase();
    /** Ask for passphrase to unlock wallet temporarily */
    void unlockWallet();

    void lockWallet();

    /** Show window if hidden, unminimize when minimized, rise when obscured or show if hidden and fToggleHidden is true */
    void showNormalIfMinimized(bool fToggleHidden = false);
    /** simply calls showNormalIfMinimized(true) for use in SLOT() macro */
    void toggleHidden();

    void updateWeight();
    void updateStakingIcon();

    void minimizeApp();
    void maximizeApp();
    void closeApp();
};

#endif
