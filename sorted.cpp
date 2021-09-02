#include <stdlib.h>
#include <vector>
#include <iostream>

using namespace std;

vector<int> quickSort (vector<int> vect)
{
  if (vect.size() < 2) return vect;
  const int pivot = vect.back();
  vect.pop_back();
  const int size = vect.size();
  vector<int> left, right, acc;
  for (int i = 0; i < size; i++) vect[i] < pivot ? left.push_back(vect[i]) : right.push_back(vect[i]);
  const vector<int> sortedLeft = quickSort(left), sortedRight = quickSort(right);
  const int leftSize = sortedLeft.size(), rightSize = sortedRight.size();
  for (int i = 0; i < leftSize; i++) acc.push_back(sortedLeft[i]);
  acc.push_back(pivot);
  for (int i = 0; i < rightSize; i++) acc.push_back(sortedRight[i]);
  return acc;
}

int main ()
{
  vector<int> unsorted;
  for (int i = 0; i < 100; i++) unsorted.push_back(rand());
  const vector<int> sorted = quickSort(unsorted);
  for (int i = 0; i < 100; i++) cout << sorted[i] << "|";
  cout << endl;
}
