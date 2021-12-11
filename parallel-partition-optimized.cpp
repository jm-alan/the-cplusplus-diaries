#include <future>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>

std::random_device rd{};
std::default_random_engine generator{rd()};
std::uniform_int_distribution<unsigned long long> distribution{0, 0xFFFFFFFFFFFFFFFF};

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

void sort_serial(
    unsigned long long *ptrV,
    const unsigned long long start,
    const unsigned long long end)
{
  if ((end - start) < 2)
    return;
  const unsigned long long pivot = ptrV[end - 1];
  unsigned long long breakPoint{start}, validPointer{start};
  unsigned long long staging{};
  while (validPointer < (end - 1))
  {
    if (ptrV[validPointer] < pivot)
    {
      staging = ptrV[breakPoint];
      ptrV[breakPoint] = ptrV[validPointer];
      ptrV[validPointer] = staging;
      breakPoint++;
    }
    validPointer++;
  }
  staging = ptrV[breakPoint];
  ptrV[breakPoint] = ptrV[end - 1];
  ptrV[end - 1] = staging;
  sort_serial(ptrV, start, breakPoint);
  sort_serial(ptrV, breakPoint + 1, end);
};

void sort_concurrent(
    unsigned long long *ptrV,
    const unsigned long long start,
    const unsigned long long end,
    std::mutex *sectionMut)
{
  std::lock_guard<std::mutex> guard{*sectionMut};
  if ((end - start) < 2)
    return;
  const unsigned long long pivot = ptrV[end - 1];
  unsigned long long breakPoint{start}, validPointer{start};
  unsigned long long staging{};
  while (validPointer < (end - 1))
  {
    if (ptrV[validPointer] < pivot)
    {
      staging = ptrV[breakPoint];
      ptrV[breakPoint] = ptrV[validPointer];
      ptrV[validPointer] = staging;
      breakPoint++;
    }
    validPointer++;
  }
  staging = ptrV[breakPoint];
  ptrV[breakPoint] = ptrV[end - 1];
  ptrV[end - 1] = staging;
  sort_serial(ptrV, start, breakPoint);
  sort_serial(ptrV, breakPoint + 1, end);
};

void alloc(
    unsigned long long *ptrV,
    const unsigned long long start,
    const unsigned long long end,
    std::mutex *sectionLock)
{
  for (unsigned long long i{start}; i < end; i++)
    ptrV[i] = distribution(generator);
  // ptrV[i] = rand();
  sectionLock->unlock();
};

int main()
{
  const unsigned int threads{std::thread::hardware_concurrency()};
}
