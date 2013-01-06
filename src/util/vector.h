#ifndef HELIUM_UTIL_VECTOR_H
#define HELIUM_UTIL_VECTOR_H

#include <vector>
#include <set>
#include <ostream>

namespace Helium {

  template<typename T>
  bool operator<(const std::vector<T> &v1, const std::vector<T> &v2)
  {
    if (v1.size() < v2.size())
      return true;
    for (std::size_t i = 0; i < v1.size(); ++i) {
      if (v1[i] < v2[i])
        return true;
      if (v1[i] > v2[i])
        return false;
    }
    return false;
  }

  template<typename T>
  bool operator>(const std::vector<T> &v1, const std::vector<T> &v2)
  {
    if (v1.size() > v2.size())
      return true;
    for (std::size_t i = 0; i < v1.size(); ++i) {
      if (v1[i] > v2[i])
        return true;
      if (v1[i] < v2[i])
        return false;
    }
    return false;
  }

  template<typename T>
  bool operator==(const std::vector<T> &v1, const std::vector<T> &v2)
  {
    if (v1.size() != v2.size())
      return false;
    for (std::size_t i = 0; i < v1.size(); ++i)
      if (v1[i] != v2[i])
        return false;
    return true;
  }  

  template<typename T>
  std::ostream& operator<<(std::ostream &os, const std::vector<T> &v)
  {
    os << "[ ";
    for (std::size_t i = 0; i < v.size(); ++i)
      os << v[i] << " ";
    os << "]";
    return os;
  }

  template<typename T>
  std::size_t unique_elements(const std::vector<T> &v)
  {
    std::set<T> set;
    for (std::size_t i = 0; i < v.size(); ++i)
      set.insert(v[i]);
    return set.size();
  }

}

#endif
