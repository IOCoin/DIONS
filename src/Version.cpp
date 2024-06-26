
#include <string>
#include "Version.h"
const std::string CLIENT_NAME("Satoshi");
#define CLIENT_VERSION_SUFFIX ""
#ifdef HAVE_BUILD_INFO
# include "build.h"
#endif
#define BUILD_DATE __DATE__ ", " __TIME__
const std::string CLIENT_DATE(BUILD_DATE);
#ifdef GIT_ARCHIVE
#define GIT_COMMIT_ID         "$Id: dac21d94001b2763123a70cc2731642f8f276874 $"
#define GIT_COMMIT_HASH       "$Format:%H$"
#define GIT_COMMIT_ABREV_HASH "$Format:%h$"
#define GIT_COMMITTER_DATE    "$Format:%cD$"
#endif
#define BUILD_DESC_FROM_COMMIT(maj,min,rev,build,commit) \
    "v" DO_STRINGIZE(maj) "." DO_STRINGIZE(min) "." DO_STRINGIZE(rev) "." DO_STRINGIZE(build) "-g" commit
#define BUILD_DESC_FROM_UNKNOWN(maj,min,rev,build) \
    "v" DO_STRINGIZE(maj) "." DO_STRINGIZE(min) "." DO_STRINGIZE(rev) "." DO_STRINGIZE(build) "-unk"
#ifndef BUILD_DESC
#  ifdef GIT_COMMIT_ID
#    define BUILD_DESC BUILD_DESC_FROM_COMMIT(CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_REVISION, CLIENT_VERSION_BUILD, GIT_COMMIT_ID)
#  else
#    define BUILD_DESC BUILD_DESC_FROM_UNKNOWN(CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_REVISION, CLIENT_VERSION_BUILD)
#  endif
const std::string CLIENT_BUILD(BUILD_DESC BUILD_DATE CLIENT_VERSION_SUFFIX);
#else
#    define BUILD_DESC_CONF BUILD_DESC_FROM_COMMIT(CLIENT_VERSION_MAJOR, CLIENT_VERSION_MINOR, CLIENT_VERSION_REVISION, CLIENT_VERSION_BUILD, BUILD_DESC)
const std::string CLIENT_BUILD(BUILD_DESC_CONF CLIENT_VERSION_SUFFIX);
#endif
