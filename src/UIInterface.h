#ifndef BITCOIN_UI_INTERFACE_H
#define BITCOIN_UI_INTERFACE_H

#include <boost/signals2/last_value.hpp>
#include <boost/signals2/signal.hpp>

#include <string>

#include <stdint.h>

class CBasicKeyStore;
class __wx__;
class uint256;


enum ChangeType
{
  CT_NEW,
  CT_UPDATED,
  CT_DELETED
};


class CClientUIInterface
{
public:

  enum MessageBoxFlags
  {
    YES = 0x00000002,
    OK = 0x00000004,
    NO = 0x00000008,
    YES_NO = (YES|NO),
    CANCEL = 0x00000010,
    APPLY = 0x00000020,
    CLOSE = 0x00000040,
    OK_DEFAULT = 0x00000000,
    YES_DEFAULT = 0x00000000,
    NO_DEFAULT = 0x00000080,
    CANCEL_DEFAULT = 0x80000000,
    ICON_EXCLAMATION = 0x00000100,
    ICON_HAND = 0x00000200,
    ICON_WARNING = ICON_EXCLAMATION,
    ICON_ERROR = ICON_HAND,
    ICON_QUESTION = 0x00000400,
    ICON_INFORMATION = 0x00000800,
    ICON_STOP = ICON_HAND,
    ICON_ASTERISK = ICON_INFORMATION,
    ICON_MASK = (0x00000100|0x00000200|0x00000400|0x00000800),
    FORWARD = 0x00001000,
    BACKWARD = 0x00002000,
    RESET = 0x00004000,
    HELP = 0x00008000,
    MORE = 0x00010000,
    SETUP = 0x00020000,

    MODAL = 0x00040000
  };

  std::string translate(const char* psz)
  {
    boost::optional<std::string> rv = this->Translate(psz);
    return rv ? (*rv) : psz;
  }

  bool initError_(const std::string &str)
  {
    this->ThreadSafeMessageBox(str, translate("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::MODAL);
    return false;
  }
  bool initWarning_(const std::string &str)
  {
    this->ThreadSafeMessageBox(str, translate("I/OCoin"), CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
    return true;
  }

  boost::signals2::signal<void (const std::string& message, const std::string& caption, int style)> ThreadSafeMessageBox;


  boost::signals2::signal<bool (int64_t nFeeRequired, const std::string& strCaption), boost::signals2::last_value<bool> > ThreadSafeAskFee;


  boost::signals2::signal<void (const std::string& strURI)> ThreadSafeHandleURI;


  boost::signals2::signal<void (const std::string &message)> InitMessage;


  boost::signals2::signal<void ()> QueueShutdown;


  boost::signals2::signal<std::string (const char* psz)> Translate;


  boost::signals2::signal<void ()> NotifyBlocksChanged;


  boost::signals2::signal<void (int newNumConnections)> NotifyNumConnectionsChanged;





  boost::signals2::signal<void (const uint256 &hash, ChangeType status)> NotifyAlertChanged;
};

extern CClientUIInterface uiInterface;

inline std::string _(const char* psz)
{
  boost::optional<std::string> rv = uiInterface.Translate(psz);
  return rv ? (*rv) : psz;
}

#endif
