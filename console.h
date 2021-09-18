#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

namespace console
{
  std::string exec(const char *cmd)
  {
    // Taken from this absolutley wonderful SO post https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
    std::array<char, 128> buffer{};
    std::string result{};
    std::unique_ptr<FILE, decltype(&pclose)> pipe{popen(cmd, "r"), pclose};
    if (!pipe)
      throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      result += buffer.data();
    return result;
  }

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

  void pause()
  {
    std::cout << std::flush;
    std::clog << "Press any key to continue..." << std::endl;
    std::cout << std::flush;
    std::cin.get();
  }
}
