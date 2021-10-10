#include <iostream>
#include <iomanip>

namespace console
{
  void inl(const auto msg)
  {
    std::clog << msg;
  }

  void inl(const auto *msg)
  {
    std::clog << msg;
  }

  void log()
  {
    std::clog << std::endl;
  }

  void log(const auto msg)
  {
    std::clog << msg << std::endl;
  }

  void log(const auto *msg)
  {
    std::clog << msg << std::endl;
  }

  void setprecision(const unsigned long long p)
  {
    std::cout << std::setprecision(p);
  }

  void pause()
  {
    std::cout << std::flush;
    std::clog << "Press any key to continue..." << std::endl;
    std::cout << std::flush;
    std::cin.get();
  }
}
