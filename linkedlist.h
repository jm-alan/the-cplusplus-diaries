#include <mutex>

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
  static void merge(
      List *ptrL,
      List *ptrR,
      List *ptrAcc)
  {
    while (ptrL->length && ptrR->length)
    {
      if (ptrL->head->val < ptrR->head->val)
        ptrAcc->pushNode(ptrL->shift());
      else
        ptrAcc->pushNode(ptrR->shift());
    }
    while (ptrL->length)
      ptrAcc->pushNode(ptrL->shift());
    while (ptrR->length)
      ptrAcc->pushNode(ptrR->shift());
    delete ptrL;
    delete ptrR;
  };
  static void threadmerge(
      List *ptrL,
      List *ptrR,
      List *ptrAcc,
      std::mutex *lockL,
      std::mutex *lockR,
      std::mutex *lockAcc)
  {
    std::lock_guard<std::mutex> guardL{*lockL}, guardR{*lockR};
    while (ptrL->length && ptrR->length)
    {
      if (ptrL->head->val < ptrR->head->val)
        ptrAcc->pushNode(ptrL->shift());
      else
      {
        ptrAcc->pushNode(ptrR->shift());
      }
    }
      while (ptrL->length)
        ptrAcc->pushNode(ptrL->shift());
      while (ptrR->length)
        ptrAcc->pushNode(ptrR->shift());
    lockAcc->unlock();
    delete ptrL;
    delete ptrR;
  }
  List()
  {
    this->head = nullptr;
    this->tail = nullptr;
    this->length = 0;
  };
  unsigned long long pushNode(Node *ptrOn)
  {
    if (this->tail != nullptr)
    {
      this->tail->append(ptrOn);
      this->tail = this->tail->next;
    }
    else
    {
      this->tail = ptrOn;
      this->head = this->tail;
    }
    this->length = this->length + 1;
    return this->length;
  };
  unsigned long long pushVal(const unsigned long long on)
  {
    Node *ptrOn{new Node(on)};
    if (this->tail != nullptr)
    {
      this->tail->append(ptrOn);
      this->tail = this->tail->next;
    }
    else
    {
      this->tail = ptrOn;
      this->head = this->tail;
    }
    this->length = this->length + 1;
    return this->length;
  };
  Node *shift()
  {
    if (this->head == nullptr)
      return this->head;
    Node *staging{this->head};
    this->head = this->head->next;
    if (this->head)
      this->head->prev = nullptr;
    else
      this->tail = nullptr;
    staging->prev = nullptr;
    staging->next = nullptr;
    this->length = this->length - 1;
    if (this->length == 1)
    {
      this->tail = this->head;
      this->head->prev = nullptr;
    }
    else if (this->length == 0)
    {
      this->head = nullptr;
      this->tail = nullptr;
    }
    return staging;
  };
  void sort()
  {
    if (this->length < 2)
      return;
    const unsigned long long half{this->length / 2};
    const unsigned long long leftLength{half};
    const unsigned long long rightLength{this->length - half};
    unsigned long long currPos{1};
    Node *current{this->head};
    List *left{new List()}, *right{new List()};
    left->head = current;
    while (++currPos <= leftLength)
      current = current->next;
    left->tail = current;
    left->length = leftLength;
    currPos = 1;
    if (current != nullptr)
      current = current->next;
    right->head = current;
    while (++currPos <= rightLength)
      current = current->next;
    right->tail = current;
    right->length = rightLength;
    this->head = nullptr;
    this->tail = nullptr;
    this->length = 0;
    left->sort();
    right->sort();
    List::merge(left, right, this);
  }
};
