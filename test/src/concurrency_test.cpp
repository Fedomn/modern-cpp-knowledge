#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <numeric>
#include <shared_mutex>
#include <thread>

using namespace std;

shared_mutex shared_lock;
int shared_data = 0;

void f(vector<double>& v, double* res)
{
  unique_lock<shared_mutex> l(shared_mutex);
  shared_data += 1;
  for (auto x : v)
    *res += x;
  cout << "Hello ";
}

struct F
{
  const vector<double>& v;
  double* res;

  F(vector<double>& vv, double* res) : v{ vv }, res{ res }
  {
  }
  void operator()()
  {
    unique_lock<shared_mutex> l(shared_mutex);
    shared_data += 1;
    for (auto x : v)
      *res += x;
    cout << "Parallel World!\n";
  }
};

TEST(ConcurrencyTest, Task_Thread_Test)
{
  vector<double> some_vec{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  vector<double> vec2{ 10, 11, 12, 13 };
  double res1;
  double res2;

  thread t1{ f, ref(some_vec), &res1 };
  thread t2{ F{ vec2, &res2 } };

  t1.join();  // wait for t1
  t2.join();  // wait for t2

  ASSERT_EQ(res1, 45);
  ASSERT_EQ(res2, 46);

  ASSERT_EQ(shared_data, 2);
}

void promise_set_value(promise<int>& p)
{
  p.set_value(42);
}
TEST(ConcurrencyTest, Future_Promise_Test)
{
  promise<int> p;
  future<int> f = p.get_future();

  thread t2{ promise_set_value, ref(p) };
  t2.join();
  ASSERT_EQ(f.get(), 42);
}

double accum(double* beg, double* end, double init)
{
  return accumulate(beg, end, init);
}
TEST(ConcurrencyTest, Async_Test)
{
  vector<double> vec1{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  vector<double> vec2{ 10, 11, 12, 13 };

  auto res1 = async(accum, vec1.data(), vec1.data() + vec1.size(), 0.0);
  auto res2 = async(accum, vec2.data(), vec2.data() + vec2.size(), 0.0);

  ASSERT_EQ(res1.get(), 45);
  ASSERT_EQ(res2.get(), 46);
}