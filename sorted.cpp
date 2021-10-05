#include <vector>
#include <future>
#include <algorithm>
#include <windows.h>
#include <random>
#include <chrono>
#include "console.h"

#define now() std::chrono::system_clock::now()

#define syncWatch(timer) timer = now()

#define timerInMS(timer) std::chrono::duration_cast<std::chrono::milliseconds>(now() - timer).count()

void sort(std::vector<unsigned long long> *ptrV)
{
  std::sort(ptrV->begin(), ptrV->end());
}

void trash(std::vector<unsigned long long> *ptrV)
{
  std::vector<unsigned long long> vTrash{};
  ptrV->swap(vTrash);
}

void trash(std::vector<std::future<void>> *ptrV)
{
  std::vector<std::future<void>> vTrash{};
  ptrV->swap(vTrash);
}

void trash(std::vector<std::vector<unsigned long long> *> *ptrV)
{
  std::vector<std::vector<unsigned long long> *> vTrash{};
  ptrV->swap(vTrash);
}

void merge(
    std::vector<unsigned long long> *ptrL,
    std::vector<unsigned long long> *ptrR,
    std::vector<unsigned long long> *ptrAcc)
{
  const size_t sizeL{ptrL->size()}, sizeR{ptrR->size()};
  size_t posL{}, posR{};
  ptrAcc->reserve(sizeL + sizeR);

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
      trash(ptrR);
      while (posL < sizeL)
        ptrAcc->push_back((*ptrL)[posL++]);
      trash(ptrL);
    }
    else
    {
      trash(ptrL);
      while (posR < sizeR)
        ptrAcc->push_back((*ptrR)[posR++]);
      trash(ptrR);
    }
  }
}

void alloc(std::vector<unsigned long long> *vect, const int ceil)
{
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<unsigned long long> distribution(0, 0xFFFFFFFFFFFFFFFF);
  vect->reserve(ceil);
  for (int i = 0; i < ceil; i++)
    vect->emplace_back(distribution(generator));
  // vect->push_back(rand());
}

int main()
{
  char again{};
  const unsigned int queriedCPUs{std::thread::hardware_concurrency()};
  console::inl("Queried available logical processors: ");
  console::log(queriedCPUs);
  console::log("Is this correct? [y/n]");
  char yesno{};
  std::cin >> yesno;
  int threads;
  if (yesno != 'y' && yesno != 'Y')
  {
    console::log("Please input the correct number of threads available on your machine: ");
    std::cin >>
        threads;
  }
  else
    threads = queriedCPUs;

  do
  {
    yesno = 0;
    MEMORYSTATUSEX mstax{};
    mstax.dwLength = sizeof(mstax);
    GlobalMemoryStatusEx(&mstax);
    const int physMem{mstax.ullAvailPhys / (1024 * 1024)};
    const int totMem{mstax.ullTotalPhys / (1024 * 1024)};
    console::inl("Queried available memory in MB: ");
    console::inl(physMem);
    console::inl(" of ");
    console::log(totMem);
    console::log("Is this roughly correct? [y/n]");
    std::cin >> yesno;
    int mem{};
    if (yesno != 'y' && yesno != 'Y')
    {
      console::log("Please input the current amount of AVAILABLE (total - occupied) memory in your machine, in MB:");
      std::cin >> mem;
    }
    else
      mem = physMem;

    int totalSort{(((mem / 16) * 1000000) / threads) * threads};

    console::inl("Max sortable long ints: ");
    console::log(totalSort);

    std::vector<std::future<void>> allocators{}, sorters{}, mergers{};
    std::vector<std::vector<unsigned long long> *> sieves{}, vects{};
    std::vector<unsigned long long> acc{};

    for (int i = 0; i < threads; i++)
      vects.emplace_back(new std::vector<unsigned long long>{});

    console::log("Allocating vector space");
    // Start an absolute and per-area timer
    const std::chrono::system_clock::time_point globalTimer{now()};
    std::chrono::system_clock::time_point runningTimer{globalTimer};

    for (int i = 0; i < (threads - 1); i++)
      allocators.emplace_back(std::async(std::launch::async, alloc, vects[i], totalSort / threads));

    alloc(vects[threads - 1], totalSort / threads);

    for (int i = 0; i < allocators.size(); i++)
      allocators[i].wait();

    console::inl("Allocation completed in ");
    console::inl(timerInMS(runningTimer));
    console::log(" milliseconds");

    trash(&allocators);

    syncWatch(runningTimer);
    console::log("Let the sorting begin...");

    for (int i = 0; i < (threads - 1); i++)
      sorters.emplace_back(std::async(std::launch::async, sort, vects[i]));

    sort(vects[threads - 1]);

    for (int i = 0; i < sorters.size(); i++)
      sorters[i].wait();

    console::inl("Sort completed in ");
    console::inl(timerInMS(runningTimer));
    console::log(" milliseconds");
    console::log("Beginning merge. Memory usage will spike sporadically.");
    syncWatch(runningTimer);

    for (int i = 0; i < threads; i += 2)
    {
      sieves.emplace_back(new std::vector<unsigned long long>{});
      mergers.emplace_back(std::async(std::launch::async, merge, vects[i], vects[i + 1], sieves[sieves.size() - 1]));
    }

    for (int i = 0; i < mergers.size(); i++)
      mergers[i].wait();

    int sPos = 0;

    while (sieves[sieves.size() - 1]->size() < totalSort)
    {

      const size_t sieveSizeNow = sieves.size();

      for (; sPos + 1 < sieveSizeNow; sPos += 2)
      {
        sieves.emplace_back(new std::vector<unsigned long long>{});
        mergers.emplace_back(std::async(std::launch::async, merge, sieves[sPos], sieves[sPos + 1], sieves[sieves.size() - 1]));
      }

      for (int i = 0; i < mergers.size(); i++)
        mergers[i].wait();

      trash(&mergers);
    }
    console::inl("Merge completed in");
    console::inl(timerInMS(runningTimer));
    console::log("milliseconds");

    const auto finalTimer = timerInMS(globalTimer);

    console::log("Total operation for allocating, sorting, and merging completed in");
    console::inl(finalTimer);
    console::inl(" milliseconds, for a total average rate of ");
    console::inl(totalSort / finalTimer);
    console::log(" ints/ms");

    acc.swap(*(sieves[sieves.size() - 1]));
    // Uncomment the three lines below if you want to print the sorted vector when it's finished
    // const size_t accSize = acc.size();
    // for (int i = 0; i < accSize; i++)
    //   console::log(acc[i]);
    trash(&sieves);
    trash(&acc);
    console::log("Run again? [y/n] ");
    std::cin >> again;
    std::cout << std::endl;
  } while (again == 'y' || again == 'Y');
}
