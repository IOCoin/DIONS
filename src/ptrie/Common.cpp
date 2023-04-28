#include "ptrie/Common.h"



#if defined(_WIN32)
#include <windows.h>
#endif



using namespace std;

namespace dev
{

bytes const NullBytes;
std::string const EmptyString;

void InvariantChecker::checkInvariants(HasInvariants const* _this, char const* _fn, char const* _file, int _line, bool _pre)
{
  if (!_this->invariants())
  {
    std::cout << (_pre ? "Pre" : "Post") << "invariant failed in" << _fn << "at" << _file << ":" << _line;


  }
}

TimerHelper::~TimerHelper()
{
  auto e = std::chrono::high_resolution_clock::now() - m_t;
  if (!m_ms || e > chrono::milliseconds(m_ms))

  {
    std::cout << m_id << " " << chrono::duration_cast<chrono::milliseconds>(e).count() << " ms";
  }
}

int64_t utcTime()
{



  return time(0);
}

string inUnits(bigint const& _b, strings const& _units)
{
  ostringstream ret;
  u256 b;
  if (_b < 0)
  {
    ret << "-";
    b = (u256)-_b;
  }
  else
  {
    b = (u256)_b;
  }

  u256 biggest = 1;
  for (unsigned i = _units.size() - 1; !!i; --i)
  {
    biggest *= 1000;
  }

  if (b > biggest * 1000)
  {
    ret << (b / biggest) << " " << _units.back();
    return ret.str();
  }
  ret << setprecision(3);

  u256 unit = biggest;
  for (auto it = _units.rbegin(); it != _units.rend(); ++it)
  {
    auto i = *it;
    if (i != _units.front() && b >= unit)
    {
      ret << (double(b / (unit / 1000)) / 1000.0) << " " << i;
      return ret.str();
    }
    else
    {
      unit /= 1000;
    }
  }
  ret << b << " " << _units.front();
  return ret.str();
}
# 101 "Common.cpp"
void setDefaultOrCLocale()
{
#if __unix__
  if (!setlocale(LC_ALL, ""))
  {
    setenv("LC_ALL", "C", 1);
  }
#endif

#if defined(_WIN32)


  SetConsoleOutputCP(CP_UTF8);
#endif
}

bool ExitHandler::s_shouldExit = false;

bool isTrue(std::string const& _m)
{
  return _m == "on" || _m == "yes" || _m == "true" || _m == "1";
}

bool isFalse(std::string const& _m)
{
  return _m == "off" || _m == "no" || _m == "false" || _m == "0";
}
template <typename _T>
inline _T contentsGeneric(boost::filesystem::path const& _file)
{
  _T ret;
  size_t const c_elementSize = sizeof(typename _T::value_type);
  boost::filesystem::ifstream is(_file, std::ifstream::binary);
  if (!is)
  {
    return ret;
  }


  is.seekg(0, is.end);
  std::streamoff length = is.tellg();
  if (length == 0)
  {
    return ret;
  }
  is.seekg(0, is.beg);

  ret.resize((length + c_elementSize - 1) / c_elementSize);
  is.read(const_cast<char*>(reinterpret_cast<char const*>(ret.data())), length);
  return ret;
}

bytes contents(boost::filesystem::path const& _file)
{
  return contentsGeneric<bytes>(_file);
}

}
