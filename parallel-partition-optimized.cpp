#include <future>
#include <vector>

const std::vector<unsigned long long> partition(
    unsigned long long *ptrArr,
    const unsigned long long start,
    const unsigned long long end,
    const unsigned int numPivots,
    std::vector<std::mutex *> sectionLocks)
{
  // cannot std::lock_guard bc lock_guard cannot be itself put into a vector
  // due to copy/move constraints
  for (size_t i{}; i < sectionLocks.size(); i++)
    sectionLocks[i]->lock();
  std::vector<unsigned long long> pivots{};
  for (unsigned int i{}; i < numPivots; i++)
    pivots.push_back(ptrArr[end - i]);

  for (size_t i{}; i < sectionLocks.size(); i++)
    sectionLocks[i]->unlock();
};

int main()
{
  const unsigned int threads{std::thread::hardware_concurrency()};
}
