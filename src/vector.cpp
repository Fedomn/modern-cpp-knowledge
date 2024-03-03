#include "mcpp/vector.h"

#include <algorithm>
#include <cstring>

namespace vector
{
  template<typename T>
  Vector<T>::Vector(int size) : elem{ new T[static_cast<unsigned long>(size)]{} },
                                sz{ size }
  {
    // non c++11 use std::memset
    // std::memset(elem, 0, static_cast<unsigned long>(size) * sizeof(T));
  }

  template<typename T>
  Vector<T>::Vector(std::initializer_list<T> list) : elem{ new T[list.size()] },
                                                     sz{ static_cast<int>(list.size()) }
  {
    std::copy(list.begin(), list.end(), elem);
  }

  template<typename T>
  T& Vector<T>::operator[](int idx)
  {
    return elem[idx];
  }

  // Below are explicit instantiation of template functions.
  // In order to make the compiler know the implementation of the template functions, we need to explicitly instantiate them
  // here, so that they can be picked up by the linker.

  template Vector<int>::Vector(int size);
  template Vector<int>::Vector(std::initializer_list<int> list);
  template int& Vector<int>::operator[](int idx);
  template int Vector<int>::size();

  template Vector<double>::Vector(int size);
  template Vector<double>::Vector(std::initializer_list<double> list);
  template double& Vector<double>::operator[](int idx);
  template int Vector<double>::size();
}  // namespace vector
