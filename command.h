#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "./pstream.h"

namespace child_process
{
  std::string exec(const char *cmd)
  {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
      throw std::runtime_error("popen() failed!");
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
      result += buffer.data();
    return result;
  }

  std::string exec_safe(const char *cmd)
  {
    redi::ipstream proc(cmd, redi::pstreams::pstdout | redi::pstreams::pstderr);
    std::string line, acc;
    // read child's stdout
    while (std::getline(proc.out(), line))
      acc += line;
    // if reading stdout stopped at EOF then reset the state:
    if (proc.eof() && proc.fail())
      proc.clear();
    else
      return acc;
    // read child's stderr
    while (std::getline(proc.err(), line))
      acc += line;
    return acc;
  }
}

// Taken from this absolutley wonderful SO post https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
