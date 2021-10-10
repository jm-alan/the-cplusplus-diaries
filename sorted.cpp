#include <vector>
#include <future>
#include <memory>
#include <mutex>
#include <algorithm>
#include <windows.h>
#include <random>
#include <chrono>
#include <iterator>
#include "console.h"

#define now() std::chrono::system_clock::now()

#define syncWatch(timer) timer = now()

#define timerInMS(timer) std::chrono::duration_cast<std::chrono::milliseconds>(now() - timer).count()

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

void trash(std::shared_ptr<std::vector<unsigned long long>> ptrV)
{
  std::vector<unsigned long long> vTrash{};
  ptrV->swap(vTrash);
}

void trash(std::vector<std::shared_ptr<std::vector<unsigned long long>>> *ptrV)
{
  std::vector<std::shared_ptr<std::vector<unsigned long long>>> vTrash{};
  ptrV->swap(vTrash);
}

void alloc(
    std::shared_ptr<std::vector<unsigned long long>> allocV,
    const int ceil,
    std::shared_ptr<std::mutex> lockV)
{
  std::random_device rd;
  std::default_random_engine generator(rd());
  std::uniform_int_distribution<unsigned long long> distribution(0, 0xFFFFFFFFFFFFFFFF);
  allocV->reserve(ceil);
  for (int i{}; i < ceil; i++)
    allocV->emplace_back(distribution(generator));
  // vect->push_back(rand());
  lockV->unlock();
}

void vMerge(
    std::shared_ptr<std::vector<unsigned long long>> ptrL,
    std::shared_ptr<std::vector<unsigned long long>> ptrR,
    std::shared_ptr<std::vector<unsigned long long>> ptrAcc,
    std::shared_ptr<std::mutex> lockL,
    std::shared_ptr<std::mutex> lockR,
    std::shared_ptr<std::mutex> lockAcc)
{
  std::lock_guard<std::mutex> guardL{*lockL}, guardR{*lockR};
  const size_t sizeL{ptrL->size()}, sizeR{ptrR->size()};
  // ptrAcc->resize(sizeL + sizeR);
  // std::merge(ptrL->begin(), ptrL->end(), ptrR->begin(), ptrR->end(), ptrAcc->begin());
  // trash(ptrL);
  // trash(ptrR);

  size_t posL{}, posR{};
  ptrAcc->reserve(sizeL + sizeR);

  while (posL < sizeL && posR < sizeR)
  {
    if (ptrL->operator[](posL) < ptrR->operator[](posR))
      ptrAcc->push_back(ptrL->operator[](posL++));
    else
      ptrAcc->push_back(ptrR->operator[](posR++));
  }
  if (posL < sizeL)
    trash(ptrR);
  while (posL < sizeL)
    ptrAcc->push_back(ptrL->operator[](posL++));
  trash(ptrL);
  if (posR < sizeR)
    trash(ptrL);
  while (posR < sizeR)
    ptrAcc->push_back(ptrR->operator[](posR++));
  trash(ptrR);

  lockAcc->unlock();
}

void lMergeThreaded(
    std::shared_ptr<List> ptrL,
    std::shared_ptr<List> ptrR,
    std::shared_ptr<List> ptrAcc,
    std::shared_ptr<std::mutex> lockL,
    std::shared_ptr<std::mutex> lockR,
    std::shared_ptr<std::mutex> lockAcc)
{
  std::lock_guard<std::mutex> guardL{*lockL}, guardR{*lockR};
  while (ptrL->length && ptrR->length)
  {
    if (ptrL->head->val < ptrR->head->val)
      ptrAcc->push(ptrL->shift());
    else
      ptrAcc->push(ptrR->shift());
    while (ptrL->length)
      ptrAcc->push(ptrL->shift());
    while (ptrR->length)
      ptrAcc->push(ptrR->shift());
  }
  lockAcc->unlock();
}

void lMergeUnthreaded(
    List *ptrL,
    List *ptrR,
    List *ptrAcc)
{
  while (ptrL->length && ptrR->length)
  {
    if (ptrL->head->val < ptrR->head->val)
      ptrAcc->push(ptrL->shift());
    else
      ptrAcc->push(ptrR->shift());
    while (ptrL->length)
      ptrAcc->push(ptrL->shift());
    while (ptrR->length)
      ptrAcc->push(ptrR->shift());
  }
}

void sort(
    std::shared_ptr<std::vector<unsigned long long>> ptrV,
    std::shared_ptr<std::mutex> lockAlloc,
    std::shared_ptr<std::mutex> lockSort)
{
  std::lock_guard<std::mutex> guardV{*lockAlloc};
  std::sort(ptrV->begin(), ptrV->end());
  lockSort->unlock();
}

class Node
{
public:
  unsigned long long val;
  Node *prev;
  Node *next;
  Node(unsigned long long init)
  {
    this->val = init;
  };
  void append(Node *ptrN)
  {
    this->next = ptrN;
    this->next->prev = this;
  };
  void prepend(Node *ptrN)
  {
    this->prev = ptrN;
    this->prev->next = this;
  };
};

class List
{
public:
  Node *head;
  Node *tail;
  unsigned long long length;
  List()
  {
    this->head = nullptr;
    this->tail = nullptr;
    this->length = 0;
  };
  unsigned long long push(Node *ptrOn)
  {
    if (this->tail)
    {
      this->tail->append(ptrOn);
      this->tail = this->tail->next;
    }
    else
    {
      this->tail = ptrOn;
      this->head = this->tail;
    }
    return ++this->length;
  };
  unsigned long long unshift(Node *ptrOn)
  {
    if (this->head)
    {
      this->head->append(ptrOn);
      this->head = this->head->next;
    }
    else
    {
      this->head = ptrOn;
      this->tail = this->head;
    }
    return ++this->length;
  };
  Node *pop()
  {
    if (this->tail == nullptr)
      return this->tail;
    Node *staging{this->tail};
    this->tail = this->tail->prev;
    if (this->tail)
      this->tail->next = nullptr;
    else
      this->head = nullptr;
    staging->prev = nullptr;
    staging->next = nullptr;
    if (--this->length == 1)
    {
      this->head = this->tail;
      this->tail->next = nullptr;
    }
    return staging;
  };
  Node *shift()
  {
    if (this->head == nullptr)
      return this->head;
    Node *staging{this->head};
    this->head = this->head->prev;
    if (this->head)
      this->head->prev = nullptr;
    else
      this->tail = nullptr;
    staging->prev = nullptr;
    staging->next = nullptr;
    if (--this->length == 1)
    {
      this->tail = this->head;
      this->head->prev = nullptr;
    }
    return staging;
  };
  void sort()
  {
    if (this->length < 2)
      return;
    const unsigned long long mid{this->length / 2}, leftLength{mid}, rightLength{this->length - mid};
    List *lAcc{new List()}, *rAcc{new List()};
    int currPos{1};
    Node *current{this->head};
    while (++currPos <= leftLength && current != nullptr && current->next != nullptr)
      current = current->next;
    lAcc->tail = current;
    lAcc->length = leftLength;
    currPos = 1;
    if (current)
      current = current->next;
    rAcc->head = current;
    while (++currPos <= rightLength && current != nullptr && current->next != nullptr)
      current = current->next;
    rAcc->tail = current;
    rAcc->length = rightLength;
    this->head = nullptr;
    this->tail = nullptr;
    this->length = 0;
    lAcc->sort();
    rAcc->sort();
    lMergeUnthreaded(lAcc, rAcc, this);
  }
};

int main()
{
  char again{};
  const unsigned int queriedCPUs{std::thread::hardware_concurrency()};
  console::inl("Queried available logical processors: ");
  console::log(queriedCPUs);
  console::log("Is this correct? [y/n]");
  char yesno{};
  std::cin >> yesno;
  unsigned long long threads;
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
    const unsigned long long physMem{mstax.ullAvailPhys / (1024 * 1024)};
    const unsigned long long totMem{mstax.ullTotalPhys / (1024 * 1024)};
    console::inl("Queried available memory in MB: ");
    console::inl(physMem);
    console::inl(" of ");
    console::log(totMem);
    console::log("Is this roughly correct? [y/n]");
    std::cin >> yesno;
    unsigned long long mem{};
    if (yesno != 'y' && yesno != 'Y')
    {
      console::log("Please input the current amount of AVAILABLE (total - occupied) memory in your machine, in MB:");
      std::cin >> mem;
    }
    else
      mem = physMem;

    // const unsigned long long totalSort{(((mem / 16) * 1000000) / threads) * threads};
    const unsigned long long totalSort{(1000 / threads) * threads};

    console::inl("Max sortable long ints: ");
    console::log(totalSort);

    std::vector<std::shared_ptr<std::mutex>> locksAlloc, locksSorts{}, locksSieves{};
    std::vector<std::future<void>> allocators{}, sorters{}, mergers{};
    std::vector<
        std::shared_ptr<
            std::vector<unsigned long long>>>
        sieves{}, vects{};
    std::vector<unsigned long long> acc{};

    for (int i{}; i < threads; i++)
    {
      vects.emplace_back(std::make_shared<std::vector<unsigned long long>>());
    }

    // Start an absolute and per-area timer
    const std::chrono::system_clock::time_point globalTimer{now()};
    std::chrono::system_clock::time_point runningTimer{globalTimer};

    for (int i{}; i < threads; i++)
    {
      locksAlloc.emplace_back(std::make_shared<std::mutex>());
      locksAlloc[i]->lock();
      allocators.emplace_back(std::async(std::launch::async, alloc, vects[i], totalSort / threads, locksAlloc[i]));
    }

    for (int i{}; i < threads; i++)
    {
      locksSorts.emplace_back(std::make_shared<std::mutex>());
      locksSorts[i]->lock();
      sorters.emplace_back(std::async(std::launch::async, sort, vects[i], locksAlloc[i], locksSorts[i]));
    }

    for (int i{}; i < threads; i += 2)
    {
      sieves.emplace_back(std::make_shared<std::vector<unsigned long long>>());
      locksSieves.emplace_back(std::make_shared<std::mutex>());
      locksSieves[locksSieves.size() - 1]->lock();
      mergers.emplace_back(std::async(std::launch::async, vMerge, vects[i], vects[i + 1], sieves[sieves.size() - 1], locksSorts[i], locksSorts[i + 1], locksSieves[locksSieves.size() - 1]));
    }

    int sPos{};

    while (sieves[sieves.size() - 1]->size() < totalSort)
    {
      const size_t sieveSizeNow = sieves.size();

      for (; sPos + 1 < sieveSizeNow; sPos += 2)
      {
        sieves.emplace_back(std::make_shared<std::vector<unsigned long long>>());
        locksSieves.emplace_back(std::make_shared<std::mutex>());
        locksSieves[locksSieves.size() - 1]->lock();
        mergers.emplace_back(std::async(std::launch::async, vMerge, sieves[sPos], sieves[sPos + 1], sieves[sieves.size() - 1], locksSieves[sPos], locksSieves[sPos + 1], locksSieves[locksSieves.size() - 1]));
      }
    }

    console::inl("Merge completed in ");
    console::inl(timerInMS(runningTimer));
    console::log(" milliseconds");

    const auto finalTimer = timerInMS(globalTimer);

    console::log("Total operation for allocating, sorting, and merging completed in");
    console::inl(finalTimer);
    console::inl(" milliseconds, for a total average rate of ");
    console::inl(totalSort / finalTimer);
    console::log(" ints/ms");

    // Uncomment the lines below if you want to print the sorted vector when it's finished
    acc.swap(*(sieves[sieves.size() - 1]));
    trash(&sieves);
    const size_t accSize = acc.size();
    for (int i{}; i < accSize; i++)
      console::log(acc[i]);
    trash(&acc);
    console::log("Run again? [y/n] ");
    std::cin >> again;
    std::cout << std::endl;
  } while (again == 'y' || again == 'Y');
}
