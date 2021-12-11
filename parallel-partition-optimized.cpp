#include <future>
#include <vector>
#include <algorithm>
#include <random>
#include <iostream>
#include <chrono>

#define sync(timer) timer = std::chrono::system_clock::now()
#define timer_cast_ms(timer) (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer)).count()

std::random_device rd{};
std::default_random_engine generator{rd()};
std::uniform_int_distribution<unsigned long long> distribution{0, 0xFFFFFFFFFFFFFFFF};

std::vector<unsigned long long> partition_serial(
    unsigned long long *ptrArr,
    const unsigned long long start,
    const unsigned long long end,
    const unsigned int numPivots)
{
  std::vector<unsigned long long> partIdxs{};
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
  return partIdxs;
};

std::vector<unsigned long long> partition_known_start(
    unsigned long long *ptrArr,
    const unsigned long long start,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned int numPivots)
{
  const unsigned long long end{futEnd.get()[0]};
  std::vector<unsigned long long> partIdxs{};
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
  return partIdxs;
};

std::vector<unsigned long long> partition_known_end(
    unsigned long long *ptrArr,
    std::shared_future<std::vector<unsigned long long>> futStart,
    const unsigned long long end,
    const unsigned int numPivots)
{
  const unsigned long long start{futStart.get()[0]};
  std::vector<unsigned long long> partIdxs{};
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
  return partIdxs;
};

std::vector<unsigned long long> partition_concurrent(
    unsigned long long *ptrArr,
    std::shared_future<std::vector<unsigned long long>> futStart,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned int numPivots)
{
  const unsigned long long start{futStart.get()[0]};
  const unsigned long long end{futEnd.get()[0]};
  std::vector<unsigned long long> partIdxs{};
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

void sort_known_start(
    unsigned long long *ptrV,
    const unsigned long long start,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned long long futIdx)
{
  std::clog << "Entered sort" << std::endl;
  const unsigned long long end{futEnd.get()[futIdx]};
  std::clog << "Successfully started sort " << start << " " << end << std::endl;
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

void sort_known_end(
    unsigned long long *ptrV,
    std::shared_future<std::vector<unsigned long long>> futStart,
    const unsigned long long end,
    const unsigned long long futIdx)
{
  std::clog << "Entered sort" << std::endl;
  const unsigned long long start{futStart.get()[futIdx]};
  std::clog << "Successfully started sort " << start << " " << end << std::endl;
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
    std::shared_future<std::vector<unsigned long long>> futStart,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned long long futStartIdx,
    const unsigned long long futEndIdx)
{
  std::clog << "Entered sort" << std::endl;
  const unsigned long long start{futStart.get()[futStartIdx]}, end{futEnd.get()[futEndIdx]};
  std::clog << "Successfully started sort " << start << " " << end << std::endl;
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
    unsigned long long *ptrArr,
    const unsigned long long start,
    const unsigned long long end)
{
  for (unsigned long long i{start}; i < end; i++)
    ptrArr[i] = distribution(generator);
  // ptrArr[i] = rand();
};

bool is_sorted(
    const unsigned long long *ptrArr,
    const unsigned long long len)
{
  for (unsigned long long i{}; i < len - 1; i++)
    if (ptrArr[i] > ptrArr[i + 1])
      return false;
  return true;
}

int main()
{
  const unsigned long long threads{std::thread::hardware_concurrency()};
  unsigned long long parts{threads};
  std::vector<std::shared_future<void>> futures{};
  const unsigned long long allocSectionSize{2500000000ULL / threads};
  const unsigned long long cap{allocSectionSize * threads};
  auto timer{std::chrono::system_clock::now()};
  const auto globalTimer{timer};
  unsigned long long *ints{new unsigned long long[cap]{}};
  std::clog << "Allocated array in " << timer_cast_ms(timer) << "ms" << std::endl;
  sync(timer);
  for (unsigned int i{}; i < threads; i++)
  {
    futures.push_back(
        std::async(
            std::launch::async,
            alloc,
            ints,
            allocSectionSize * i,
            allocSectionSize * (i + 1)));
  }
  for (size_t i{}; i < futures.size(); i++)
    futures[i].wait();
  std::clog << "Randomized array in " << timer_cast_ms(timer) << "ms" << std::endl;
  sync(timer);
  const unsigned long long partOne{partition_serial(ints, 0, cap, 1)[0]};
  std::shared_future<std::vector<unsigned long long>>
      futPartTwo{std::async(std::launch::async, partition_serial, ints, 0, partOne, 1)},
      futPartThree{std::async(std::launch::async, partition_serial, ints, partOne, cap, 1)},
      futPartFour{std::async(std::launch::async, partition_known_start, ints, 0, futPartTwo, 2)},
      futPartFive{std::async(std::launch::async, partition_known_end, ints, futPartTwo, partOne, 2)},
      futPartSix{std::async(std::launch::async, partition_known_start, ints, partOne, futPartThree, 2)},
      futPartSeven{std::async(std::launch::async, partition_known_end, ints, futPartThree, cap, 2)};
  futures.push_back(std::async(std::launch::async, sort_known_start, ints, 0, futPartFour, 0));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartFour, futPartFour, 0, 1));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartFour, futPartTwo, 1, 0));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartTwo, futPartFive, 0, 0));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartFive, futPartFive, 0, 1));
  futures.push_back(std::async(std::launch::async, sort_known_end, ints, futPartFive, partOne, 1));
  futures.push_back(std::async(std::launch::async, sort_known_start, ints, partOne, futPartSix, 0));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartSix, futPartSix, 0, 1));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartSix, futPartThree, 1, 0));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartThree, futPartSeven, 0, 0));
  futures.push_back(std::async(std::launch::async, sort_concurrent, ints, futPartSeven, futPartSeven, 0, 1));
  futures.push_back(std::async(std::launch::async, sort_known_end, ints, futPartSeven, cap, 1));
  for (size_t i{}; i < futures.size(); i++)
    futures[i].wait();
  std::clog << "Partitioned and sorted array in " << timer_cast_ms(timer) << "ms" << std::endl;
  sync(timer);
  std::clog << "Array is sorted: " << std::boolalpha << is_sorted(ints, cap) << std::endl;
  std::clog << "Validated array in " << timer_cast_ms(timer) << "ms" << std::endl;
  const auto finalTime{timer_cast_ms(globalTimer)};
  std::clog
      << "Total process for allocating, partitioning, sorting, and verifying "
      << cap << " 64-bit unsigned ints completed in " << finalTime
      << "ms, for a total speed of " << (cap / finalTime)
      << " ints/ms" << std::endl;
}
