#include <vector>
#include <future>
#include <random>
#include <algorithm>

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

void sort(
    std::shared_ptr<std::vector<unsigned long long>> ptrV,
    std::shared_ptr<std::mutex> lockAlloc,
    std::shared_ptr<std::mutex> lockSort)
{
  std::lock_guard<std::mutex> guardV{*lockAlloc};
  std::sort(ptrV->begin(), ptrV->end());
  lockSort->unlock();
}
