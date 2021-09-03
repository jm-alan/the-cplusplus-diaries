#include <iostream>
#include <vector>
#include <future>
#include <algorithm>

#define log(x) std::clog << x << std::endl;

void sort(std::vector<int> *ptrV)
{
  std::sort(ptrV->begin(), ptrV->end());
}

void merge(std::vector<int> *ptrL, std::vector<int> *ptrR, std::vector<int> *ptrAcc)
{
  const size_t sizeL{(*ptrL).size()}, sizeR{(*ptrR).size()};
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
      std::vector<int>{}.swap((*ptrR));
      for (; posL < sizeL; posL++)
        ptrAcc->push_back((*ptrL)[posL]);
      std::vector<int>{}.swap((*ptrL));
    }
    else
    {
      std::vector<int>{}.swap((*ptrL));
      for (; posL < sizeL; posL++)
        ptrAcc->push_back((*ptrL)[posL]);
      std::vector<int>{}.swap((*ptrR));
    }
  }
}

void alloc(std::vector<int> *vect)
{
  for (int i = 0; i < 125000000; i++)
    vect->push_back(rand());
}

int main()
{
  std::vector<int> a{}, b{}, c{}, d{}, e{}, f{}, g{}, h{}, sieve1{}, sieve2{}, sieve3{}, sieve4{}, sieve5{}, sieve6{}, acc{};
  log("allocating");
  const std::future<void>
    allocA{std::async(std::launch::async, alloc, &a)},
    allocB{std::async(std::launch::async, alloc, &b)},
    allocC{std::async(std::launch::async, alloc, &c)},
    allocD{std::async(std::launch::async, alloc, &d)},
    allocE{std::async(std::launch::async, alloc, &e)},
    allocF{std::async(std::launch::async, alloc, &f)},
    allocG{std::async(std::launch::async, alloc, &g)};
  alloc(&h);
  allocG.wait();
  allocF.wait();
  allocE.wait();
  allocD.wait();
  allocC.wait();
  allocB.wait();
  allocA.wait();
  log("allocating complete, sorting");
  const std::future<void>
    sortA{std::async(std::launch::async, sort, &a)},
    sortB{std::async(std::launch::async, sort, &b)},
    sortC{std::async(std::launch::async, sort, &c)},
    sortD{std::async(std::launch::async, sort, &d)},
    sortE{std::async(std::launch::async, sort, &e)},
    sortF{std::async(std::launch::async, sort, &f)},
    sortG{std::async(std::launch::async, sort, &g)};
  sort(&h);
  sortG.wait();
  sortF.wait();
  sortE.wait();
  sortD.wait();
  sortC.wait();
  sortB.wait();
  sortA.wait();
  log("sort complete");
  log("merging phase 1");
  const std::future<void>
    mergeAB{std::async(std::launch::async, merge, &a, &b, &sieve1)},
    mergeCD{std::async(std::launch::async, merge, &c, &d, &sieve2)},
    mergeEF{std::async(std::launch::async, merge, &e, &f, &sieve3)};
  merge(&g, &h, &sieve4);
  mergeEF.wait();
  mergeCD.wait();
  mergeAB.wait();
  log("merging phase 2");
  const std::future<void> merge12{std::async(std::launch::async, merge, &sieve1, &sieve2, &sieve5)};
  merge(&sieve3, &sieve4, &sieve6);
  merge12.wait();
  log("merging phase 3");
  merge(&sieve5, &sieve6, &acc);
}
