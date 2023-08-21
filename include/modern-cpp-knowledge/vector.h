#include <cstdio>

#include "initializer_list"

namespace vector
{
  template<typename T>
  class Vector
  {
   private:
    T* elem;
    int sz;

   public:
    Vector() = default;

    explicit Vector(int size);

    // initialize with a list of doubles
    Vector(std::initializer_list<T> list);

    Vector(const Vector& input)
    {
      elem = new T[static_cast<unsigned long>(input.sz)];
      sz = input.sz;
      puts("copy constructor");
      for (int i = 0; i < sz; ++i)
      {
        elem[i] = input.elem[i];
      }
    };  // copy constructor
    Vector& operator=(const Vector& input)
    {
      puts("copy assignment");
      T* p = new T[static_cast<unsigned long>(input.sz)];
      for (int i = 0; i < input.sz; ++i)
      {
        p[i] = input.elem[i];
      }
      delete[] elem;
      elem = p;
      sz = input.sz;
      return *this;
    };  // copy assignment

    Vector(Vector&& input) : elem(input.elem), sz(input.sz)
    {
      puts("move constructor");
      input.elem = nullptr;
      input.sz = 0;
    };  // move constructor
    Vector& operator=(Vector&& input)
    {
      puts("move assignment");
      delete[] elem;
      elem = input.elem;
      sz = input.sz;
      input.elem = nullptr;
      input.sz = 0;
      return *this;
    };  // move assignment

    ~Vector()
    {
      delete[] elem;
      puts("destructor");
    }

    T& operator[](int idx);
    int size()
    {
      return sz;
    }
  };
}  // namespace vector