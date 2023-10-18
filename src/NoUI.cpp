
#include "UIInterface.h"
#include "rpc/client.h"
#include <string>
static int noui_ThreadSafeMessageBox(const std::string& message, const std::string& caption, int style)
{
  printf("%s: %s\n", caption.c_str(), message.c_str());
  fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
  return 4;
}
static bool noui_ThreadSafeAskFee(int64_t nFeeRequired, const std::string& strCaption)
{
  return true;
}
void noui_connect()
{
  uiInterface.ThreadSafeMessageBox.connect(noui_ThreadSafeMessageBox);
  uiInterface.ThreadSafeAskFee.connect(noui_ThreadSafeAskFee);
}
