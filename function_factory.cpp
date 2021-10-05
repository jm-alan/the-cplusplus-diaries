#include "console.h"
#include <functional>

std::function<void()> functionFactory()
{
  static unsigned long long closed{};
  struct MutNum
  {
    void operator()() const
    {
      closed++;
      console::log(closed);
    }
  };
  static std::function<void()> mutator = MutNum();

  return mutator;
}

int main()
{
  const auto factOut = functionFactory();
  const auto differentFactOut = functionFactory();
  factOut();
  factOut();
  differentFactOut();
  differentFactOut();
}
