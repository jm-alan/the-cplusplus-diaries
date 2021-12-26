#include <future>    // threads
#include <vector>    // dynamic arrays
#include <algorithm> // std::sort
#include <random>    // for creating random number generators
#include <iostream>  // terminal input / output
#include <chrono>    // timer

// macros (illegal, I know) to manage the timer
#define sync(timer) timer = std::chrono::system_clock::now()
#define timer_cast_ms(timer)                                 \
  (                                                          \
      std::chrono::duration_cast<std::chrono::milliseconds>( \
          std::chrono::system_clock::now() - timer))         \
      .count()

// Instantiate random device & range of numbers to be generated
std::random_device rd{};
std::default_random_engine generator{rd()}; //
std::uniform_int_distribution<unsigned int> distribution{0, 0xFFFFFFFF};

// The partition function, generally speaking, does the pivot step
// of quickSort, without recursing
// There are 3 overloads; one where the start & end are concrete,
// one each where either the start or end may be std::shared_future
// It's been genericized to take in an arbitrary number of desired
// pivot points, all of which are selected from the end of the array
// It will use these pivots "blindly" i.e. no consideration is given
// to how well those pivots represent the distribution of numbers
// within the selection of the array being partitioned
// It returns a vector of the indicies where the partitions land

// Known start + end
std::vector<unsigned long long> partition(
    unsigned int *ptrArr,
    const unsigned long long start,
    const unsigned long long end,
    const unsigned int numPivots)
{
  std::vector<unsigned long long> partIdxs{};
  std::sort(ptrArr + (end - numPivots), ptrArr + end);
  size_t breakPoint{start}, validPoint{};
  unsigned int staging{};
  for (unsigned int i{}; i < numPivots; i++)
  {
    unsigned long long pivotIdx{end - (numPivots - i)};
    const unsigned int pivot{ptrArr[pivotIdx]};
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

// Known start, possibly unknown end
std::vector<unsigned long long> partition(
    unsigned int *ptrArr,
    const unsigned long long start,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned int numPivots)
{
  const unsigned long long end{futEnd.get()[0]};
  std::vector<unsigned long long> partIdxs{};
  std::sort(ptrArr + (end - numPivots), ptrArr + end);
  size_t breakPoint{start}, validPoint{};
  unsigned int staging{};
  for (unsigned int i{}; i < numPivots; i++)
  {
    unsigned long long pivotIdx{end - (numPivots - i)};
    const unsigned int pivot{ptrArr[pivotIdx]};
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

// Possibly unknown start, known end
std::vector<unsigned long long> partition(
    unsigned int *ptrArr,
    std::shared_future<std::vector<unsigned long long>> futStart,
    const unsigned long long end,
    const unsigned int numPivots)
{
  const unsigned long long start{futStart.get()[0]};
  std::vector<unsigned long long> partIdxs{};
  std::sort(ptrArr + (end - numPivots), ptrArr + end);
  size_t breakPoint{start}, validPoint{};
  unsigned int staging{};
  for (unsigned int i{}; i < numPivots; i++)
  {
    unsigned long long pivotIdx{end - (numPivots - i)};
    const unsigned int pivot{ptrArr[pivotIdx]};
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

// This is just fully in-place quicksort
void sort_serial(
    unsigned int *ptrArr,
    const unsigned long long start,
    const unsigned long long end)
{
  if ((end - start) < 2)
    return;
  const unsigned int pivot = ptrArr[end - 1];
  unsigned long long breakPoint{start}, validPointer{start};
  unsigned int staging{};
  while (validPointer < (end - 1))
  {
    if (ptrArr[validPointer] < pivot)
    {
      staging = ptrArr[breakPoint];
      ptrArr[breakPoint] = ptrArr[validPointer];
      ptrArr[validPointer] = staging;
      breakPoint++;
    }
    validPointer++;
  }
  staging = ptrArr[breakPoint];
  ptrArr[breakPoint] = ptrArr[end - 1];
  ptrArr[end - 1] = staging;
  sort_serial(ptrArr, start, breakPoint);
  sort_serial(ptrArr, breakPoint + 1, end);
};

// The sort_concurrent is identical to sort_serial except that
// it exists in an overload set spanning the different permutations
// of known & unknown start & end indicies
// The unknown indicies are passed in from a vector, since that
// is the return value of the partition function
// They are wrapped in a lambda to pass overloads to std::async,
// so all overloads must take in 5 arguments even though in
// the case of only having 1 future passed in, only one of the
// index arguments will be used. In those cases, the 4th argument
// is taken as the selecting index, and the 5th argument is discarded.

// Known start, possibly unknown end, 1 index, 1 waste
void sort_concurrent(
    unsigned int *ptrArr,
    const unsigned long long start,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned int futIdx,
    const unsigned int _waste)
{
  const unsigned long long end{futEnd.get()[futIdx]};
  if ((end - start) < 2)
    return;
  const unsigned int pivot = ptrArr[end - 1];
  unsigned long long breakPoint{start}, validPointer{start};
  unsigned int staging{};
  while (validPointer < (end - 1))
  {
    if (ptrArr[validPointer] < pivot)
    {
      staging = ptrArr[breakPoint];
      ptrArr[breakPoint] = ptrArr[validPointer];
      ptrArr[validPointer] = staging;
      breakPoint++;
    }
    validPointer++;
  }
  staging = ptrArr[breakPoint];
  ptrArr[breakPoint] = ptrArr[end - 1];
  ptrArr[end - 1] = staging;
  sort_serial(ptrArr, start, breakPoint);
  sort_serial(ptrArr, breakPoint + 1, end);
};

// Possibly unknown start, known end, 1 index, 1 waste
void sort_concurrent(
    unsigned int *ptrArr,
    std::shared_future<std::vector<unsigned long long>> futStart,
    const unsigned long long end,
    const unsigned int futIdx,
    const unsigned int _waste)
{
  const unsigned long long start{futStart.get()[futIdx]};
  if ((end - start) < 2)
    return;
  const unsigned int pivot = ptrArr[end - 1];
  unsigned long long breakPoint{start}, validPointer{start};
  unsigned int staging{};
  while (validPointer < (end - 1))
  {
    if (ptrArr[validPointer] < pivot)
    {
      staging = ptrArr[breakPoint];
      ptrArr[breakPoint] = ptrArr[validPointer];
      ptrArr[validPointer] = staging;
      breakPoint++;
    }
    validPointer++;
  }
  staging = ptrArr[breakPoint];
  ptrArr[breakPoint] = ptrArr[end - 1];
  ptrArr[end - 1] = staging;
  sort_serial(ptrArr, start, breakPoint);
  sort_serial(ptrArr, breakPoint + 1, end);
};

// Possibly unknown start & end, both indicies used
void sort_concurrent(
    unsigned int *ptrArr,
    std::shared_future<std::vector<unsigned long long>> futStart,
    std::shared_future<std::vector<unsigned long long>> futEnd,
    const unsigned int futStartIdx,
    const unsigned int futEndIdx)
{
  const unsigned long long
      start{futStart.get()[futStartIdx]},
      end{futEnd.get()[futEndIdx]};
  if ((end - start) < 2)
    return;
  const unsigned int pivot = ptrArr[end - 1];
  unsigned long long breakPoint{start}, validPointer{start};
  unsigned int staging{};
  while (validPointer < (end - 1))
  {
    if (ptrArr[validPointer] < pivot)
    {
      staging = ptrArr[breakPoint];
      ptrArr[breakPoint] = ptrArr[validPointer];
      ptrArr[validPointer] = staging;
      breakPoint++;
    }
    validPointer++;
  }
  staging = ptrArr[breakPoint];
  ptrArr[breakPoint] = ptrArr[end - 1];
  ptrArr[end - 1] = staging;
  sort_serial(ptrArr, start, breakPoint);
  sort_serial(ptrArr, breakPoint + 1, end);
};

// Inserts random numbers ptrArr:[start, end)
void randomize(
    unsigned int *ptrArr,
    const unsigned long long start,
    const unsigned long long end)
{
  for (unsigned long long i{start}; i < end; i++)
    ptrArr[i] = distribution(generator);
  // ptrArr[i] = rand();
};

bool is_sorted(
    const unsigned int *ptrArr,
    const unsigned int len)
{
  for (unsigned int i{}; i < len - 1; i++)
    if (ptrArr[i] > ptrArr[i + 1])
      return false;
  return true;
}

// Lambda wrapper for partition function to clean up repeatedly
// writing futures.push_back(std::async etc etc etc)
const auto async_partition =
    [](auto to_part, auto start, auto end, auto num_parts)
{
  return std::async(
      std::launch::async,
      [](auto one, auto two, auto three, auto four)
      { return partition(one, two, three, four); },
      to_part,
      start,
      end,
      num_parts);
};

// Lambda wraper for sort function for the same reason as the partition
const auto async_sort =
    [](auto to_sort, auto start, auto end, auto posStartIdx, auto posEndIdx)
{
  return std::async(
      std::launch::async,
      [](auto one, auto two, auto three, auto four, auto five)
      { sort_concurrent(one, two, three, four, five); },
      to_sort,
      start,
      end,
      posStartIdx,
      posEndIdx);
};

int main()
{
  // Hopefully returns the number of threads on the system
  const unsigned int threads{std::thread::hardware_concurrency()};

  // To hold the sort futures so they don't auto-destroy
  std::vector<std::shared_future<void>> futures{};

  // How big is this array going to be?
  unsigned long long ceil{};
  std::cin >> ceil;
  std::clog << std::endl;

  // Divide the array into equally sized sections
  const unsigned long long randomizeSectionSize{ceil / threads};

  // Set the array length to section size * original divider,
  // intentionally possibly rounding down to avoid the sections
  // postentially not covering the entire array
  const unsigned long long cap{randomizeSectionSize * threads};

  // Start the timer
  auto timer{std::chrono::system_clock::now()};

  // Copy it to track the entire runtime
  const auto globalTimer{timer};

  // Allocate the array
  unsigned int *ints{new unsigned int[cap]{}};
  std::clog << "Allocated array in "
            << timer_cast_ms(timer) << "ms" << std::endl;

  // Sync only the non-global timer
  sync(timer);

  // Spawn threads to randomize individual sections of the array
  for (unsigned int i{}; i < threads; i++)
  {
    futures.push_back(
        std::async(
            std::launch::async,
            randomize,
            ints,
            randomizeSectionSize * i,
            randomizeSectionSize * (i + 1)));
  }

  // Wait for 100% of the randomizers to complete before beginning
  // partitioning, since the first partition operates on the entire
  // array
  for (size_t i{}; i < futures.size(); i++)
    futures[i].wait();
  std::clog << "Randomized array in " << timer_cast_ms(timer) << "ms" << std::endl;
  sync(timer);

  // Split the array into two parts, producing a vector of length 1,
  // whose sole element we grab immediately
  const unsigned long long partition_one{partition(ints, 0, cap, 1)[0]};

  // Create two threads, each partitioning one of the two regions
  // created by the first partition
  // The M1 Max has 10 cores (without regard for performance vs efficiency),
  // so we seek in total to produce 10 partitions, or 9 dividers.
  // Each one of these will produce 5 partitions split by 4 dividers, on
  // either side of the original divider, for a total of 10 partitions
  std::shared_future<std::vector<unsigned long long>>
      f_partition_two{async_partition(ints, 0, partition_one, 4)},
      f_partition_three{async_partition(ints, partition_one, cap, 4)};

  // Spawn a thread, individually sorting each of the partitions
  // The partitioning has effectively macro-sorted the array into 10
  // sections which are in order in terms of their rangest; the largest
  // value of partition 3 is guaranteed to be less than or equal to the
  // smallest value of partition 4, etc. So, if we now sort every individual
  // slice of the array, when the dust settles, the entire thing will be
  // sorted.
  // We don't have to worry about waiting for the previous two partition
  // futures to resolve, since we call .get on them within the sort function.
  // This works something like a mutex, but without me having to come up with
  // some sort of wild system for managing all of the mutexes and dynamically
  // assigning what section of the array they monitor.
  // Plus, in this particular case, literally all of the threads sorting the
  // "left" side of the array have to wait for f_partition_two, and all of
  // the threads on the right have to wait for f_partition_three, since it's
  // otherwise impossible to guarantee the individual section that thread will
  // be sorting will have had its range appropriately constrained beforehand
  futures.push_back(async_sort(ints, 0, f_partition_two, 0, 0));
  futures.push_back(async_sort(ints, f_partition_two, f_partition_two, 0, 1));
  futures.push_back(async_sort(ints, f_partition_two, f_partition_two, 1, 2));
  futures.push_back(async_sort(ints, f_partition_two, f_partition_two, 2, 3));
  futures.push_back(async_sort(ints, f_partition_two, partition_one, 3, 0));
  futures.push_back(async_sort(ints, partition_one, f_partition_three, 0, 0));
  futures.push_back(async_sort(ints, f_partition_three, f_partition_three, 0, 1));
  futures.push_back(async_sort(ints, f_partition_three, f_partition_three, 1, 2));
  futures.push_back(async_sort(ints, f_partition_three, f_partition_three, 2, 3));
  futures.push_back(async_sort(ints, f_partition_three, cap, 3, 0));

  // Then we wait for all of the sorting to finish
  for (size_t i{}; i < futures.size(); i++)
    futures[i].wait();

  // We clock the total sort time and store it separately so that we can use it
  // for calculations
  const auto sortTime{timer_cast_ms(timer)};
  std::clog
      << "Partitioned and sorted array in " << sortTime << "ms" << std::endl
      << "for a total processing speed of " << cap / sortTime << " ints/ms" << std::endl;

  // Sync the timer AFTER printing to the console so that the is_sorted function is
  // not unduly punished by the amount of time it takes to do IOps
  sync(timer);

  // Validate the sort and clock the validation time     vvvvvvvvvvvvvvvvvv
  std::clog << "Array is sorted: " << std::boolalpha << is_sorted(ints, cap) << std::endl;
  //                                    vvvvvvvvvvvvvvvvvvv
  std::clog << "Validated array in " << timer_cast_ms(timer) << "ms" << std::endl;

  // Clock the global timer after everything is finished
  const auto finalTime{timer_cast_ms(globalTimer)};
  std::clog
      << "Total process for randomizeating, partitioning, sorting, and verifying " << std::endl
      << cap << " 32-bit unsigned ints completed in " << finalTime << "ms" << std::endl
      << "for a total speed of " << (cap / finalTime) << " ints/ms" << std::endl;
}
