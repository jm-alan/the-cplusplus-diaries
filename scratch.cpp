#include <string>
#include <console.h>
#include <random>

struct Dog
{
  int a;
  int b;
  int c;
  int d;
  int e;
  int f;
  int g;
};

int main()
{
  const Dog brutus = {74, 83, 32, 62, 32, 80, 89};
  const int *someChars = (const int *)&brutus;
  for (int i = 1000000000; i > 0; i--)
  {
    console::inl(someChars[i]);
  }
  console::log();
}
