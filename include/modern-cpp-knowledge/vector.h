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
    explicit Vector(int size);

    // initialize with a list of doubles
    Vector(std::initializer_list<T> list);

    Vector(const Vector& input);             // copy constructor
    Vector& operator=(const Vector& input);  // copy assignment
    Vector(Vector&& input);                  // move constructor
    Vector& operator=(Vector&& input);       // move assignment

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