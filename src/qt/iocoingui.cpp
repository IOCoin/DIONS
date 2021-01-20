/*
 * Qt4 bitcoin GUI.
 *
 * W.J. van der Laan 2011-2012
 * The Iocoin Developers 2011-2012
 */
#include "iocoingui.h"
#include "transactiontablemodel.h"
#include "addressbookpage.h"
#include "sendcoinsdialog.h"
#include "signverifymessagedialog.h"
#include "optionsdialog.h"
#include "aboutdialog.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "editaddressdialog.h"
#include "optionsmodel.h"
#include "transactiondescdialog.h"
#include "addresstablemodel.h"
#include "transactionview.h"
#include "overviewpage.h"
#include "dionspage.h"
#include "iocoinunits.h"
#include "guiconstants.h"
#include "askpassphrasedialog.h"
#include "notificator.h"
#include "guiutil.h"
#include "rpcconsole.h"
#include "wallet.h"

#include "SvgIconEngine.h"

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QIcon>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QToolButton>
#include <QStatusBar>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QStackedWidget>
#include <QDateTime>
#include <QMovie>
#include <QFileDialog>
#include <QDesktopServices>
#include <QTimer>
#include <QDragEnterEvent>
#include <QUrl>
#include <QStyle>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QWebFrame>
#include <QWidgetAction>

#include <iostream>


extern __wx__* pwalletMain;
extern int64_t nLastCoinStakeSearchInterval;
double GetPoSKernelPS(int nHeight = -1);

//        <svg  x="0px" y="0px" width="119.185px" height="116.22px" viewBox="14.148 6.833 119.185 116.22" enable-background="new 14.148 6.833 119.185 116.22" class="icon icon--glyph">
//            <g>
//                <path d="M133.333,67.107c0,41.16-41.473,52.983-41.473,52.983V99.03c0,0,21.06-7.611,21.873-31.923
//                    c0,0,1.293-24.458-21.873-36.281V10.079C91.86,10.079,133.333,20.129,133.333,67.107z"/>
//                <path d="M14.148,63.062c0-41.16,41.494-52.983,41.494-52.983v21.06c0,0-21.06,7.611-21.873,31.923
//                    c0,0-1.293,24.458,21.873,36.281v20.747C55.642,120.09,14.148,110.04,14.148,63.062z"/>
//                <path d="M87.148,46.214v74.689c-12.406,4.837-27.065,0-27.065,0V46.214C60.104,46.214,72.907,40.71,87.148,46.214z"
//                    />
//                <path d="M87.148,9.308v20.372c-12.469-6.235-27.065,0-27.065,0V9.308C60.104,9.308,73.866,3.74,87.148,9.308z"/>
//            </g>
//        </svg>


    string logoSVG = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" width=\"119.185px\" height=\"116.22px\" viewBox=\"14.148 6.833 119.185 116.22\" enable-background=\"new 14.148 6.833 119.185 116.22\" style=\"fill:white\">"
"  <g>"
"                <path d=\"M133.333,67.107c0,41.16-41.473,52.983-41.473,52.983V99.03c0,0,21.06-7.611,21.873-31.923,c0,0,1.293-24.458-21.873-36.281V10.079C91.86,10.079,133.333,20.129,133.333,67.107z\"/>"
"                <path d=\"M14.148,63.062c0-41.16,41.494-52.983,41.494-52.983v21.06c0,0-21.06,7.611-21.873,31.923,c0,0-1.293,24.458,21.873,36.281v20.747C55.642,120.09,14.148,110.04,14.148,63.062z\"/>"
"                <path d=\"M87.148,46.214v74.689c-12.406,4.837-27.065,0-27.065,0V46.214C60.104,46.214,72.907,40.71,87.148,46.214z\""
"                    />"
"                <path d=\"M87.148,9.308v20.372c-12.469-6.235-27.065,0-27.065,0V9.308C60.104,9.308,73.866,3.74,87.148,9.308z\"/>"
"  </g>"
"</svg>";
    string overviewSVGUnchecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"    <polygon points=\"2,9.6 2,24 9,24 9,17 15,17 15,24 22,24 22,9.6 12,0.7 \"></polygon>"
"  </g>"
"</svg>";
    string sendSVGUnchecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"                <path d=\"M1.4,11l2.2,1.6l6.4-3c0.5-0.2,1,0.4,0.5,0.8l-4.5,4v6.7c0,0.9,1.2,1.4,1.8,0.6l3.1-3.7l6.5,4.9 c0.6,0.4,1.4,0.1,1.6-0.6l4-20c0.2-0.8-0.6-1.4-1.4-1.1l-20,8C0.9,9.6,0.8,10.6,1.4,11z\"></path>"
"  </g>"
"</svg>";
    string receiveSVGUnchecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"            <g transform=\"translate(0, 0)\"> "
"                <line data-cap=\"butt\" stroke-miterlimit=\"10\" x1=\"12\" y1=\"3\" x2=\"12\" y2=\"17\" stroke-linejoin=\"miter\" stroke-linecap=\"butt\"></line> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"5,10 19,10 19,8 5,8 \" stroke-linejoin=\"miter\"></polyline> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"7,12 12,17 17,12 \" stroke-linejoin=\"miter\"></polyline> "
"                <line data-color=\"color-2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" x1=\"19\" y1=\"21\" x2=\"5\" y2=\"21\" stroke-linejoin=\"miter\"></line> "
"            </g> "
"</svg>";
    string historySVGUnchecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"                <path data-color=\"color-2\" d=\"M21,16H8v-4l-8,6l8,6v-4h13c0.6,0,1-0.4,1-1v-2C22,16.4,21.6,16,21,16z\"></path> "
"                <path d=\"M16,12l8-6l-8-6v4H3C2.4,4,2,4.4,2,5v2c0,0.6,0.4,1,1,1h13V12z\"></path> "
"  </g>"
"</svg>";
    string addressbookSVGUnchecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"  <g>"
"                <path d=\"M15.6,8L12,0.2L8.4,8H0.2l6,6.2l-2.3,9.3l8.1-4.6l8.1,4.6l-2.3-9.3l6-6.2H15.6z\"></path>"
"  </g>"
"  <text x=\"10\" y=\"10\">Dions</text> "
"</svg>";
    string dionsSVGUnchecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#646464 \">"
"   <g> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"7.5,0 10,0 9.5,24 7,24\" stroke-linejoin=\"miter\"></polyline>  "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"10.5,0 13,0 12.5,24 10,24\" stroke-linejoin=\"miter\"></polyline>  "
"   </g> "
"   <text x=\"0\" y=\"21\" font-size=\"27\">D</text> "
"</svg>";


// receive
//        <svg x="0px" y="0px" viewBox="0 0 24 24" class="icon icon--outline"> 
 //           <g transform="translate(0, 0)"> 
 //               <line data-cap="butt" stroke-miterlimit="10" x1="12" y1="3" x2="12" y2="17" stroke-linejoin="miter" stroke-linecap="butt"></line> 
 //               <polyline stroke-linecap="square" stroke-miterlimit="10" points="7,12 12,17 17,12 " stroke-linejoin="miter"></polyline> 
 //               <line data-color="color-2" stroke-linecap="square" stroke-miterlimit="10" x1="19" y1="21" x2="5" y2="21" stroke-linejoin="miter"></line> 
 //           </g> 
 //       </svg>

IocoinGUI::IocoinGUI(QWidget *parent):
    QMainWindow(parent),
    clientModel(0),
    walletModel(0),
    encryptWalletAction(0),
    changePassphraseAction(0),
    unlockWalletAction(0),
    lockWalletAction(0),
    aboutQtAction(0),
    trayIcon(0),
    notificator(0),
    rpcConsole(0),
    nWeight(0)
{
    resize(850, 550);
    setWindowTitle(tr("I/OCoin") + " - " + tr("Wallet"));
#ifndef Q_OS_MAC
    qApp->setWindowIcon(QIcon(":icons/bitcoin"));
    setWindowIcon(QIcon(":icons/bitcoin"));
    //setStyleSheet("background-color:#fffdd0"); //cream
    //setStyleSheet("background-color:#f8f8ff ; QToolTip { background-color:black }");
    QFile qssFile(":qss/stylesheet");
    qssFile.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(qssFile.readAll());
    setStyleSheet(styleSheet);
    //setStyleSheet("background-color:#1aa8ea ");
#else
    setUnifiedTitleAndToolBarOnMac(true);
    QApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
#endif
    // Accept D&D of URIs
    setAcceptDrops(true);

    // Create actions for the toolbar, menu bar and tray/dock icon
    createActions();

    // Create application menu bar
    createMenuBar();

    // Create the toolbars
    createToolBars();

    // Create the tray icon (or setup the dock icon)
    createTrayIcon();

    // Create tabs
    overviewPage = new OverviewPage();
    dionsPage = new DIONSPage();

    transactionsPage = new QWidget(this);
    QVBoxLayout *vbox = new QVBoxLayout();
    transactionView = new TransactionView(this);
    vbox->addWidget(transactionView);
    transactionsPage->setLayout(vbox);

    addressBookPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::SendingTab);

    receiveCoinsPage = new AddressBookPage(AddressBookPage::ForEditing, AddressBookPage::ReceivingTab);

    sendCoinsPage = new SendCoinsDialog(this);

    signVerifyMessageDialog = new SignVerifyMessageDialog(this);


    centralWidget = new QStackedWidget(this);
    centralWidget->addWidget(overviewPage);
    centralWidget->addWidget(transactionsPage);
    centralWidget->addWidget(addressBookPage);
    centralWidget->addWidget(receiveCoinsPage);
    centralWidget->addWidget(sendCoinsPage);
    centralWidget->addWidget(dionsPage);
    setCentralWidget(centralWidget);

    // Create status bar
    //statusBar();

    // Status bar notification icons
    QFrame *frameBlocks = new QFrame(this);
    frameBlocks->setContentsMargins(0,0,0,0);
    frameBlocks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout *frameBlocksLayout = new QHBoxLayout(frameBlocks);
    frameBlocksLayout->setContentsMargins(3,0,3,0);
    frameBlocksLayout->setSpacing(3);
    labelEncryptionIcon = new QLabel();
    labelStakingIcon = new QLabel();
    labelConnectionsIcon = new QLabel();
    labelBlocksIcon = new QLabel();
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelEncryptionIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelStakingIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelConnectionsIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->addWidget(labelBlocksIcon);
    frameBlocksLayout->addStretch();
    frameBlocksLayout->setAlignment(Qt::AlignRight);
    QWidget* qw = new QWidget(this);
    qw->setObjectName("statusbar");
    //STYLE qw->setStyleSheet("background-color:#1aa8ea ; QToolTip { background-color:black } ");
    QHBoxLayout* l = new QHBoxLayout(qw);
    l->addWidget(appMenuBar);
    l->addWidget(frameBlocks);
    l->insertStretch(1,100);
    setMenuWidget(qw);

    if (GetBoolArg("-staking", true))
    {
        QTimer *timerStakingIcon = new QTimer(labelStakingIcon);
        connect(timerStakingIcon, SIGNAL(timeout()), this, SLOT(updateStakingIcon()));
        timerStakingIcon->start(30 * 1000);
        updateStakingIcon();
    }

    // Progress bar and label for blocks download
    progressBarLabel = new QLabel();
    progressBarLabel->setVisible(false);
    progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignCenter);
    progressBar->setVisible(false);

    // Override style sheet for progress bar for styles that have a segmented progress bar,
    // as they make the text unreadable (workaround for issue #1071)
    // See https://qt-project.org/doc/qt-4.8/gallery.html
    QString curStyle = qApp->style()->metaObject()->className();
    if(curStyle == "QWindowsStyle" || curStyle == "QWindowsXPStyle")
    {
        //STYLE progressBar->setStyleSheet("QProgressBar { background-color: #e8e8e8; border: 1px solid grey; border-radius: 7px; padding: 1px; text-align: center; } QProgressBar::chunk { background: QLinearGradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #FF8000, stop: 1 orange); border-radius: 7px; margin: 0px; }");
    }

    //statusBar()->addWidget(progressBarLabel);
    //statusBar()->addWidget(progressBar);
    //statusBar()->addPermanentWidget(frameBlocks);

    syncIconMovie = new QMovie(":/movies/update_spinner", "mng", this);

    // Clicking on a transaction on the overview page simply sends you to transaction history page
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), this, SLOT(gotoHistoryPage()));
    connect(overviewPage, SIGNAL(transactionClicked(QModelIndex)), transactionView, SLOT(focusTransaction(QModelIndex)));

    // Double-clicking on a transaction on the transaction history page shows details
    connect(transactionView, SIGNAL(doubleClicked(QModelIndex)), transactionView, SLOT(showDetails()));

    rpcConsole = new RPCConsole(this);
    connect(openRPCConsoleAction, SIGNAL(triggered()), rpcConsole, SLOT(show()));

    // Clicking on "Verify Message" in the address book sends you to the verify message tab
    connect(addressBookPage, SIGNAL(verifyMessage(QString)), this, SLOT(gotoVerifyMessageTab(QString)));
    // Clicking on "Sign Message" in the receive coins page sends you to the sign message tab
    connect(receiveCoinsPage, SIGNAL(signMessage(QString)), this, SLOT(gotoSignMessageTab(QString)));

    gotoOverviewPage();
}

IocoinGUI::~IocoinGUI()
{
    if(trayIcon) // Hide tray icon, as deleting will let it linger until quit (on Ubuntu)
        trayIcon->hide();
#ifdef Q_OS_MAC
    delete appMenuBar;
#endif
}

void IocoinGUI::createActions()
{
    QActionGroup *tabGroup = new QActionGroup(this);

    QIcon overviewicon = QIcon(new SVGIconEngine(overviewSVGUnchecked));
    overviewAction = new QAction(this);
    overviewAction->setText(tr("&Overview"));
    overviewAction->setIcon(overviewicon);
    overviewAction->setToolTip(tr("Show general overview of wallet"));
    overviewAction->setCheckable(true);
    overviewAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_1));
    tabGroup->addAction(overviewAction);

    QIcon sendicon = QIcon(new SVGIconEngine(sendSVGUnchecked));
    sendCoinsAction = new QAction(this);
    sendCoinsAction->setText(tr("&Send coins"));
    sendCoinsAction->setIcon(sendicon);
    sendCoinsAction->setToolTip(tr("Send coins to a I/OCoin address"));
    sendCoinsAction->setCheckable(true);
    sendCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_2));
    tabGroup->addAction(sendCoinsAction);

    QIcon receiveicon = QIcon(new SVGIconEngine(receiveSVGUnchecked));
    receiveCoinsAction = new QAction(this);
    receiveCoinsAction->setText(tr("&Receive coins"));
    receiveCoinsAction->setIcon(receiveicon);
    receiveCoinsAction->setToolTip(tr("Show the list of addresses for receiving payments"));
    receiveCoinsAction->setCheckable(true);
    receiveCoinsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_3));
    tabGroup->addAction(receiveCoinsAction);

    
    QIcon historyicon = QIcon(new SVGIconEngine(historySVGUnchecked));
    historyAction = new QAction(this);
    historyAction->setText(tr("&Transactions"));
    historyAction->setIcon(historyicon);
    historyAction->setToolTip(tr("Browse transaction history"));
    historyAction->setCheckable(true);
    historyAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_4));
    tabGroup->addAction(historyAction);

    QIcon addressbookicon = QIcon(new SVGIconEngine(addressbookSVGUnchecked));
    addressBookAction = new QAction(this);
    addressBookAction->setText(tr("&Address Book"));
    addressBookAction->setIcon(addressbookicon);
    addressBookAction->setToolTip(tr("Edit the list of stored addresses and labels"));
    addressBookAction->setCheckable(true);
    addressBookAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(addressBookAction);

    QIcon dionsicon = QIcon(new SVGIconEngine(dionsSVGUnchecked));
    dionsAction = new QAction(this);
    dionsAction->setText(tr("&Dions"));
    dionsAction->setIcon(dionsicon);
    dionsAction->setToolTip(tr("DIONS management page"));
    dionsAction->setCheckable(true);
    dionsAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_5));
    tabGroup->addAction(dionsAction);


    connect(overviewAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(overviewAction, SIGNAL(triggered()), this, SLOT(gotoOverviewPage()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(sendCoinsAction, SIGNAL(triggered()), this, SLOT(gotoSendCoinsPage()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(receiveCoinsAction, SIGNAL(triggered()), this, SLOT(gotoReceiveCoinsPage()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(historyAction, SIGNAL(triggered()), this, SLOT(gotoHistoryPage()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(addressBookAction, SIGNAL(triggered()), this, SLOT(gotoAddressBookPage()));
    connect(dionsAction, SIGNAL(triggered()), this, SLOT(showNormalIfMinimized()));
    connect(dionsAction, SIGNAL(triggered()), this, SLOT(gotoDIONSPage()));

    quitAction = new QAction(QIcon(":/icons/quit"), tr("E&xit"), this);
    quitAction->setToolTip(tr("Quit application"));
    quitAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));
    quitAction->setMenuRole(QAction::QuitRole);
    aboutAction = new QAction(QIcon(":/icons/bitcoin"), tr("&About I/OCoin"), this);
    aboutAction->setToolTip(tr("Show information about I/OCoin"));
    aboutAction->setMenuRole(QAction::AboutRole);
    aboutQtAction = new QAction(QIcon(":/trolltech/qmessagebox/images/qtlogo-64.png"), tr("About &Qt"), this);
    aboutQtAction->setToolTip(tr("Show information about Qt"));
    aboutQtAction->setMenuRole(QAction::AboutQtRole);
    optionsAction = new QAction(QIcon(":/icons/options"), tr("&Options..."), this);
    optionsAction->setToolTip(tr("Modify configuration options for I/OCoin"));
    optionsAction->setMenuRole(QAction::PreferencesRole);
    toggleHideAction = new QAction(QIcon(":/icons/bitcoin"), tr("&Show / Hide"), this);
    encryptWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Encrypt Wallet..."), this);
    encryptWalletAction->setToolTip(tr("Encrypt or decrypt wallet"));
    encryptWalletAction->setCheckable(true);
    backupWalletAction = new QAction(QIcon(":/icons/filesave"), tr("&Backup Wallet..."), this);
    backupWalletAction->setToolTip(tr("Backup wallet to another location"));
    changePassphraseAction = new QAction(QIcon(":/icons/key"), tr("&Change Passphrase..."), this);
    changePassphraseAction->setToolTip(tr("Change the passphrase used for wallet encryption"));
    unlockWalletAction = new QAction(QIcon(":/icons/lock_open"), tr("&Unlock Wallet..."), this);
    unlockWalletAction->setToolTip(tr("Unlock wallet"));
    lockWalletAction = new QAction(QIcon(":/icons/lock_closed"), tr("&Lock Wallet"), this);
    lockWalletAction->setToolTip(tr("Lock wallet"));
    signMessageAction = new QAction(QIcon(":/icons/edit"), tr("Sign &message..."), this);
    verifyMessageAction = new QAction(QIcon(":/icons/transaction_0"), tr("&Verify message..."), this);

    exportAction = new QAction(QIcon(":/icons/export"), tr("&Export..."), this);
    exportAction->setToolTip(tr("Export the data in the current tab to a file"));
    openRPCConsoleAction = new QAction(QIcon(":/icons/debugwindow"), tr("&Debug window"), this);
    openRPCConsoleAction->setToolTip(tr("Open debugging and diagnostic console"));

    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutClicked()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(optionsAction, SIGNAL(triggered()), this, SLOT(optionsClicked()));
    connect(toggleHideAction, SIGNAL(triggered()), this, SLOT(toggleHidden()));
    connect(encryptWalletAction, SIGNAL(triggered(bool)), this, SLOT(encryptWallet(bool)));
    connect(backupWalletAction, SIGNAL(triggered()), this, SLOT(backupWallet()));
    connect(changePassphraseAction, SIGNAL(triggered()), this, SLOT(changePassphrase()));
    connect(unlockWalletAction, SIGNAL(triggered()), this, SLOT(unlockWallet()));
    connect(lockWalletAction, SIGNAL(triggered()), this, SLOT(lockWallet()));
    connect(signMessageAction, SIGNAL(triggered()), this, SLOT(gotoSignMessageTab()));
    connect(verifyMessageAction, SIGNAL(triggered()), this, SLOT(gotoVerifyMessageTab()));
}

void IocoinGUI::createMenuBar()
{
#ifdef Q_OS_MAC
    // Create a decoupled menu bar on Mac which stays even if the window is closed
    appMenuBar = new QMenuBar();
#else
    // Get the main window's menu bar on other platforms
    appMenuBar = menuBar();
#endif

    //appMenuBar->setStyleSheet("background-color: #1ee8ea ");
    // Configure the menus
    //XXXX QMenu *file = appMenuBar->addMenu(tr("&File"));
    //XXXX file->addAction(backupWalletAction);
    //XXXX file->addAction(exportAction);
    //XXXX file->addAction(signMessageAction);
    //XXXX file->addAction(verifyMessageAction);
    //XXXX file->addSeparator();
    //XXXX file->addAction(quitAction);

    //XXXX QMenu *settings = appMenuBar->addMenu(tr("&Settings"));
    //XXXX settings->addAction(encryptWalletAction);
    //XXXX settings->addAction(changePassphraseAction);
    //XXXX settings->addAction(unlockWalletAction);
    //XXXX settings->addAction(lockWalletAction);
    //XXXX settings->addSeparator();
    //XXXX settings->addAction(optionsAction);

    //XXXX QMenu *help = appMenuBar->addMenu(tr("&Help"));
    //XXXX help->addAction(openRPCConsoleAction);
    //XXXX help->addSeparator();
    //XXXX help->addAction(aboutAction);
    //XXXX help->addAction(aboutQtAction);
}

void IocoinGUI::createToolBars()
{
    //logo area QToolBar *toolbar0 = addToolBar("label");
    //logo area toolbar0->setToolButtonStyle(Qt::ToolButtonIconOnly);
    //logo area addToolBar(Qt::LeftToolBarArea,toolbar0);
    //logo area toolbar0->setObjectName("label");
    //logo area QIcon i1 = QIcon(new SVGIconEngine(logoSVG));
    //logo area QToolButton* pic = new QToolButton();
    //logo area pic->setIcon(i1);
    //logo area toolbar0->addWidget(pic);
    //logo area pic->setObjectName("logo");
    //logo area pic->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //logo area pic->setEnabled(false);

    // profile image area QToolBar *toolbar1 = addToolBar("profile");
    // profile image area toolbar1->setToolButtonStyle(Qt::ToolButtonIconOnly);
    // profile image area addToolBar(Qt::LeftToolBarArea,toolbar1);
    // profile image area toolbar1->setObjectName("profile");
    // profile image area QIcon i2 = QIcon(new SVGIconEngine(logoSVG));
    // profile image area QToolButton* pic1 = new QToolButton();
    // profile image area pic1->setIcon(i2);
    // profile image area toolbar1->addWidget(pic1);
    // profile image area pic1->setObjectName("profileimage");
    // profile image area pic1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //pic1->setEnabled(false);

    QToolBar *toolbar = addToolBar(tr("Tabs toolbar"));
    addToolBar(Qt::LeftToolBarArea,toolbar);
    toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addAction(overviewAction);
    toolbar->addAction(sendCoinsAction);
    toolbar->addAction(receiveCoinsAction);
    toolbar->addAction(historyAction);
    toolbar->addAction(addressBookAction);
    toolbar->addAction(dionsAction);

    overviewbutton = static_cast<QToolButton*>(toolbar->widgetForAction(overviewAction));
    overviewbutton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    overviewbuttonwatcher = new ButtonHoverWatcher(this,overviewbutton);
    overviewbutton->installEventFilter(overviewbuttonwatcher);

    sendbutton = static_cast<QToolButton*>(toolbar->widgetForAction(sendCoinsAction));
    sendbutton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    sendbuttonwatcher = new ButtonHoverWatcher(this,sendbutton);
    sendbutton->installEventFilter(sendbuttonwatcher);

    receivebutton = static_cast<QToolButton*>(toolbar->widgetForAction(receiveCoinsAction));
    receivebutton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    receivebuttonwatcher = new ButtonHoverWatcher(this,receivebutton);
    receivebutton->installEventFilter(receivebuttonwatcher);

    historybutton = static_cast<QToolButton*>(toolbar->widgetForAction(historyAction));
    historybutton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    historybuttonwatcher = new ButtonHoverWatcher(this,historybutton);
    historybutton->installEventFilter(historybuttonwatcher);

    addressbookbutton = static_cast<QToolButton*>(toolbar->widgetForAction(addressBookAction));
    addressbookbutton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    addressbookbuttonwatcher = new ButtonHoverWatcher(this,addressbookbutton);
    addressbookbutton->installEventFilter(addressbookbuttonwatcher);

    dionsbutton = static_cast<QToolButton*>(toolbar->widgetForAction(dionsAction));
    dionsbutton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    dionsbuttonwatcher = new ButtonHoverWatcher(this,dionsbutton);
    dionsbutton->installEventFilter(dionsbuttonwatcher);

    QLayout* tl = toolbar->layout();
    for(int i=0;i<tl->count();i++) { 
	    //XXXX tl->itemAt(i)->setAlignment(Qt::AlignLeft);
	    tl->itemAt(i)->setAlignment(Qt::AlignCenter);
    }
    // profile image area QLayout* tlprofile = toolbar1->layout();
    // profile image area for(int i=0;i<tlprofile->count();i++) { 
// profile image area 	    tlprofile->itemAt(i)->setAlignment(Qt::AlignCenter);
    // profile image area }
 
    //STYLE toolbar->setStyleSheet("QToolBar {background-color: black; color:#646464; } QToolButton { background-color:black ; font-size:18px; border: 2px; text-align: left } QToolButton:hover { color: #bebebe } QToolButton:checked { color: #1aa8ea ; } QToolButtonText { text-align: left } QToolButton#logo { background-color:#1aa8ea; color:white; border:0px; margin:0px } QToolTip { background-color:black }");
    //logo area toolbar0->setStyleSheet("QToolBar#label {background-color: #1aa8ea; color:white; } QToolButton#logo { background-color:black }");
    // profile image area toolbar1->setStyleSheet("QToolBar#profile {background-color: #1aa8ea; color:white; } QToolButton#profileimage { background-color:black; margin:10px; border-width:4px; border-style: solid; border-color:red; border-radius:50px ; max-width:100px; max-height:100px; min-width:100px; min-height:100px }");
    //logo area toolbar0->setMovable(false);
    // profile image upload  toolbar1->setMovable(false);
    toolbar->setMovable(false);
}

void IocoinGUI::setClientModel(ClientModel *clientModel)
{
    this->clientModel = clientModel;
    if(clientModel)
    {
        // Replace some strings and icons, when using the testnet
        if(clientModel->isTestNet())
        {
            setWindowTitle(windowTitle() + QString(" ") + tr("[testnet]"));
#ifndef Q_OS_MAC
            qApp->setWindowIcon(QIcon(":icons/bitcoin_testnet"));
            setWindowIcon(QIcon(":icons/bitcoin_testnet"));
#else
            MacDockIconHandler::instance()->setIcon(QIcon(":icons/bitcoin_testnet"));
#endif
            if(trayIcon)
            {
                trayIcon->setToolTip(tr("I/OCoin client") + QString(" ") + tr("[testnet]"));
                trayIcon->setIcon(QIcon(":/icons/toolbar_testnet"));
                toggleHideAction->setIcon(QIcon(":/icons/toolbar_testnet"));
            }

            aboutAction->setIcon(QIcon(":/icons/toolbar_testnet"));
        }

        // Keep up to date with client
        setNumConnections(clientModel->getNumConnections());
        connect(clientModel, SIGNAL(numConnectionsChanged(int)), this, SLOT(setNumConnections(int)));

        setNumBlocks(clientModel->getNumBlocks(), clientModel->getNumBlocksOfPeers());
        connect(clientModel, SIGNAL(numBlocksChanged(int,int)), this, SLOT(setNumBlocks(int,int)));

        // Report errors from network/worker thread
        connect(clientModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        rpcConsole->setClientModel(clientModel);
        addressBookPage->setOptionsModel(clientModel->getOptionsModel());
        receiveCoinsPage->setOptionsModel(clientModel->getOptionsModel());
    }
}

void IocoinGUI::setWalletModel(WalletModel *walletModel)
{
    this->walletModel = walletModel;
    if(walletModel)
    {
        // Report errors from wallet thread
        connect(walletModel, SIGNAL(error(QString,QString,bool)), this, SLOT(error(QString,QString,bool)));

        // Put transaction list in tabs
        transactionView->setModel(walletModel);

        overviewPage->setModel(walletModel);
        addressBookPage->setModel(walletModel->getAddressTableModel());
        receiveCoinsPage->setModel(walletModel->getAddressTableModel());
        sendCoinsPage->setModel(walletModel);
        dionsPage->setModel(walletModel);
        signVerifyMessageDialog->setModel(walletModel);

        setEncryptionStatus(walletModel->getEncryptionStatus());
        connect(walletModel, SIGNAL(encryptionStatusChanged(int)), this, SLOT(setEncryptionStatus(int)));

        // Balloon pop-up for new transaction
        connect(walletModel->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(incomingTransaction(QModelIndex,int,int)));

        // Ask for passphrase if needed
        connect(walletModel, SIGNAL(requireUnlock()), this, SLOT(unlockWallet()));
    }
}

void IocoinGUI::createTrayIcon()
{
    QMenu *trayIconMenu;
#ifndef Q_OS_MAC
    trayIcon = new QSystemTrayIcon(this);
    trayIconMenu = new QMenu(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(tr("I/OCoin client"));
    trayIcon->setIcon(QIcon(":/icons/toolbar"));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
#else
    // Note: On Mac, the dock icon is used to provide the tray's functionality.
    MacDockIconHandler *dockIconHandler = MacDockIconHandler::instance();
    dockIconHandler->setMainWindow((QMainWindow *)this);
    trayIconMenu = dockIconHandler->dockMenu();
#endif

    // Configuration of the tray icon (or dock icon) icon menu
    trayIconMenu->addAction(toggleHideAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(sendCoinsAction);
    trayIconMenu->addAction(receiveCoinsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(signMessageAction);
    trayIconMenu->addAction(verifyMessageAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(optionsAction);
    trayIconMenu->addAction(openRPCConsoleAction);
#ifndef Q_OS_MAC // This is built-in on Mac
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
#endif

    notificator = new Notificator(qApp->applicationName(), trayIcon);
}

#ifndef Q_OS_MAC
void IocoinGUI::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        // Click on system tray icon triggers show/hide of the main window
        toggleHideAction->trigger();
    }
}
#endif

void IocoinGUI::optionsClicked()
{
    if(!clientModel || !clientModel->getOptionsModel())
        return;
    OptionsDialog dlg;
    dlg.setModel(clientModel->getOptionsModel());
    dlg.exec();
}

void IocoinGUI::aboutClicked()
{
    AboutDialog dlg;
    dlg.setModel(clientModel);
    dlg.exec();
}

void IocoinGUI::setNumConnections(int count)
{
    QString icon;
    switch(count)
    {
    case 0: icon = ":/icons/connect_0"; break;
    case 1: case 2: case 3: icon = ":/icons/connect_1"; break;
    case 4: case 5: case 6: icon = ":/icons/connect_2"; break;
    case 7: case 8: case 9: icon = ":/icons/connect_3"; break;
    default: icon = ":/icons/connect_4"; break;
    }
    labelConnectionsIcon->setPixmap(QIcon(icon).pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
    labelConnectionsIcon->setToolTip(tr("%n active connection(s) to I/OCoin network", "", count));
}

void IocoinGUI::setNumBlocks(int count, int nTotalBlocks)
{
    // don't show / hide progress bar and its label if we have no connection to the network
    if (!clientModel || clientModel->getNumConnections() == 0)
    {
        progressBarLabel->setVisible(false);
        progressBar->setVisible(false);

        return;
    }

    QString strStatusBarWarnings = clientModel->getStatusBarWarnings();
    QString tooltip;

    if(count < nTotalBlocks)
    {
        int nRemainingBlocks = nTotalBlocks - count;
        float nPercentageDone = count / (nTotalBlocks * 0.01f);

        if (strStatusBarWarnings.isEmpty())
        {
            progressBarLabel->setText(tr("Synchronizing with network..."));
            progressBarLabel->setVisible(true);
            progressBar->setFormat(tr("~%n block(s) remaining", "", nRemainingBlocks));
            progressBar->setMaximum(nTotalBlocks);
            progressBar->setValue(count);
            progressBar->setVisible(true);
        }

        tooltip = tr("Downloaded %1 of %2 blocks of transaction history (%3% done).").arg(count).arg(nTotalBlocks).arg(nPercentageDone, 0, 'f', 2);
    }
    else
    {
        if (strStatusBarWarnings.isEmpty())
            progressBarLabel->setVisible(false);

        progressBar->setVisible(false);
        tooltip = tr("Downloaded %1 blocks of transaction history.").arg(count);
    }

    // Override progressBarLabel text and hide progress bar, when we have warnings to display
    if (!strStatusBarWarnings.isEmpty())
    {
        progressBarLabel->setText(strStatusBarWarnings);
        progressBarLabel->setVisible(true);
        progressBar->setVisible(false);
    }

    QDateTime lastBlockDate = clientModel->getLastBlockDate();
    int secs = lastBlockDate.secsTo(QDateTime::currentDateTime());
    QString text;

    // Represent time from last generated block in human readable text
    if(secs <= 0)
    {
        // Fully up to date. Leave text empty.
    }
    else if(secs < 60)
    {
        text = tr("%n second(s) ago","",secs);
    }
    else if(secs < 60*60)
    {
        text = tr("%n minute(s) ago","",secs/60);
    }
    else if(secs < 24*60*60)
    {
        text = tr("%n hour(s) ago","",secs/(60*60));
    }
    else
    {
        text = tr("%n day(s) ago","",secs/(60*60*24));
    }

    // Set icon state: spinning if catching up, tick otherwise
    if(secs < 90*60 && count >= nTotalBlocks)
    {
        tooltip = tr("Up to date") + QString(".<br>") + tooltip;
        labelBlocksIcon->setPixmap(QIcon(":/icons/synced").pixmap(STATUSBAR_ICONSIZE, STATUSBAR_ICONSIZE));

        overviewPage->showOutOfSyncWarning(false);
    }
    else
    {
        tooltip = tr("Catching up...") + QString("<br>") + tooltip;
        labelBlocksIcon->setMovie(syncIconMovie);
        syncIconMovie->start();

        overviewPage->showOutOfSyncWarning(true);
    }

    if(!text.isEmpty())
    {
        tooltip += QString("<br>");
        tooltip += tr("Last received block was generated %1.").arg(text);
    }

    // Don't word-wrap this (fixed-width) tooltip
    tooltip = QString("<nobr>") + tooltip + QString("</nobr>");

    labelBlocksIcon->setToolTip(tooltip);
    progressBarLabel->setToolTip(tooltip);
    progressBar->setToolTip(tooltip);
}

void IocoinGUI::error(const QString &title, const QString &message, bool modal)
{
    // Report errors from network/worker thread
    if(modal)
    {
        QMessageBox::critical(this, title, message, QMessageBox::Ok, QMessageBox::Ok);
    } else {
        notificator->notify(Notificator::Critical, title, message);
    }
}



void IocoinGUI::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
#ifndef Q_OS_MAC // Ignored on Mac
    if(e->type() == QEvent::WindowStateChange)
    {
        if(clientModel && clientModel->getOptionsModel()->getMinimizeToTray())
        {
            QWindowStateChangeEvent *wsevt = static_cast<QWindowStateChangeEvent*>(e);
            if(!(wsevt->oldState() & Qt::WindowMinimized) && isMinimized())
            {
                QTimer::singleShot(0, this, SLOT(hide()));
                e->ignore();
            }
        }
    }
#endif
}

void IocoinGUI::closeEvent(QCloseEvent *event)
{
    if(clientModel)
    {
#ifndef Q_OS_MAC // Ignored on Mac
        if(!clientModel->getOptionsModel()->getMinimizeToTray() &&
           !clientModel->getOptionsModel()->getMinimizeOnClose())
        {
            qApp->quit();
        }
#endif
    }
    QMainWindow::closeEvent(event);
}

void IocoinGUI::askFee(qint64 nFeeRequired, bool *payFee)
{
    QString strMessage =
        tr("This transaction is over the size limit.  You can still send it for a fee of %1, "
          "which goes to the nodes that process your transaction and helps to support the network.  "
          "Do you want to pay the fee?").arg(
                IocoinUnits::formatWithUnit(IocoinUnits::BTC, nFeeRequired));
    QMessageBox::StandardButton retval = QMessageBox::question(
          this, tr("Confirm transaction fee"), strMessage,
          QMessageBox::Yes|QMessageBox::Cancel, QMessageBox::Yes);
    *payFee = (retval == QMessageBox::Yes);
}

void IocoinGUI::incomingTransaction(const QModelIndex & parent, int start, int end)
{
    if(!walletModel || !clientModel)
        return;
    TransactionTableModel *ttm = walletModel->getTransactionTableModel();
    qint64 amount = ttm->index(start, TransactionTableModel::Amount, parent)
                    .data(Qt::EditRole).toULongLong();
    if(!clientModel->inInitialBlockDownload())
    {
        // On new transaction, make an info balloon
        // Unless the initial block download is in progress, to prevent balloon-spam
        QString date = ttm->index(start, TransactionTableModel::Date, parent)
                        .data().toString();
        QString type = ttm->index(start, TransactionTableModel::Type, parent)
                        .data().toString();
        QString address = ttm->index(start, TransactionTableModel::ToAddress, parent)
                        .data().toString();
        QIcon icon = qvariant_cast<QIcon>(ttm->index(start,
                            TransactionTableModel::ToAddress, parent)
                        .data(Qt::DecorationRole));

        notificator->notify(Notificator::Information,
                            (amount)<0 ? tr("Sent transaction") :
                                         tr("Incoming transaction"),
                              tr("Date: %1\n"
                                 "Amount: %2\n"
                                 "Type: %3\n"
                                 "Address: %4\n")
                              .arg(date)
                              .arg(IocoinUnits::formatWithUnit(walletModel->getOptionsModel()->getDisplayUnit(), amount, true))
                              .arg(type)
                              .arg(address), icon);
    }
}

void IocoinGUI::gotoOverviewPage()
{
    overviewAction->setChecked(true);
    centralWidget->setCurrentWidget(overviewPage);

    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"  <g>"
"    <polygon points=\"2,9.6 2,24 9,24 9,17 15,17 15,24 22,24 22,9.6 12,0.7 \"></polygon>"
"  </g>"
"</svg>"; 
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        overviewbutton->setIcon(icon);


    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void IocoinGUI::gotoDIONSPage()
{
    dionsAction->setChecked(true);
    centralWidget->setCurrentWidget(dionsPage);

    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"   <g> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"7.5,0 10,0 9.5,24 7,24\" stroke-linejoin=\"miter\"></polyline>  "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" rotate=\"5\" points=\"10.5,0 13,0 12.5,24 10,24\" stroke-linejoin=\"miter\"></polyline>  "
"   </g> "
"   <text x=\"0\" y=\"21\" font-size=\"27\">D</text> "
"</svg>"; 
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        dionsbutton->setIcon(icon);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void IocoinGUI::gotoHistoryPage()
{
    historyAction->setChecked(true);
    centralWidget->setCurrentWidget(transactionsPage);
    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"  <g>"
"                <path data-color=\"color-2\" d=\"M21,16H8v-4l-8,6l8,6v-4h13c0.6,0,1-0.4,1-1v-2C22,16.4,21.6,16,21,16z\"></path> "
"                <path d=\"M16,12l8-6l-8-6v4H3C2.4,4,2,4.4,2,5v2c0,0.6,0.4,1,1,1h13V12z\"></path> "
"  </g>"
"</svg>";
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        historybutton->setIcon(icon);
    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), transactionView, SLOT(exportClicked()));
}

void IocoinGUI::gotoAddressBookPage()
{
    addressBookAction->setChecked(true);
    centralWidget->setCurrentWidget(addressBookPage);
    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"  <g>"
"                <path d=\"M15.6,8L12,0.2L8.4,8H0.2l6,6.2l-2.3,9.3l8.1-4.6l8.1,4.6l-2.3-9.3l6-6.2H15.6z\"></path>"
"  </g>"
"</svg>"; 
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        addressbookbutton->setIcon(icon);
    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), addressBookPage, SLOT(exportClicked()));
}

void IocoinGUI::gotoReceiveCoinsPage()
{
    receiveCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(receiveCoinsPage);
    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"            <g transform=\"translate(0, 0)\"> "
"                <line data-cap=\"butt\" stroke-miterlimit=\"10\" x1=\"12\" y1=\"3\" x2=\"12\" y2=\"17\" stroke-linejoin=\"miter\" stroke-linecap=\"butt\"></line> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"5,10 19,10 19,8 5,8 \" stroke-linejoin=\"miter\"></polyline> "
"                <polyline stroke-linecap=\"square\" stroke-miterlimit=\"10\" points=\"7,12 12,17 17,12 \" stroke-linejoin=\"miter\"></polyline> "
"                <line data-color=\"color-2\" stroke-linecap=\"square\" stroke-miterlimit=\"10\" x1=\"19\" y1=\"21\" x2=\"5\" y2=\"21\" stroke-linejoin=\"miter\"></line> "
"            </g> "
"</svg>";
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        receivebutton->setIcon(icon);
    exportAction->setEnabled(true);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
    connect(exportAction, SIGNAL(triggered()), receiveCoinsPage, SLOT(exportClicked()));
}

void IocoinGUI::gotoSendCoinsPage()
{
    sendCoinsAction->setChecked(true);
    centralWidget->setCurrentWidget(sendCoinsPage);
    string iconSvgDataChecked = 
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
" <svg   xmlns=\"http://www.w3.org/2000/svg\""
"   xmlns:xlink=\"http://www.w3.org/1999/xlink\""
"   width=\"16px\""
"   height=\"16px\""
"   x=\"0px\" y=\"0px\" viewBox=\"0 0 24 24\" style=\"fill:#1aa8ea \">"
"  <g>"
"                <path d=\"M1.4,11l2.2,1.6l6.4-3c0.5-0.2,1,0.4,0.5,0.8l-4.5,4v6.7c0,0.9,1.2,1.4,1.8,0.6l3.1-3.7l6.5,4.9 c0.6,0.4,1.4,0.1,1.6-0.6l4-20c0.2-0.8-0.6-1.4-1.4-1.1l-20,8C0.9,9.6,0.8,10.6,1.4,11z\"></path>"
"  </g>"
"</svg>"; 
    QIcon icon = QIcon(new SVGIconEngine(iconSvgDataChecked));
        sendbutton->setIcon(icon);
    exportAction->setEnabled(false);
    disconnect(exportAction, SIGNAL(triggered()), 0, 0);
}

void IocoinGUI::gotoSignMessageTab(QString addr)
{
    // call show() in showTab_SM()
    signVerifyMessageDialog->showTab_SM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_SM(addr);
}

void IocoinGUI::gotoVerifyMessageTab(QString addr)
{
    // call show() in showTab_VM()
    signVerifyMessageDialog->showTab_VM(true);

    if(!addr.isEmpty())
        signVerifyMessageDialog->setAddress_VM(addr);
}

void IocoinGUI::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only URIs
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void IocoinGUI::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        int nValidUrisFound = 0;
        QList<QUrl> uris = event->mimeData()->urls();
        foreach(const QUrl &uri, uris)
        {
            if (sendCoinsPage->handleURI(uri.toString()))
                nValidUrisFound++;
        }

        // if valid URIs were found
        if (nValidUrisFound)
            gotoSendCoinsPage();
        else
            notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid I/OCoin address or malformed URI parameters."));
    }

    event->acceptProposedAction();
}

void IocoinGUI::handleURI(QString strURI)
{
    // URI has to be valid
    if (sendCoinsPage->handleURI(strURI))
    {
        showNormalIfMinimized();
        gotoSendCoinsPage();
    }
    else
        notificator->notify(Notificator::Warning, tr("URI handling"), tr("URI can not be parsed! This can be caused by an invalid I/OCoin address or malformed URI parameters."));
}

void IocoinGUI::setEncryptionStatus(int status)
{
    switch(status)
    {
    case WalletModel::Unencrypted:
        labelEncryptionIcon->hide();
        encryptWalletAction->setChecked(false);
        changePassphraseAction->setEnabled(false);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(true);
        break;
    case WalletModel::Unlocked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_open").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>unlocked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(false);
        lockWalletAction->setVisible(true);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    case WalletModel::Locked:
        labelEncryptionIcon->show();
        labelEncryptionIcon->setPixmap(QIcon(":/icons/lock_closed").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelEncryptionIcon->setToolTip(tr("Wallet is <b>encrypted</b> and currently <b>locked</b>"));
        encryptWalletAction->setChecked(true);
        changePassphraseAction->setEnabled(true);
        unlockWalletAction->setVisible(true);
        lockWalletAction->setVisible(false);
        encryptWalletAction->setEnabled(false); // TODO: decrypt currently not supported
        break;
    }
}

void IocoinGUI::encryptWallet(bool status)
{
    if(!walletModel)
        return;
    AskPassphraseDialog dlg(status ? AskPassphraseDialog::Encrypt:
                                     AskPassphraseDialog::Decrypt, this);
    dlg.setModel(walletModel);
    dlg.exec();

    setEncryptionStatus(walletModel->getEncryptionStatus());
}

void IocoinGUI::backupWallet()
{
    QString saveDir = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
    QString filename = QFileDialog::getSaveFileName(this, tr("Backup Wallet"), saveDir, tr("Wallet Data (*.dat)"));
    if(!filename.isEmpty()) {
        if(!walletModel->backupWallet(filename)) {
            QMessageBox::warning(this, tr("Backup Failed"), tr("There was an error trying to save the wallet data to the new location."));
        }
    }
}

void IocoinGUI::changePassphrase()
{
    AskPassphraseDialog dlg(AskPassphraseDialog::ChangePass, this);
    dlg.setModel(walletModel);
    dlg.exec();
}

void IocoinGUI::unlockWallet()
{
    if(!walletModel)
        return;
    // Unlock wallet when requested by wallet model
    if(walletModel->getEncryptionStatus() == WalletModel::Locked)
    {
        AskPassphraseDialog::Mode mode = sender() == unlockWalletAction ?
              AskPassphraseDialog::UnlockStaking : AskPassphraseDialog::Unlock;
        AskPassphraseDialog dlg(mode, this);
        dlg.setModel(walletModel);
        dlg.exec();
    }
}

void IocoinGUI::lockWallet()
{
    if(!walletModel)
        return;

    walletModel->setWalletLocked(true);
}

void IocoinGUI::showNormalIfMinimized(bool fToggleHidden)
{
    // activateWindow() (sometimes) helps with keyboard focus on Windows
    if (isHidden())
    {
        show();
        activateWindow();
    }
    else if (isMinimized())
    {
        showNormal();
        activateWindow();
    }
    else if (GUIUtil::isObscured(this))
    {
        raise();
        activateWindow();
    }
    else if(fToggleHidden)
        hide();
}

void IocoinGUI::toggleHidden()
{
    showNormalIfMinimized(true);
}

void IocoinGUI::updateWeight()
{
    if (!pwalletMain)
        return;

    TRY_LOCK(cs_main, lockMain);
    if (!lockMain)
        return;

    TRY_LOCK(pwalletMain->cs_wallet, lockWallet);
    if (!lockWallet)
        return;

    pwalletMain->GetStakeWeight(nWeight);
}

void IocoinGUI::updateStakingIcon()
{
    updateWeight();

    if (nLastCoinStakeSearchInterval && nWeight)
    {
        uint64_t nNetworkWeight = GetPoSKernelPS();
        unsigned nEstimateTime = GetTargetSpacing(nBestHeight) * nNetworkWeight / nWeight;

        QString text;
        if (nEstimateTime < 60)
        {
            text = tr("%n second(s)", "", nEstimateTime);
        }
        else if (nEstimateTime < 60*60)
        {
            text = tr("%n minute(s)", "", nEstimateTime/60);
        }
        else if (nEstimateTime < 24*60*60)
        {
            text = tr("%n hour(s)", "", nEstimateTime/(60*60));
        }
        else
        {
            text = tr("%n day(s)", "", nEstimateTime/(60*60*24));
        }

        if (IsProtocolV2(nBestHeight+1))
        {
            nWeight /= COIN;
            nNetworkWeight /= COIN;
        }

        labelStakingIcon->setPixmap(QIcon(":/icons/staking_on").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        labelStakingIcon->setToolTip(tr("Staking.<br>Your weight is %1<br>Network weight is %2<br>Expected time to earn reward is %3").arg(nWeight).arg(nNetworkWeight).arg(text));
    }
    else
    {
        labelStakingIcon->setPixmap(QIcon(":/icons/staking_off").pixmap(STATUSBAR_ICONSIZE,STATUSBAR_ICONSIZE));
        if (pwalletMain && pwalletMain->as())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is locked"));
        else if (vNodes.empty())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is offline"));
        else if (IsInitialBlockDownload())
            labelStakingIcon->setToolTip(tr("Not staking because wallet is syncing"));
        else if (!nWeight)
            labelStakingIcon->setToolTip(tr("Not staking because you don't have mature coins"));
        else
            labelStakingIcon->setToolTip(tr("Not staking"));
    }
}

