#include "console.h"
#include <bitset>

int main()
{
  unsigned short int btest{0xff0u};
  console::log(std::bitset<16>{btest << 1});
}
