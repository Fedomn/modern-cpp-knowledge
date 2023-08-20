#include <cstdio>

#include "initializer_list"

namespace vector
{
  class Vector
  {
   public:
    explicit Vector(unsigned long size);

    // initialize with a list of doubles
    Vector(std::initializer_list<double> list);

    double& operator[](int idx);
    unsigned long size();

    ~Vector()
    {
      delete[] elem;
      puts("destructor");
    }

   private:
    double* elem;
    unsigned long sz;
  };
}  // namespace vector