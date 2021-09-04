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
// #include <random>

//Make a log macro bc I do not want to type this everywhere
#define log(x) std::clog << x << std::endl;

// This does what it says on the tin. I had to abstract it because for some reason
// std::sort can't be passed directly to std::async, probably some pointer bs
void sort(std::vector<unsigned long long> *ptrV)
{
  std::sort(ptrV->begin(), ptrV->end());
}

// Also does what it looks like; takes in two vectors and merges them in ascending order, into a 3rd already extant vector
void merge(
    std::vector<unsigned long long> *ptrL,
    std::vector<unsigned long long> *ptrR,
    std::vector<unsigned long long> *ptrAcc,
    const int id)
{
  std::cout << "STARTED MERGE WITH ID" << id << std::endl;
  const size_t sizeL{ptrL->size()}, sizeR{ptrR->size()};
  unsigned int posL{}, posR{};
  while (posL < sizeL || posR < sizeR)
  {
    if (posL < sizeL && posR < sizeR)
    {
      if ((*ptrL)[posL] < (*ptrR)[posR])
        ptrAcc->push_back((*ptrL)[posL++]);
      else
        ptrAcc->push_back((*ptrR)[posR++]);
    }
    else if (posL < sizeL)
    {
      std::vector<unsigned long long>{}.swap(*ptrR);
      for (; posL < sizeL; posL++)
        ptrAcc->push_back((*ptrL)[posL]);
      std::vector<unsigned long long>{}.swap(*ptrL);
    }
    else
    {
      std::vector<unsigned long long>{}.swap(*ptrL);
      for (; posR < sizeR; posR++)
        ptrAcc->push_back((*ptrR)[posR]);
      std::vector<unsigned long long>{}.swap(*ptrR);
    }
  }
  std::cout << "FINISHED MERGE WITH ID " << id << std::endl;
}

// This fills a given vector with ceil number of random unsigned 8byte integers
void alloc(std::vector<unsigned long long> *vect, const int ceil)
{
  // std::random_device rd;
  // std::default_random_engine generator(rd());
  // std::uniform_int_distribution<long long unsigned> distribution(0, 0xFFFFFFFFFFFFFFFF);
  for (int i = 0; i < ceil; i++)
    vect->push_back(rand());
  // vect->push_back(distribution(generator));
  log("ALLOCATED VECTOR");
  log(vect->size());
}

int main()
{
  // Poll hardware for number of logical threads
  const unsigned int queriedCPUs{std::thread::hardware_concurrency()};
  log("Queried available logical processors");
  log(queriedCPUs);
  log("Is this correct? [y/n]");
  char yesno;
  int threads;
  std::cin >> yesno;
  if (yesno != 'y' && yesno != 'Y')
  {
    log("Please input the correct number of threads available on your machine:");
    std::cin >> threads;
  }
  else
    threads = queriedCPUs;
  // Use some proprietary Windows C++ library garbage to query total system memory
  MEMORYSTATUSEX stax;
  stax.dwLength = sizeof(stax);
  GlobalMemoryStatusEx(&stax);
  const int physMem = stax.ullTotalPhys / (1024 * 1024);
  log("Queried available memory in MB");
  log(physMem);
  log("Is this roughly correct? [y/n]");
  std::cin >> yesno;
  int mem;
  if (yesno != 'y' && yesno != 'Y')
  {
    log("Please input the correct amount of physical memory in your machine, in MB");
    std::cin >> mem;
  }
  else
    mem = physMem;
  // Rough estimate that it takes about 30mb to split, allocate, sort, and merge a vector with 1,000,000 unsigned 8-byte integers
  int totalSort = (physMem / 30) * 1000000;
  totalSort = 10000;
  log("Max sortable long ints");
  log(totalSort);
  // Declare vectors to hold the different multi-threading "promsies"
  std::vector<std::future<void>> allocators{}, sorters{}, mergers{};
  // Declare 2D vectors to hold the dynamically allocated child vectors
  // One for the raw integers, and one for the merging phases
  std::vector<std::vector<unsigned long long>> vects{}, sieves{};
  std::vector<unsigned long long> acc{};
  // Create a vector for each CPU thread
  for (int i = 0; i < threads; i++)
    vects.push_back(std::vector<unsigned long long>{});
  log("Allocating vector space");
  // Spawn n - 1 futures/promises to launch n - 1 threads besides main for allocating the vector space
  for (int i = 0; i < (threads - 1); i++)
    allocators.push_back(std::async(std::launch::async, alloc, &vects[i], totalSort / threads));
  // Manually create one final allocation process on main, effectively using all n threads
  alloc(&vects[threads - 1], totalSort / threads);
  // Wait for all of the allocation futures to resolve before continuing
  for (int i = 0; i < allocators.size(); i++)
    allocators[i].wait();
  // Dump the vector of resolved allocation futures bc we need every MB of RAM we can get :)
  std::vector<std::future<void>>{}.swap(allocators);
  log("Let the sorting begin...");
  // Identical process for creating the allocation threads, only for creating sorts now
  for (int i = 0; i < (threads - 1); i++)
    sorters.push_back(std::async(std::launch::async, sort, &vects[i]));
  // and one last sort right here in main
  sort(&vects[threads - 1]);
  // wait for all of the sorts to resolve
  for (int i = 0; i < sorters.size(); i++)
    sorters[i].wait();
  log("Threaded sort complete, beginning merge");
  log("(memory usage will increase substantially)");
  // Here's where the fun starts
  // Each merge takes in 2 vectors, so we only need to iterate every other thread
  // At this point, we're guaranteed to be spawning merge threads < CPU threads, so we no longer need
  // to do the -1/+1 trick from allocating and sorting
  for (int i = 0; i < threads; i += 2)
  {
    // Create a new vector into which we will merge the other two
    // Creating it out here and mutating it saves difficulty with having to
    // retrieve the value from the future later
    sieves.push_back(std::vector<unsigned long long>{});
    // Push a merge future into the mergers future vector
    mergers.push_back(std::async(std::launch::async, merge, &vects[i], &vects[i + 1], &sieves[sieves.size() - 1], i));
  }
  // Wait for the merge threads to resolve
  for (int i = 0; i < mergers.size(); i++)
    mergers[i].wait();
  // Preemptively create a position to track how many of the sieves we have recursively merged
  int sPos = 0;
  // While this position memory has not yet passed the total size of the sieves container
  while (sieves[sieves.size() - 1].size() < totalSort)
  {
    // We store the size of the sieves vector prior to pushing anything into it so that we don't loop infinitely
    const size_t sieveSizeNow = sieves.size();
    // And also grab the size of the mergers for the same reason
    const size_t mergeSizeNow = mergers.size();
    // And then loop until there are no elements at one position past where we're looking
    // This, combined with the root while loop condition, will ensure that we only try to merge when we actually have 2 vectors
    // available to do so; if we've previously pushed on an odd number of vectors, we'll only merge an even number, and bring
    // in the odd one out on the next iteration of the outer while loop, when this for loop starts over
    for (; sPos + 1 < sieveSizeNow; sPos += 2)
    {
      sieves.push_back(std::vector<unsigned long long>{});
      mergers.push_back(std::async(std::launch::async, merge, &sieves[sPos], &sieves[sPos + 1], &sieves[sieves.size() - 1], sPos));
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
  // Grab the very final sieve
  acc.swap(sieves[sieves.size() - 1]);
  // And dump the 2D sieve and vector containers from memory
  std::vector<std::vector<unsigned long long>>{}.swap(sieves);
  std::vector<std::vector<unsigned long long>>{}.swap(vects);
  const size_t accSize = acc.size();
  for (int i = 0; i < accSize; i++)
    log(acc[i]);
  log("SORT AND MERGE COMPLETE");
  system("pause");
}

// Skip to content

// Search…
// All gists
// Back to GitHub
// @jm-alan
// @mcleary
// mcleary/Timer.cpp
// Last active 2 months ago • Report abuse
// 29
// 10
//  Code
//  Revisions 2
//  Stars 29
//  Forks 10
// C++ Timer using std::chrono
// Timer.cpp
// #include <iostream>
// #include <chrono>
// #include <ctime>
// #include <cmath>

// class Timer
// {
// public:
//     void start()
//     {
//         m_StartTime = std::chrono::system_clock::now();
//         m_bRunning = true;
//     }

//     void stop()
//     {
//         m_EndTime = std::chrono::system_clock::now();
//         m_bRunning = false;
//     }

//     double elapsedMilliseconds()
//     {
//         std::chrono::time_point<std::chrono::system_clock> endTime;

//         if(m_bRunning)
//         {
//             endTime = std::chrono::system_clock::now();
//         }
//         else
//         {
//             endTime = m_EndTime;
//         }

//         return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
//     }

//     double elapsedSeconds()
//     {
//         return elapsedMilliseconds() / 1000.0;
//     }

// private:
//     std::chrono::time_point<std::chrono::system_clock> m_StartTime;
//     std::chrono::time_point<std::chrono::system_clock> m_EndTime;
//     bool                                               m_bRunning = false;
// };

// long fibonacci(unsigned n)
// {
//     if (n < 2) return n;
//     return fibonacci(n-1) + fibonacci(n-2);
// }

// int main()
// {
// //    std::chrono::time_point<std::chrono::system_clock> start, end;
// //    start = std::chrono::system_clock::now();
// //    Timer timer;
// //    timer.start();
// //    std::cout << "f(42) = " << fibonacci(42) << '\n';
// //    timer.stop();
// //
// //    std::cout << "Time: " << timer.elapsed() << std::endl;
// //    end = std::chrono::system_clock::now();

// //    std::chrono::duration<double> elapsed_seconds = end-start;
// //    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

// //    std::cout << "finished computation at " << std::ctime(&end_time)
// //    << "elapsed time: " << elapsed_seconds.count() << "s\n";

//     Timer timer;
//     timer.start();
//     int counter = 0;
//     double test, test2;
//     while(timer.elapsedSeconds() < 10.0)
//     {
//         counter++;
//         test = std::cos(counter / M_PI);
//         test2 = std::sin(counter / M_PI);
//     }
//     timer.stop();

//     std::cout << counter << std::endl;
//     std::cout << "Seconds: " << timer.elapsedSeconds() << std::endl;
//     std::cout << "Milliseconds: " << timer.elapsedMilliseconds() << std::endl
//     ;
// }
// @Vasily-Biryukov
// Vasily-Biryukov commented on Jan 21, 2020
// Better use std::chrono::steady_clock, because the system time (used by system_clock) can be adjusted at any moment.

// @jm-alan

// Leave a comment
// No file chosen
// Attach files by dragging & dropping, selecting or pasting them.
// © 2021 GitHub, Inc.
// Terms
// Privacy
// Security
// Status
// Docs
// Contact GitHub
// Pricing
// API
// Training
// Blog
// About
