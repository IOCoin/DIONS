#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "util.h"

BOOST_AUTO_TEST_SUITE(getarg_tests)

static void
ResetArgs(const std::string& strArg)
{
    std::vector<std::string> vecArg;
    boost::split(vecArg, strArg, boost::is_space(), boost::token_compress_on);

    // Insert dummy executable name:
    vecArg.insert(vecArg.begin(), "testbitcoin");

    // Convert to char*:
    std::vector<const char*> vecChar;
    BOOST_FOREACH(std::string& s, vecArg)
        vecChar.push_back(s.c_str());

    ParseParameters(vecChar.size(), &vecChar[0]);
}

BOOST_AUTO_TEST_CASE(boolarg)
{
    ResetArgs("-bar");
    BOOST_CHECK(GetBoolArg("-bar"));
    BOOST_CHECK(GetBoolArg("-bar", false));
    BOOST_CHECK(GetBoolArg("-bar", true));

    BOOST_CHECK(!GetBoolArg("-fo"));
    BOOST_CHECK(!GetBoolArg("-fo", false));
    BOOST_CHECK(GetBoolArg("-fo", true));

    BOOST_CHECK(!GetBoolArg("-baro"));
    BOOST_CHECK(!GetBoolArg("-baro", false));
    BOOST_CHECK(GetBoolArg("-baro", true));

    ResetArgs("-bar=0");
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", false));
    BOOST_CHECK(!GetBoolArg("-bar", true));

    ResetArgs("-bar=1");
    BOOST_CHECK(GetBoolArg("-bar"));
    BOOST_CHECK(GetBoolArg("-bar", false));
    BOOST_CHECK(GetBoolArg("-bar", true));

    // New 0.6 feature: auto-map -nosomething to !-something:
    ResetArgs("-nobar");
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", false));
    BOOST_CHECK(!GetBoolArg("-bar", true));

    ResetArgs("-nobar=1");
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", false));
    BOOST_CHECK(!GetBoolArg("-bar", true));

    ResetArgs("-bar -nobar");  // -bar should win
    BOOST_CHECK(GetBoolArg("-bar"));
    BOOST_CHECK(GetBoolArg("-bar", false));
    BOOST_CHECK(GetBoolArg("-bar", true));

    ResetArgs("-bar=1 -nobar=1");  // -bar should win
    BOOST_CHECK(GetBoolArg("-bar"));
    BOOST_CHECK(GetBoolArg("-bar", false));
    BOOST_CHECK(GetBoolArg("-bar", true));

    ResetArgs("-bar=0 -nobar=0");  // -bar should win
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", false));
    BOOST_CHECK(!GetBoolArg("-bar", true));

    // New 0.6 feature: treat -- same as -:
    ResetArgs("--bar=1");
    BOOST_CHECK(GetBoolArg("-bar"));
    BOOST_CHECK(GetBoolArg("-bar", false));
    BOOST_CHECK(GetBoolArg("-bar", true));

    ResetArgs("--nobar=1");
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", false));
    BOOST_CHECK(!GetBoolArg("-bar", true));

}

BOOST_AUTO_TEST_CASE(stringarg)
{
    ResetArgs("");
    BOOST_CHECK_EQUAL(GetArg("-bar", ""), "");
    BOOST_CHECK_EQUAL(GetArg("-bar", "eleven"), "eleven");

    ResetArgs("-bar -bar");
    BOOST_CHECK_EQUAL(GetArg("-bar", ""), "");
    BOOST_CHECK_EQUAL(GetArg("-bar", "eleven"), "");

    ResetArgs("-bar=");
    BOOST_CHECK_EQUAL(GetArg("-bar", ""), "");
    BOOST_CHECK_EQUAL(GetArg("-bar", "eleven"), "");

    ResetArgs("-bar=11");
    BOOST_CHECK_EQUAL(GetArg("-bar", ""), "11");
    BOOST_CHECK_EQUAL(GetArg("-bar", "eleven"), "11");

    ResetArgs("-bar=eleven");
    BOOST_CHECK_EQUAL(GetArg("-bar", ""), "eleven");
    BOOST_CHECK_EQUAL(GetArg("-bar", "eleven"), "eleven");

}

BOOST_AUTO_TEST_CASE(intarg)
{
    ResetArgs("");
    BOOST_CHECK_EQUAL(GetArg("-bar", 11), 11);
    BOOST_CHECK_EQUAL(GetArg("-bar", 0), 0);

    ResetArgs("-bar -bar");
    BOOST_CHECK_EQUAL(GetArg("-bar", 11), 0);
    BOOST_CHECK_EQUAL(GetArg("-bar", 11), 0);

    ResetArgs("-bar=11 -bar=12");
    BOOST_CHECK_EQUAL(GetArg("-bar", 0), 11);
    BOOST_CHECK_EQUAL(GetArg("-bar", 11), 12);

    ResetArgs("-bar=NaN -bar=NotANumber");
    BOOST_CHECK_EQUAL(GetArg("-bar", 1), 0);
    BOOST_CHECK_EQUAL(GetArg("-bar", 11), 0);
}

BOOST_AUTO_TEST_CASE(doubledash)
{
    ResetArgs("--bar");
    BOOST_CHECK_EQUAL(GetBoolArg("-bar"), true);

    ResetArgs("--bar=verbose --bar=1");
    BOOST_CHECK_EQUAL(GetArg("-bar", ""), "verbose");
    BOOST_CHECK_EQUAL(GetArg("-bar", 0), 1);
}

BOOST_AUTO_TEST_CASE(boolargno)
{
    ResetArgs("-nobar");
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", true));
    BOOST_CHECK(!GetBoolArg("-bar", false));

    ResetArgs("-nobar=1");
    BOOST_CHECK(!GetBoolArg("-bar"));
    BOOST_CHECK(!GetBoolArg("-bar", true));
    BOOST_CHECK(!GetBoolArg("-bar", false));

    ResetArgs("-nobar=0");
    BOOST_CHECK(GetBoolArg("-bar"));
    BOOST_CHECK(GetBoolArg("-bar", true));
    BOOST_CHECK(GetBoolArg("-bar", false));

    ResetArgs("-bar --nobar");
    BOOST_CHECK(GetBoolArg("-bar"));

    ResetArgs("-nobar -bar"); // bar always wins:
    BOOST_CHECK(GetBoolArg("-bar"));
}

BOOST_AUTO_TEST_SUITE_END()
