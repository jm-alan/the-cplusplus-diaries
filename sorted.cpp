// Need IO
#include <iostream>
// Vectors
#include <vector>
// Threading/async
#include <future>
// Sorting
#include <algorithm>
// Hardware querying
#include <windows.h>
// Generating numbers to sort
#include <random>
// Timer
#include <chrono>

// and a system time macro as well
#define now() std::chrono::system_clock::now()

// one more for updating the timer
#define syncWatch(timer) timer = now()

// aaaaand one more for formatting the timer
#define timerInMS(timer) std::chrono::duration_cast<std::chrono::milliseconds>(now() - timer).count()

// This does what it says on the tin. I had to abstract it because for some reason
// std::sort can't be passed directly to std::async, probably some pointer bs
void sort(std::vector<unsigned long long> *ptrV)
{
  std::sort(ptrV->begin(), ptrV->end());
}

void trash(std::vector<unsigned long long> *ptrV)
{
  std::vector<unsigned long long> vTrash;
  ptrV->swap(vTrash);
}

// Also does what it looks like; takes in two vectors and merges them in ascending order, into a 3rd already extant vector
void merge(
    std::vector<unsigned long long> *ptrL,
    std::vector<unsigned long long> *ptrR,
    std::vector<unsigned long long> *ptrAcc)
{

  // Storing the vector sizes in discrete variables is marginally faster than calling .size() constantly
  const size_t sizeL{ptrL->size()}, sizeR{ptrR->size()};

  // Independently track the iterator positions of the left and right vectors
  int posL{}, posR{};

  // If either one of the position trackers has not yet reached the size of its respective vectors, keep iterating
  while (posL < sizeL || posR < sizeR)
  {

    // If BOTH haven't reached the end yet, then we know we can safetly key into both vectors to directly compare the values
    if (posL < sizeL && posR < sizeR)
    {

      // Put the lesser of the two into the accumulator
      if ((*ptrL)[posL] < (*ptrR)[posR])
        ptrAcc->push_back((*ptrL)[posL++]);
      else
        ptrAcc->push_back((*ptrR)[posR++]);
    }

    // If we've skipped the && above, then one of the two must have finished. If it's not this one, then we know the right must
    // have finished
    else if (posL < sizeL)
    {
      // So we create an empty vector that will get destroyed when this frame comes off the stack
      std::vector<unsigned long long> leftTrash{}, rightTrash{};
      // and swap its contents with the contents of the vector we know we're finished with, effectively deallocating the memory
      // when this stack frame is cleared
      ptrR->swap(rightTrash);

      // We iterate through the remainder of the unfinished vector
      for (; posL < sizeL; posL++)
        ptrAcc->push_back((*ptrL)[posL]);
      // and then dump its memory the same way
      ptrL->swap(leftTrash);
    }

    // If we get here, we know that the opposite of the above is true and we can do the same steps in reverse
    else
    {
      std::vector<unsigned long long> leftTrash{}, rightTrash{};
      ptrL->swap(leftTrash);
      for (; posR < sizeR; posR++)
        ptrAcc->push_back((*ptrR)[posR]);
      ptrR->swap(rightTrash);
    }
  }
}

// This fills a given vector with ceil number of random unsigned 8byte integers
void alloc(std::vector<unsigned long long> *vect, const int ceil)
{
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<long long unsigned> distribution(0, 0xFFFFFFFFFFFFFFFF);
  for (int i = 0; i < ceil; i++)
    // vect->push_back(rand());
    vect->push_back(distribution(generator));
}

int main()
{
  char again{};
  // Poll hardware for number of logical threads
  const unsigned int queriedCPUs{std::thread::hardware_concurrency()};
  std::clog
      << "Queried available logical processors: "
      << queriedCPUs
      << std::endl
      << std::endl;
  std::clog
      << "Is this correct? [y/n] ";
  char yesno;
  int threads;
  std::cin >> yesno;
  std::clog
      << std::endl;
  if (yesno != 'y' && yesno != 'Y')
  {
    std::clog
        << "Please input the correct number of threads available on your machine: ";
    std::cin >> threads;
  }
  else
    threads = queriedCPUs;
  // Use some proprietary Windows C++ library garbage to query total system memory
  MEMORYSTATUSEX stax;
  stax.dwLength = sizeof(stax);
  GlobalMemoryStatusEx(&stax);
  const int physMem = stax.ullAvailPhys / (1024 * 1024);
  const int totMem = stax.ullTotalPhys / (1024 * 1024);
  std::clog
      << "Queried available memory in MB: "
      << std::endl
      << std::endl
      << physMem
      << " of "
      << totMem
      << std::endl
      << std::endl
      << "Is this roughly correct? [y/n] ";
  std::cin >> yesno;
  int mem;
  if (yesno != 'y' && yesno != 'Y')
  {
    std::clog
        << std::endl
        << std::endl
        << "Please input the current amount of AVAILABLE (total - occupied) memory in your machine, in MB ";
    std::cin >> mem;
  }
  else
    mem = physMem;
  std::clog << std::endl;
  // Rough estimate that it takes about 19mb of RAM to allocate, sort, and merge 1,000,000 unsigned long longs
  int totalSort((mem / 19) * 1000000);
  std::clog
      << "Max sortable long ints: "
      << totalSort
      << std::endl
      << std::endl;

  do
  {
    // Declare vectors to hold the different multi-threading "promsies"
    std::vector<std::future<void>> allocators{}, sorters{}, mergers{};

    // Declare 2D vectors to hold the dynamically allocated child vectors
    // One for the raw integers, and one for the merging phases
    // These had to be constructed as a vector of pointers, because it seems the double-nesting of
    // the while loop and for loop below was causing the regular vectors to be garbage collected before
    // they could be accessed in multiple threads, so I had to heap allocate them, and inline dereferencing
    // the heap pointers caused the same issue so it HAD to be explicit pointers
    std::vector<std::vector<unsigned long long> *> sieves{}, vects{};

    // And one final vector to hold the sorted integers when we're finished
    std::vector<unsigned long long> acc{};

    // Create a vector for each CPU thread
    for (int i = 0; i < threads; i++)
      vects.push_back(new std::vector<unsigned long long>{});

    std::clog
        << "Allocating vector space"
        << std::endl
        << std::endl;
    // Start an absolute and per-area timer
    const std::chrono::_V2::system_clock::time_point globalTimer{now()};
    std::chrono::_V2::system_clock::time_point runningTimer{globalTimer};

    // Spawn n - 1 futures/promises to launch n - 1 threads besides main for allocating the vector space
    for (int i = 0; i < (threads - 1); i++)
      allocators.push_back(std::async(std::launch::async, alloc, vects[i], totalSort / threads));

    // Manually create one final allocation process on main, effectively using all n threads
    alloc(vects[threads - 1], totalSort / threads);

    // Wait for all of the allocation futures to resolve before continuing
    for (int i = 0; i < allocators.size(); i++)
      allocators[i].wait();
    std::clog
        << "Allocation completed in "
        << timerInMS(runningTimer)
        << " milliseconds"
        << std::endl
        << std::endl;

    // Dump the vector of resolved allocation futures
    std::vector<std::future<void>>{}.swap(allocators);

    // Bring the timer up to date
    syncWatch(runningTimer);
    std::clog
        << "Let the sorting begin..."
        << std::endl
        << std::endl;

    // Identical process for creating the allocation threads, only for creating sorts now
    for (int i = 0; i < (threads - 1); i++)
      sorters.push_back(std::async(std::launch::async, sort, vects[i]));

    // and one last sort right here in main
    sort(vects[threads - 1]);

    // wait for all of the sorts to resolve
    for (int i = 0; i < sorters.size(); i++)
      sorters[i].wait();

    std::clog
        << "Sort completed in "
        << timerInMS(runningTimer)
        << " milliseconds"
        << std::endl
        << std::endl;

    std::clog
        << "Beginning merge."
        << std::endl
        << "(memory usage will spike sporadically)"
        << std::endl
        << std::endl;

    syncWatch(runningTimer);

    // Here's where the fun starts
    // Each merge takes in 2 vectors, so we only need to iterate every other thread
    // At this point, we're guaranteed to be spawning merge threads < CPU threads, so we no longer need
    // to do the -1/+1 trick from allocating and sorting
    for (int i = 0; i < threads; i += 2)
    {

      // Create a new vector into which we will merge the other two
      // Creating it out here and mutating it saves difficulty with having to
      // retrieve the value from the future later
      sieves.push_back(new std::vector<unsigned long long>{});

      // Spawn a merge thread
      mergers.push_back(std::async(std::launch::async, merge, vects[i], vects[i + 1], sieves[sieves.size() - 1]));
    }

    // Wait for the merge threads to resolve
    for (int i = 0; i < mergers.size(); i++)
      mergers[i].wait();

    // Preemptively create a position to track how many of the sieves we have recursively merged
    int sPos = 0;

    // While the final sieve in the sieves container is not yet the same size as the total number of integers we're sorting,
    // then we know that we must continue merging
    while (sieves[sieves.size() - 1]->size() < totalSort)
    {

      // We store the size of the sieves vector prior to pushing anything into it so that we don't loop infinitely
      const size_t sieveSizeNow = sieves.size();

      // And also grab the size of the mergers for the same reason
      const size_t mergeSizeNow = mergers.size();

      // And then loop until there are no elements at one position past where we're looking
      // This will ensure that we only try to merge when we actually have 2 vectors
      // available to do so; if we've previously pushed on an odd number of vectors, we'll only merge an even number, and bring
      // in the odd one out on the next iteration of the outer while loop, when this for loop starts over
      for (; sPos + 1 < sieveSizeNow; sPos += 2)
      {
        sieves.push_back(new std::vector<unsigned long long>{});
        mergers.push_back(std::async(std::launch::async, merge, sieves[sPos], sieves[sPos + 1], sieves[sieves.size() - 1]));
      }

      // Wait for all the merge threads to resolve
      for (int i = mergeSizeNow; i < mergers.size(); i++)
        mergers[i].wait();

      // We can (hopefully) trust the merge function to correctly trash the vectors it ingests when its done with them, so
      // we don't have to do any memory cleanup here. There's a bit of garbage in the previous sieve positions now holding
      // empty vectors, but it's negligible
    }

    // Theoretically, when this while loop merge has finished, we've successfully merged all of our sorts together into one massive
    // vector, and we're finished.

    std::clog
        << "Merge completed in "
        << timerInMS(runningTimer)
        << " milliseconds"
        << std::endl
        << std::endl;

    const auto finalTimer = timerInMS(globalTimer);

    std::clog
        << "Total operation for allocating, sorting, and merging "
        << totalSort
        << " unsigned long long integers completed in "
        << finalTimer
        << " milliseconds"
        << std::endl
        << "for a total average rate of "
        << totalSort / finalTimer
        << " ints/ms"
        << std::endl
        << std::endl;

    // Uncomment the three lines below if you want to print the sorted vector when it's finished
    acc.swap(*(sieves[sieves.size() - 1]));
    // const size_t accSize = acc.size();
    // for (int i = 0; i < accSize; i++)
    //   std::clog << acc[i]);
    trash(&acc);
    std::cout << "Run again? [y/n] ";
    std::cin >> again;
    std::cout << std::endl;
  } while (again == 'y' || again == 'Y');

  system("pause");
}
