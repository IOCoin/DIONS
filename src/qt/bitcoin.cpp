#include "iocoingui.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"

#include "init.h"
#include "ui_interface.h"
#include "qtipcserver.h"

#include <QApplication>
#include <QMessageBox>
#include <QTextCodec>
#include <QLocale>
#include <QThread>
#include <QTranslator>
#include <QSplashScreen>
#include <QLibraryInfo>
#include <QDesktopWidget>
#include <QVBoxLayout>

#if defined(BITCOIN_NEED_QT_PLUGINS) && !defined(_BITCOIN_QT_PLUGINS_INCLUDED)
#define _BITCOIN_QT_PLUGINS_INCLUDED
#define __INSURE__
#include <QtPlugin>
Q_IMPORT_PLUGIN(qcncodecs)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qtwcodecs)
Q_IMPORT_PLUGIN(qkrcodecs)
Q_IMPORT_PLUGIN(qtaccessiblewidgets)
#endif

const string BOOTSTRAP_LOCATION = "/home/argon/bootstrap.dat";
// Need a global reference for the notifications to find the GUI
static IocoinGUI *guiref;
static QSplashScreen *splashref;
//static SplashScreen *splashref;

static void ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style)
{
    // Message from network thread
    if(guiref)
    {
        bool modal = (style & CClientUIInterface::MODAL);
        // in case of modal message, use blocking connection to wait for user to click OK
        QMetaObject::invokeMethod(guiref, "error",
                                   modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(message)),
                                   Q_ARG(bool, modal));
    }
    else
    {
        printf("%s: %s\n", caption.c_str(), message.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
    }
}

static bool ThreadSafeAskFee(int64_t nFeeRequired, const std::string& strCaption)
{
    if(!guiref)
        return false;
    if(nFeeRequired < MIN_TX_FEE || nFeeRequired <= nTransactionFee || fDaemon)
        return true;
    bool payFee = false;

    QMetaObject::invokeMethod(guiref, "askFee", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(qint64, nFeeRequired),
                               Q_ARG(bool*, &payFee));

    return payFee;
}

static void ThreadSafeHandleURI(const std::string& strURI)
{
    if(!guiref)
        return;

    QMetaObject::invokeMethod(guiref, "handleURI", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(QString, QString::fromStdString(strURI)));
}

static void InitMessage(const std::string &message)
{
    if(splashref)
    {
        splashref->showMessage(QString::fromStdString(message), Qt::AlignVCenter|Qt::AlignHCenter, QColor(33,120,185));
        QApplication::instance()->processEvents();
    }
}

static void QueueShutdown()
{
    QMetaObject::invokeMethod(QCoreApplication::instance(), "quit", Qt::QueuedConnection);
}

/*
   Translate string to current locale using Qt.
 */
static std::string Translate(const char* psz)
{
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
}

/* Handle runaway exceptions. Shows a message box with the problem and quits the program.
 */
static void handleRunawayException(std::exception *e)
{
    PrintExceptionContinue(e, "Runaway exception");
    QMessageBox::critical(0, "Runaway exception", IocoinGUI::tr("A fatal error occurred. I/OCoin can no longer continue safely and will quit.") + QString("\n\n") + QString::fromStdString(strMiscWarning));
    exit(1);
}
static int argc_;
static char** argv_;
extern "C" int setup_application(IocoinGUI& window, std::string directory)
{
  bool init = true;
  boost::filesystem::path p(directory);
  boost::filesystem::directory_iterator end_itr;
  boost::filesystem::directory_iterator it(p);
  for(; it != end_itr; it++)
  {
    if(boost::filesystem::is_regular_file(it->path()))
    {
      string f = it->path().filename().string();
      if(f == "iocoin.conf")
      {
        init=false;
      }
    }
  }

  if(init)
  {
    boost::filesystem::ofstream ofs(p / "iocoin.conf");   
    ofs << "rpcuser=kjkjkjkjkjk" << endl;
    ofs << "rpcpassword=kjkjkjkjkjkjhjhjhjhjhjhj" << endl;
    ofs << endl;
    ofs << endl;
    ofs << "addnode=amer.supernode.iocoin.io" << endl;
    ofs << "addnode=emea.supernode.iocoin.io" << endl;
    ofs << "addnode=apac.supernode.iocoin.io" << endl;
  }

  mapArgs["-datadir"] = directory.c_str();
  ReadConfigFile(mapArgs, mapMultiArgs);

  AppInit2();

  return 0;
}

extern "C" int app_init()
{
  AppInit2();
  return 0;
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(bitcoin);
    QApplication app(argc, argv);

    argc_=argc; argv_=argv;
    // Install global event filter that makes sure that long tooltips can be word-wrapped
    app.installEventFilter(new GUIUtil::ToolTipToRichTextFilter(TOOLTIP_WRAP_THRESHOLD, &app));

    app.setOrganizationName("I/OCoin");
    if(GetBoolArg("-testnet")) // Separate UI settings for testnet
        app.setApplicationName("I/OCoin-Qt-testnet");
    else
        app.setApplicationName("I/OCoin-Qt");

    // Get desired locale (e.g. "de_DE") from command line or use system locale
    QString lang_territory = QString::fromStdString(GetArg("-lang", QLocale::system().name().toStdString()));
    QString lang = lang_territory;
    // Convert to "de" only by truncating "_DE"
    lang.truncate(lang_territory.lastIndexOf('_'));

    QTranslator qtTranslatorBase, qtTranslator, translatorBase, translator;
    // Load language files for configured locale:
    // - First load the translator for the base language, without territory
    // - Then load the more specific locale translator

    // Load e.g. qt_de.qm
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslatorBase);

    // Load e.g. qt_de_DE.qm
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);

    // Load e.g. bitcoin_de.qm (shortcut "de" needs to be defined in bitcoin.qrc)
    if (translatorBase.load(lang, ":/translations/"))
        app.installTranslator(&translatorBase);

    // Load e.g. bitcoin_de_DE.qm (shortcut "de_DE" needs to be defined in bitcoin.qrc)
    if (translator.load(lang_territory, ":/translations/"))
        app.installTranslator(&translator);

    // Subscribe to global signals from core
    uiInterface.ThreadSafeMessageBox.connect(ThreadSafeMessageBox);
    uiInterface.ThreadSafeAskFee.connect(ThreadSafeAskFee);
    uiInterface.ThreadSafeHandleURI.connect(ThreadSafeHandleURI);
    uiInterface.InitMessage.connect(InitMessage);
    uiInterface.QueueShutdown.connect(QueueShutdown);
    uiInterface.Translate.connect(Translate);

    // Show help message immediately after parsing command-line options (for "-lang") and setting locale,
    // but before showing splash screen.
    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        GUIUtil::HelpMessageBox help;
        help.showOrPrint();
        return 1;
    }

    QSplashScreen splash(QPixmap(":/images/splash"), 0);
    if (GetBoolArg("-splash", true) && !GetBoolArg("-min"))
    {
        splash.show();
        splashref = &splash;
    }

    app.processEvents();

    app.setQuitOnLastWindowClosed(false);

    try
    {
      IocoinGUI window;
      guiref = &window;
      {
        {
          if (splashref)
            splash.finish(&window);

          // If -min option passed, start window minimized.
          if(GetBoolArg("-min"))
          {
            window.showMinimized();
          }
          else
          {
            window.show();
          }

          app.exec();
          window.hide();
          window.setClientModel(0);
          window.setWalletModel(0);
          guiref = 0;
        }
            Shutdown(NULL);
      }
    } catch (std::exception& e) {
        handleRunawayException(&e);
    } catch (...) {
        handleRunawayException(NULL);
    }
    return 0;
}
