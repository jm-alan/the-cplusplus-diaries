#include <vector>
#include <iostream>
#include <random>

std::random_device rd{};
std::default_random_engine generator{rd()};
std::uniform_int_distribution<unsigned long long> distribution{0, 0xFFFFFFFFFFFFFFFF};

void merge(
    std::vector<unsigned long long> *ptrV,
    unsigned long long leftStart,
    unsigned long long rightStart,
    const unsigned long long end)
{
  unsigned long long staging{};
  while (leftStart < rightStart && rightStart < end)
  {
    if (ptrV->operator[](leftStart) > ptrV->operator[](rightStart))
    {
      staging = ptrV->operator[](rightStart);
      for (unsigned long long i{rightStart}; i > leftStart; i--)
        ptrV->operator[](i) = ptrV->operator[](i - 1);
      ptrV->operator[](leftStart) = staging;
      rightStart++;
    }
    leftStart++;
  }
};

void sort(
    std::vector<unsigned long long> *ptrV,
    const unsigned long long start,
    const unsigned long long end)
{
  if ((end - start) < 2)
    return;
  else if ((end - start) == 2 && ptrV->operator[](start) > ptrV->operator[](start + 1))
    std::iter_swap(ptrV->begin() + start, ptrV->begin() + (start + 1));
  else
  {
    const unsigned long long diff{(start + end) >> 1};
    sort(ptrV, start, diff);
    sort(ptrV, diff, end);
    merge(ptrV, start, diff, end);
  }
};

bool is_sorted(const std::vector<unsigned long long> &refV)
{
  for (int i{}; i < (refV.size() - 1); i++)
    if (refV[i] > refV[i + 1])
      return false;
  return true;
};

int main()
{
  std::vector<unsigned long long> ints;
  ints.reserve(2000000000);
  for (int i{}; i < 2000000000; i++)
    ints.push_back(distribution(generator));
  const unsigned long long stepSize{ints.size() / 12};
  std::cout << "Sorting" << std::endl;
  sort(&ints, 0, ints.size());
  std::cout << "Validating" << std::endl;
  std::cout << std::boolalpha << is_sorted(ints) << std::endl;
}
