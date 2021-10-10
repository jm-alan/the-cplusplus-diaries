#include "console.h"

void lMergeUnthreaded(List *, List *, List *);

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
  unsigned long long push(const unsigned long long on)
  {
    Node *ptrOn = new Node(on);
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
  friend auto operator<<(std::ostream &os, List const *ptrL) -> std::ostream &
  {
    if (ptrL->length == 0)
    {
      console::log("{");
      console::log("  head: NULL");
      console::log("  tail: NULL");
      console::log("  length: 0");
      console::log("  run: []");
      console::log("}");
    }
    else
    {
      Node *current = ptrL->head;
      console::log("{");
      console::inl("  head: ");
      console::log(ptrL->head->val);
      console::inl(" tail: ");
      console::log(ptrL->tail->val);
      console::inl("  run: [ ");
      console::inl(current->val);
      while (current->next != nullptr)
      {
        current = current->next;
        console::inl(" ");
        console::inl(current->val);
      }
      console::log("   ]");
      console::log("}");
    }
  }
};
