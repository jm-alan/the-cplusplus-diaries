#include <vector>
#include <random>
#include <future>
#include <iostream>
#include <chrono>

std::random_device rd{};
std::default_random_engine generator{rd()};
std::uniform_int_distribution<unsigned long long> distribution{0, 0xFFFFFFFFFFFFFFFF};

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
    std::mutex *sectionMut)
{
  for (unsigned long long i{start}; i < end; i++)
    ptrV[i] = distribution(generator);
  // ptrV[i] = rand();
  sectionMut->unlock();
};

bool is_sorted(const unsigned long long *ptrV, const unsigned long long size)
{
  for (unsigned long long i{}; i < (size - 1); i++)
    if (ptrV[i] > ptrV[i + 1])
      return false;
  return true;
};

void insert(
    unsigned long long *ptrV,
    const unsigned long long start,
    const unsigned long long end,
    std::vector<std::mutex *> locks)
{
  unsigned long long staging{};
  for (size_t i{}; i < locks.size(); i++)
    locks[i]->lock();
  for (unsigned long long i{1}; i < end; i++)
  {
    for (unsigned long long j{i}; j && ptrV[j - 1] > ptrV[j]; j--)
    {
      staging = ptrV[j];
      ptrV[j] = ptrV[j - 1];
      ptrV[j - 1] = staging;
    }
  }
  for (size_t i{}; i < locks.size(); i++)
    locks[i]->unlock();
};

int main()
{
  const auto threads{std::thread::hardware_concurrency()};
  std::vector<std::mutex *> locks{};
  std::vector<std::future<void>> futures{};
  const unsigned long long sectionSize{3475000000ULL / threads};
  const unsigned long long cap{sectionSize * threads};
  unsigned long long *ints{new unsigned long long[cap]{}};
  const auto timer{std::chrono::system_clock::now()};
  for (unsigned long long i{}; i < threads; i++)
    locks.push_back(new std::mutex);
  for (unsigned long long i{}; i < threads; i++)
  {
    locks[i]->lock();
    futures.push_back(
        std::async(
            std::launch::async,
            alloc,
            ints,
            (i * sectionSize),
            ((i + 1) * sectionSize),
            locks[i]));
  }
  for (unsigned long long i{}; i < threads; i++)
    futures.push_back(
        std::async(
            std::launch::async,
            sort_concurrent,
            ints,
            (i * sectionSize),
            ((i + 1) * sectionSize),
            locks[i]));
  for (unsigned long long step{2}; step < threads; step *= 2)
    for (
        unsigned long long i{};
        (i < threads) && (sectionSize * (i + step)) <= cap;
        i += step)
    {
      std::vector<std::mutex *> insertLocks{};
      for (unsigned long long l{i}; l < i + step; l++)
        insertLocks.push_back(locks[l]);
      futures.push_back(
          std::async(
              std::launch::async,
              insert,
              ints,
              sectionSize * i,
              sectionSize * (i + step),
              insertLocks));
    }
  insert(ints, 0, cap, locks);
  std::clog << std::boolalpha << is_sorted(ints, cap) << std::endl;
  const auto finalTime{(std::chrono::duration_cast<std::chrono::milliseconds>(timer - std::chrono::system_clock::now())).count()};
  std::clog
      << "Total operation to allocate, sort, and verify, completed in "
      << finalTime
      << "ms, for a total of "
      << (cap / finalTime)
      << " ints/ms"
      << std::endl;
}
