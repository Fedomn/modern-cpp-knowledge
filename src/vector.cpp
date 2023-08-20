#include "modern-cpp-knowledge/vector.h"

#include <algorithm>

namespace vector
{
  Vector::Vector(unsigned long size) : elem{ new double[size] }, sz{ size }
  {
  }

  Vector::Vector(std::initializer_list<double> list) : elem{ new double[list.size()] }, sz{ list.size() }
  {
    std::copy(list.begin(), list.end(), elem);
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
