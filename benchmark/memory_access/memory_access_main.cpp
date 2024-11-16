#include <chrono>
#include <deque>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <list>
#include <numeric>
#include <random>
#include <string>
#include <vector>

const int SIZE = 100'000'000;

template<typename T>
void sumUp(T& t, const std::string& cont)
{
  std::cout << std::fixed << std::setprecision(10);
  auto begin = std::chrono::steady_clock::now();
  std::size_t res = std::accumulate(t.begin(), t.end(), 0LL);
  std::chrono::duration<double> last = std::chrono::steady_clock::now() - begin;
  std::cout << cont << '\n';
  std::cout << "time: " << last.count() << '\n';
  std::cout << "res: " << res << '\n';
  std::cout << '\n';
  std::cout << '\n';
}

/**
    you read an int from memory more than the size of this one int is read from memory. An entire cache line is read from
    memory and stored in a CPU’s cache.

    On modern architectures, a cache line typically has 64 bytes. If you now request an additional variable from memory and
    this variable is already cached, the read directly uses this cache, and the operation is much faster

    A data structure such as std::vector, which stores its data in a contiguous memory block, is a cache line friendly data
    structure because each element in the cache line is typically used. This cache line friendliness also holds for a
    std::array, and std::string.
*/
int main()
{
  std::cout << "\n";
  std::random_device seed;
  std::mt19937 engine(seed());
  std::uniform_int_distribution<int> dist(0, 100);

  std::vector<int> randNum;
  randNum.reserve(SIZE);
  for (int i = 0; i < SIZE; i++)
  {
    randNum.push_back(dist(engine));
  }

  {
    std::vector<int> vec(randNum.begin(), randNum.end());
    sumUp(vec, "std::vector<int>");
  }

  {
    std::deque<int> deq(randNum.begin(), randNum.end());
    sumUp(deq, "std::deque<int>");
  }

  {
    std::list<int> lst(randNum.begin(), randNum.end());
    sumUp(lst, "std::list<int>");
  }

  {
    std::forward_list<int> forwardLst(randNum.begin(), randNum.end());
    sumUp(forwardLst, "std::forward_list<int>");
  }
}
