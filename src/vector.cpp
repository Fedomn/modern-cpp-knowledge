#include "modern-cpp-knowledge/vector.h"

namespace vector
{
  Vector::Vector(unsigned long size) : elem{ new double[size] }, sz{ size }
  {
  }

  double& Vector::operator[](int idx)
  {
    return elem[idx];
  }

  unsigned long Vector::size()
  {
    return sz;
  }
}  // namespace vector
