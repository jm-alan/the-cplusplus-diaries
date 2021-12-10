#include <future>
#include <vector>
#include <algorithm>

const std::vector<unsigned long long> partition(
    unsigned long long *ptrArr,
    const unsigned long long start,
    const unsigned long long end,
    const unsigned int numPivots,
    std::vector<std::mutex *> sectionLocks)
{
  std::vector<unsigned long long> partIdxs{};
  // cannot std::lock_guard bc lock_guard cannot be itself put into a vector
  // due to copy/move constraints
  for (size_t i{}; i < sectionLocks.size(); i++)
    sectionLocks[i]->lock();
  std::sort(ptrArr + (end - numPivots), ptrArr + end);
  size_t breakPoint{start}, validPoint{};
  unsigned long long staging{};
  for (unsigned int i{}; i < numPivots; i++)
  {
    unsigned long long pivotIdx{end - (numPivots - i)};
    const unsigned long long pivot{ptrArr[pivotIdx]};
    validPoint = breakPoint;
    while (validPoint < (end - (numPivots - i)))
    {
      if (ptrArr[validPoint] < pivot)
      {
        staging = ptrArr[breakPoint];
        ptrArr[breakPoint] = ptrArr[validPoint];
        ptrArr[validPoint] = staging;
        breakPoint++;
      }
      validPoint++;
    }
    partIdxs.push_back(breakPoint);
    staging = ptrArr[breakPoint];
    ptrArr[breakPoint] = ptrArr[pivotIdx];
    ptrArr[pivotIdx] = staging;
  }
  for (size_t i{}; i < sectionLocks.size(); i++)
    sectionLocks[i]->unlock();
  return partIdxs;
};

int main()
{
  const unsigned int threads{std::thread::hardware_concurrency()};
}
